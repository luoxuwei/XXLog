//
// Created by 罗旭维 on 2021/11/11.
//

#include <util/file_operations.h>
#include <dirent.h>
#include <errno.h>
#include <util/strutil.h>
#include <sys/stat.h>
#include <util/thread.h>
#include <zconf.h>
#include <util/log_util.h>
#include "log_file.h"
#include <vector>

#include "log_zlib_buffer.h"
#define LOG_EXT "xlog"

namespace xxlog {
    static Mutex sg_mutex_dir_attr; //log目录的锁
    static const long kMinLogAliveTime = 24 * 60 * 60;    // 1 days in second

    LogFile::LogFile(const XXLogConfig &_config)
    : config_(_config)
    , log_buff_(new mars::xlog::LogZlibBuffer(nullptr, 0, true, _config.pub_key.c_str())) {

    }

    void LogFile::SetMaxAliveDuration(long _max_time) {
        if (_max_time >= kMinLogAliveTime) {
            max_alive_time_ = _max_time;
        }
    }

    void LogFile::Open() {
        MutexGuard _guard(sg_mutex_dir_attr);
        if (!config_.cachedir.empty()) {
            ::mkdir(config_.cachedir.c_str(), S_IRWXU|S_IRWXG|S_IRWXO);
            Thread(std::bind(&LogFile::_DelTimeoutFile, this, config_.cachedir), "").StartAfter(2 * 60 * 1000);
            Thread(std::bind(&LogFile::_MoveOldFiles, this, config_.cachedir, config_.logdir, config_.nameprefix), "").StartAfter(3 * 60 * 1000);
        }

        Thread(std::bind(&LogFile::_DelTimeoutFile, this, config_.logdir), "").StartAfter(2 * 60 * 1000);
        ::mkdir(config_.logdir.c_str(), S_IRWXU|S_IRWXG|S_IRWXO);
    }

    void LogFile::_DelTimeoutFile(const std::string &_log_path) {
        MutexGuard _guard(sg_mutex_dir_attr);
        time_t now_time = time(nullptr);

        if (FileExists(_log_path.c_str()) && FileStatus(_log_path.c_str()) == directory_file) {
            DIR *_dir = NULL;
            struct dirent *_pdirent = NULL;

            _dir = opendir(_log_path.c_str());
            if (NULL == _dir) {
                fprintf(stderr, "opendir %s failed,errno=%d", _log_path.c_str(), errno);
                return;
            }

            while (NULL != (_pdirent = readdir(_dir))) {
                if (_pdirent->d_type == DT_REG && FileExtension(_pdirent->d_name) == (std::string(".") + LOG_EXT)) {
                    time_t file_modify_time = LastWriteTime(_log_path.c_str());
                    if (now_time > file_modify_time && now_time - file_modify_time > max_alive_time_) {
                        RemoveFileOrDirectory(_log_path.c_str());
                    }
                }
            }
            closedir(_dir);
        }
    }

    void LogFile::_MoveOldFiles(const std::string& _src_path, const std::string& _dest_path,
                                         const std::string& _nameprefix) {
        MutexGuard _guard(sg_mutex_dir_attr);

        if (_src_path == _dest_path) return;

        if (!FileExists(_src_path.c_str()) || FileStatus(_src_path.c_str()) != directory_file) return;

        MutexGuard _lock_file(mutex_log_file_);
        time_t now_time = time(nullptr);

        DIR *_dir = NULL;
        struct dirent *_pdirent = NULL;

        _dir = opendir(_src_path.c_str());
        if (NULL == _dir) {
            fprintf(stderr, "opendir %s failed,errno=%d", _src_path.c_str(), errno);
            return;
        }

        while (NULL != (_pdirent = readdir(_dir))) {
            std::string _path(_pdirent->d_name);
            std::string _file_name = FileName(_path);
            if (!strutil::StartsWith(_file_name, _nameprefix) || !strutil::EndsWith(_file_name, LOG_EXT)) {
                continue;
            }

            if (config_.cache_days > 0) {
                time_t file_modify_time = LastWriteTime(_src_path.c_str());
                if (now_time > file_modify_time && (now_time - file_modify_time) < config_.cache_days * 24 * 60 * 60) {
                    continue;
                }
            }

            if (!AppendFile(_path, config_.logdir + "/" + _file_name)) {
                break;
            }

            RemoveFileOrDirectory(_path.c_str());
        }
        closedir(_dir);
    }

    bool LogFile::_CacheLogs() {
        if (config_.cachedir.empty() || config_.cache_days <= 0) {
            return false;
        }

        struct timeval tv;
        gettimeofday(&tv, nullptr);
        char logfilepath[1024] = {0};
        _MakeLogFileName(tv, config_.logdir, config_.nameprefix.c_str(), LOG_EXT, logfilepath , 1024);
        if (FileExists(logfilepath)) {
            return false;
        }

        static const uintmax_t kAvailableSizeThreshold = (uintmax_t)1 * 1024 * 1024 * 1024;   // 1G
        space_info info = Space(config_.cachedir.c_str());
        if (info.available < kAvailableSizeThreshold) {
            return false;
        }

        return true;
    }

    void LogFile::Log2File(const void *_data, size_t _len, bool _move_file) {
        if (nullptr == _data || 0 == _len || config_.logdir.empty()) {
            return;
        }

        MutexGuard _log_file(mutex_log_file_);

        //如果没有设置缓存目录，就不用考虑任何与缓存有关的情况，写完直接return,
        // 如果有缓存目录，就算不是优先写缓存，也可以在写正式文件失败后尝试写到缓存里，这些事后面的逻辑
        if (config_.cachedir.empty()) {
            if (_OpenLogFile(config_.logdir)) {
                _WriteFile(_data, _len, logfile_);
                if (kAppenderAsync == config_.mode) {
                    CloseLogFile();
                }
            }
            return;
        }

        struct timeval tv;
        gettimeofday(&tv, nullptr);
        char logcachefilepath[1024] = {0};

        _MakeLogFileName(tv, config_.cachedir, config_.nameprefix.c_str(), LOG_EXT, logcachefilepath , 1024);

        bool cache_logs = _CacheLogs();
        if ((cache_logs || FileExists(logcachefilepath)) && _OpenLogFile(config_.cachedir)) {
            _WriteFile(_data, _len, logfile_);
            if (kAppenderAsync == config_.mode) {
                CloseLogFile();
            }

            if (cache_logs || !_move_file) {
                return;
            }

            char logfilepath[1024] = {0};
            _MakeLogFileName(tv, config_.logdir, config_.nameprefix.c_str(), LOG_EXT, logfilepath , 1024);
            if (AppendFile(logcachefilepath, logfilepath)) {
                if (kAppenderSync == config_.mode) {
                    CloseLogFile();
                }
                RemoveFileOrDirectory(logcachefilepath);
            }
            return;
        }

        //没有写到缓存目录，尝试写到正式文件
        bool write_success = false;
        bool open_success = _OpenLogFile(config_.logdir);
        if (open_success) {
            write_success = _WriteFile(_data, _len, logfile_);
            if (kAppenderAsync == config_.mode) {
                CloseLogFile();
            }
        }

        //正式文件写失败，但有缓存目录，可以继续尝试写到缓存，多一道防护
        if (!write_success) {
            if (open_success && kAppenderSync == config_.mode) {
                CloseLogFile();
            }

            if (_OpenLogFile(config_.cachedir)) {
                _WriteFile(_data, _len, logfile_);
                if (kAppenderAsync == config_.mode) {
                    CloseLogFile();
                }
            }
        }
    }

    void LogFile::CloseLogFile() {
        MutexGuard _file_log(mutex_log_file_);
        if (nullptr == logfile_) return;

        openfiletime_ = 0;
        fclose(logfile_);
        logfile_ = nullptr;
    }

    bool LogFile::_OpenLogFile(const std::string &_log_dir) {
        if (config_.logdir.empty()) return false;

        struct timeval tv;
        gettimeofday(&tv, nullptr);

        if (nullptr != logfile_) {
            time_t sec = tv.tv_sec;
            tm tcur = *localtime((const time_t*)&sec);
            tm filetm = *localtime(&openfiletime_);

            if (filetm.tm_year == tcur.tm_year
                && filetm.tm_mon == tcur.tm_mon
                && filetm.tm_mday == tcur.tm_mday) {
                return true;
            }

            fclose(logfile_);
            logfile_ = nullptr;
        }

        uint64_t now_tick = gettickcount();
        time_t now_time = tv.tv_sec;

        openfiletime_ = tv.tv_sec;
        char logfilepath[1024] = {0};
        _MakeLogFileName(tv, _log_dir, config_.nameprefix.c_str(), LOG_EXT, logfilepath, 1024);

        if (now_time < last_time_) {
            logfile_ = fopen(last_file_path_, "ab");

            if (nullptr == logfile_) {
                WriteTips2Console("open file error:%d %s, path:%s", errno, strerror(errno), last_file_path_);
            }

            return nullptr != logfile_;
        }

        logfile_ = fopen(logfilepath, "ab");
        if (nullptr == logfile_) {
            WriteTips2Console("open file error:%d %s, path:%s", errno, strerror(errno), logfilepath);
        }

        if (0 != last_time_ && (now_time - last_time_) > (time_t)((now_tick - last_tick_) / 1000 + 300)) {

            struct tm tm_tmp = *localtime((const time_t*)&last_time_);
            char last_time_str[64] = {0};
            strftime(last_time_str, sizeof(last_time_str), "%Y-%m-%d %z %H:%M:%S", &tm_tmp);

            tm_tmp = *localtime((const time_t*)&now_time);
            char now_time_str[64] = {0};
            strftime(now_time_str, sizeof(now_time_str), "%Y-%m-%d %z %H:%M:%S", &tm_tmp);

            char log[1024] = {0};
            snprintf(log, sizeof(log), "[F][ last log file:%s from %s to %s, time_diff:%ld, tick_diff:%u\n",
                    last_file_path_, last_time_str, now_time_str, now_time-last_time_, now_tick-last_tick_);

            AutoBuffer tmp_buff;
            log_buff_->Write(log, strnlen(log, sizeof(log)), tmp_buff);
            _WriteFile(tmp_buff.Ptr(), tmp_buff.Length(), logfile_);
        }

        memcpy(last_file_path_, logfilepath, sizeof(last_file_path_));
        last_tick_ = now_tick;
        last_time_ = now_time;

        return nullptr != logfile_;
    }

    void LogFile::_MakeLogFileName(const timeval &_tv, const std::string &_logdir,
                                   const char *_prefix, const std::string &_fileext,
                                   char *_filepath, unsigned int _len) {
        long index = 0;
        std::string logfilenameprefix = MakeLogFileNamePrefix(_tv, _prefix);//prefix_20211101
        if (max_file_size_ > 0) {
            index = _GetNextFileIndex(logfilenameprefix, _fileext);
        }

        std::string logfilepath = _logdir;
        logfilepath += "/";
        logfilepath += logfilenameprefix;

        if (index > 0) {
            char temp[24] = {0};
            snprintf(temp, 24, "_%ld", index);
            logfilepath += temp;
        }

        logfilepath += ".";
        logfilepath += _fileext;

        strncpy(_filepath, logfilepath.c_str(), _len - 1);
        _filepath[_len - 1] = '\0';
    }

    static bool __string_compare_greater(const std::string& s1, const std::string& s2) {
        if (s1.length() == s2.length()) {
            return s1 > s2;
        }
        return s1.length() > s2.length();
    }

    long LogFile::_GetNextFileIndex(const std::string &_fileprefix, const std::string &_fileext) {
        std::vector<std::string> filename_vec;
        _GetFileNamesByPrefix(config_.logdir, _fileprefix, _fileext, filename_vec);
        if (!config_.cachedir.empty()) {
            _GetFileNamesByPrefix(config_.cachedir, _fileprefix, _fileext, filename_vec);
        }

        long index = 0; // long is enought to hold all indexes in one day.
        if (filename_vec.empty()) {
            return index;
        }

        // high -> low
        std::sort(filename_vec.begin(), filename_vec.end(), __string_compare_greater);
        std::string last_filename = *(filename_vec.begin());
        std::size_t ext_pos = last_filename.rfind("." + _fileext);
        std::size_t index_len = ext_pos - _fileprefix.length();
        if (index_len > 0) {
            std::string index_str = last_filename.substr(_fileprefix.length(), index_len);
            if (strutil::StartsWith(index_str, "_")) {
                index_str = index_str.substr(1);
            }
            index = atol(index_str.c_str());
        }

        uint64_t filesize = 0;
        std::string logfilepath = config_.logdir + "/" + last_filename;
        if (FileExists(logfilepath.c_str())) {
            filesize += FileSize(logfilepath);
        }

        if (!config_.cachedir.empty()) {
            logfilepath = config_.cachedir + "/" + last_filename;
            if (FileExists(logfilepath.c_str())) {
                filesize += FileSize(logfilepath);
            }
        }
        return (filesize > max_file_size_) ? index + 1 : index;
    }

    void LogFile::_GetFileNamesByPrefix(const std::string &_logdir, const std::string &_fileprefix,
                                        const std::string &_fileext,
                                        std::vector<std::string> &_filename_vec) {
        if (directory_file != FileStatus(_logdir.c_str())) {
            return;
        }

        std::string filename;
        DIR *_dir = NULL;
        struct dirent *_pdirent = NULL;

        _dir = opendir(_logdir.c_str());
        if (NULL == _dir) {
            fprintf(stderr, "opendir %s failed,errno=%d", _logdir.c_str(), errno);
            return;
        }

        while (NULL != (_pdirent = readdir(_dir))) {
            std::string _path(_pdirent->d_name);
            std::string _file_name = FileName(_path);
            if (strutil::StartsWith(_file_name, _fileprefix) && strutil::EndsWith(_file_name, LOG_EXT)) {
                _filename_vec.push_back(_file_name);
            }
        }
        closedir(_dir);
    }

    bool LogFile::_WriteFile(const void *_data, size_t _len, FILE *_file) {
        if (nullptr == _file) {
            assert(false);
            return false;
        }

        long before_len = ftell(_file);
        if (before_len < 0) return false;

        if (1 != fwrite(_data, _len, 1, _file)) {
            int err = ferror(_file);

            WriteTips2Console("write file error:%d", err);

            ftruncate(fileno(_file), before_len);
            fseek(_file, 0, SEEK_END);

            char err_log[256] = {0};
            snprintf(err_log, sizeof(err_log), "\nwrite file error:%d\n", err);

            AutoBuffer tmp_buff;
            log_buff_->Write(err_log, strnlen(err_log, sizeof(err_log)), tmp_buff);

            fwrite(tmp_buff.Ptr(), tmp_buff.Length(), 1, _file);

            return false;
        }

        return true;
    }

    void LogFile::SetMaxFileSize(uint64_t _max_byte_size) {
        max_file_size_ = _max_byte_size;
    }

    void LogFile::SetMode(AppenderMode _mode) {
        MutexGuard _file_lock(mutex_log_file_);
        config_.mode = _mode;
    }
}