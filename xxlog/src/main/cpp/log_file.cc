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
#include "log_file.h"

#define LOG_EXT "xlog"

namespace xxlog {
    static Mutex sg_mutex_dir_attr; //log目录的锁
    static const long kMinLogAliveTime = 24 * 60 * 60;    // 1 days in second

    LogFile::LogFile(const XXLogConfig &_config) : config_(_config) {

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

            if (!_AppendFile(_path, config_.logdir + "/" + _file_name)) {
                break;
            }

            RemoveFileOrDirectory(_path.c_str());
        }
        closedir(_dir);
    }

    bool LogFile::_AppendFile(const std::string& _src_file, const std::string& _dst_file) {
        if (_src_file == _dst_file) {
            return false;
        }

        if (!FileExists(_src_file.c_str())) {
            return false;
        }

        if (0 >= FileSize(_src_file)) {
            return true;
        }

        FILE* src_file = fopen(_src_file.c_str(), "rb");

        if (nullptr == src_file) {
            return false;
        }

        FILE* dest_file = fopen(_dst_file.c_str(), "ab");

        if (nullptr == dest_file) {
            fclose(src_file);
            return false;
        }

        fseek(src_file, 0, SEEK_END);
        long src_file_len = ftell(src_file);
        long dst_file_len = ftell(dest_file);
        fseek(src_file, 0, SEEK_SET);

        char buffer[4096] = {0};

        while (true) {
            if (feof(src_file)) break;

            size_t read_ret = fread(buffer, 1, sizeof(buffer), src_file);

            if (read_ret == 0)   break;

            if (ferror(src_file)) break;

            fwrite(buffer, 1, read_ret, dest_file);

            if (ferror(dest_file))  break;
        }

        if (dst_file_len + src_file_len > ftell(dest_file)) {
            ftruncate(fileno(dest_file), dst_file_len);
            fclose(src_file);
            fclose(dest_file);
            return false;
        }

        fclose(src_file);
        fclose(dest_file);

        return true;
    }

    void LogFile::Log2File(const void *_data, size_t _len, bool _move_file) {
        if (nullptr == _data || 0 == _len || config_.logdir.empty()) {
            return;
        }
    }
}