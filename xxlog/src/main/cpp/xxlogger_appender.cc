//
// Created by 罗旭维 on 2021/11/9.
//
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <util/log_util.h>
#include "xxlogger_appender.h"
#include "file_operations.h"
#include "log_zlib_buffer.h"
#include "strutil.h"



namespace xxlog {

    static const unsigned int kBufferBlockLength = 150 * 1024;


    XXLoggerAppender::XXLoggerAppender(const XXLogConfig &_config)
    : thread_async_(std::bind(&XXLoggerAppender::_AsyncLogThread, this), "AsyncLogging")
    , cond_buffer_async_(mutex_buffer_async_), log_file_(_config) {
        Open(_config);
    }

    void XXLoggerAppender::Write(const XXLoggerInfo *_info, const char *_log) {
        if (log_close_) return;

        if (kAppenderSync == config_.mode)
            _WriteSync(_info, _log);
        else
            _WriteAsync(_info, _log);
    }

    void XXLoggerAppender::SetMaxAliveDuration(long _max_time) {
        log_file_.SetMaxAliveDuration(_max_time);
    }

    void XXLoggerAppender::SetMode(AppenderMode _mode) {
        log_file_.SetMode(_mode);
        MutexGuard _guard(mutex_buffer_async_);
        cond_buffer_async_.NotifyAll();
        if (kAppenderAsync == config_.mode && !thread_async_.IsStarted()) {
            thread_async_.Start();
        }
    }

    void XXLoggerAppender::Open(const XXLogConfig &_config) {
        config_ = _config;

        log_file_.Open();

        uint64_t tick = gettickcount();

        bool use_mmap = false;
        char mmap_file_path[512] = {0};
        snprintf(mmap_file_path, sizeof(mmap_file_path), "%s/%s.mmap3",
                 config_.cachedir.empty()?config_.logdir.c_str():config_.cachedir.c_str(), config_.nameprefix.c_str());

        if (OpenMmapFile(mmap_file_path, kBufferBlockLength, &mmap_buf_, &mmap_fd_)) {
            log_buff_ = new mars::xlog::LogZlibBuffer(mmap_buf_, kBufferBlockLength, true, _config.pub_key.c_str());
            use_mmap = true;
        } else {
            char* buffer = new char[kBufferBlockLength];
            log_buff_ = new mars::xlog::LogZlibBuffer(buffer, kBufferBlockLength, true, _config.pub_key.c_str());
            use_mmap = false;
        }

        if (NULL == log_buff_->GetData().Ptr()) {
            if (use_mmap && _IsMMapFileOpen()) CloseMmapFile(&mmap_fd_, mmap_buf_, (int)kBufferBlockLength);
            return;
        }

        AutoBuffer buffer;
        log_buff_->Flush(buffer);

        {
            MutexGuard _lock(mutex_buffer_async_);
            log_close_ = false;
        }

        SetMode(config_.mode);

        char mark_info[512] = {0};
        GetMarkInfo(mark_info, sizeof(mark_info));

        if (buffer.Ptr()) {
            WriteTips2File("~~~~~ begin of mmap ~~~~~\n");
            log_file_.Log2File(buffer.Ptr(), buffer.Length(), false);
            WriteTips2File("~~~~~ end of mmap ~~~~~%s\n", mark_info);
        }

        uint64_t get_mmap_time = gettickcount() - tick;

        char appender_info[728] = {0};
        snprintf(appender_info, sizeof(appender_info), "^^^^^^^^^^" __DATE__ "^^^" __TIME__ "^^^^^^^^^^%s", mark_info);

        Write(nullptr, appender_info);

        char logmsg[256] = {0};
        snprintf(logmsg, sizeof(logmsg), "get mmap time: %llu", (int64_t)get_mmap_time);
        Write(nullptr, logmsg);

        snprintf(logmsg, sizeof(logmsg), "log appender mode:%d, use mmap:%d", (int)config_.mode, use_mmap);
        Write(nullptr, logmsg);

        if (!config_.cachedir.empty()) {
            space_info info = Space(config_.cachedir.c_str());
            snprintf(logmsg, sizeof(logmsg), "cache dir space info, capacity:%llu free:%llu available:%llu", info.capacity, info.free, info.available);
            Write(nullptr, logmsg);
        }

        space_info info = Space(config_.logdir.c_str());
        snprintf(logmsg, sizeof(logmsg), "log dir space info, apacity:%llu free:%llu available:%llu", info.capacity, info.free, info.available);
        Write(nullptr, logmsg);
    }

    bool XXLoggerAppender::_IsMMapFileOpen() {
        return (mmap_buf_ != nullptr && mmap_fd_ > 0);
    }

    void XXLoggerAppender::Flush() {
        cond_buffer_async_.NotifyAll();
    }

    void XXLoggerAppender::FlushSync() {
        if (kAppenderSync == config_.mode) {
            return;
        }

        AutoBuffer tmp;
        {
            MutexGuard lock_buffer(mutex_buffer_async_);

            if (nullptr == log_buff_) return;

            log_buff_->Flush(tmp);

        }

        if (tmp.Ptr())  log_file_.Log2File(tmp.Ptr(), tmp.Length(), false);
    }

    void XXLoggerAppender::Close() {
        if (log_close_) return;

        char mark_info[512] = {0};
        GetMarkInfo(mark_info, sizeof(mark_info));
        char appender_info[728] = {0};
        snprintf(appender_info, sizeof(appender_info), "$$$$$$$$$$" __DATE__ "$$$" __TIME__ "$$$$$$$$$$%s\n", mark_info);
        Write(nullptr, appender_info);

        log_close_ = true;

        cond_buffer_async_.NotifyAll();

        if (thread_async_.IsStarted()) {
            thread_async_.Join();
        }

        {
            MutexGuard buffer_lock(mutex_buffer_async_);
            if (_IsMMapFileOpen()) {
                memset(mmap_buf_, 0, kBufferBlockLength);
                CloseMmapFile(&mmap_fd_, mmap_buf_, (int)kBufferBlockLength);
            } else {
                if (nullptr != log_buff_) {
                    delete[] (char*)((log_buff_->GetData()).Ptr());
                }
            }

            delete log_buff_;
            log_buff_ = nullptr;
        }

        log_file_.CloseLogFile();
    }

    void XXLoggerAppender::_WriteSync(const XXLoggerInfo *_info, const char *_log) {
        char temp[16 * 1024] = {0};
        PtrBuffer log(temp, 0, sizeof(temp));
        log_formater(_info, _log, log);

        AutoBuffer tmp_buff;
        if (!log_buff_->Write(log.Ptr(), log.Length(), tmp_buff)) return;
        log_file_.Log2File(tmp_buff.Ptr(), tmp_buff.Length(), false);
    }

    void XXLoggerAppender::SetMaxFileSize(uint64_t _max_byte_size) {
        log_file_.SetMaxFileSize(_max_byte_size);
    }

    void XXLoggerAppender::_WriteAsync(const XXLoggerInfo *_info, const char *_log) {
        char temp[16*1024] = {0}; //tell perry,ray if you want modify size.
        PtrBuffer log_buff(temp, 0, sizeof(temp));
        log_formater(_info, _log, log_buff);

        MutexGuard lock(mutex_buffer_async_);
        if (nullptr == log_buff_) return;

        if (log_buff_->GetData().Length() >= kBufferBlockLength*4/5) {
            int ret = snprintf(temp, sizeof(temp), "[F][ sg_buffer_async.Length() >= BUFFER_BLOCK_LENTH*4/5, len: %d\n", (int)log_buff_->GetData().Length());
            log_buff.Length(ret, ret);
        }

        if (!log_buff_->Write(log_buff.Ptr(), log_buff.Length())) return;

        if (log_buff_->GetData().Length() >= kBufferBlockLength*1/3 || (nullptr!=_info && kLevelFatal == _info->level)) {
            cond_buffer_async_.NotifyAll();
        }
    }

    void XXLoggerAppender::_AsyncLogThread() {
        while (true) {
            AutoBuffer tmp;
            {
                MutexGuard lock_buffer(mutex_buffer_async_);

                if (nullptr == log_buff_) break;

                log_buff_->Flush(tmp);
            }

            if (nullptr != tmp.Ptr()) log_file_.Log2File(tmp.Ptr(), tmp.Length(), true);

            if (log_close_) break;

            cond_buffer_async_.WaitForSeconds(15 * 60 * 1000);
        }

    }

    void XXLoggerAppender::WriteTips2File(const char *_tips_format, ...) {
        if (nullptr == _tips_format) {
            return;
        }

        char tips_info[4096] = {0};
        va_list ap;
        va_start(ap, _tips_format);
        vsnprintf(tips_info, sizeof(tips_info), _tips_format, ap);
        va_end(ap);

        AutoBuffer tmp_buff;
        log_buff_->Write(tips_info, strnlen(tips_info, sizeof(tips_info)), tmp_buff);

        log_file_.Log2File(tmp_buff.Ptr(), tmp_buff.Length(), false);
    }
}