//
// Created by 罗旭维 on 2021/11/9.
//
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
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

    void XXLoggerAppender::Write(const XXLoggerInfo &_info, const char *_log) {
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
        MutexGuard _guard(mutex_buffer_async_);
        cond_buffer_async_.NotifyAll();
        if (kAppenderAsync == config_.mode && !thread_async_.IsStarted()) {
            thread_async_.Start();
        }
    }

    void XXLoggerAppender::Open(const XXLogConfig &_config) {
        config_ = _config;

        log_file_.Open();

        bool use_mmap = false;
        char mmap_file_path[512] = {0};
        snprintf(mmap_file_path, sizeof(mmap_file_path), "%s/%s.mmap3",
                 config_.cachedir.empty()?config_.logdir.c_str():config_.cachedir.c_str(), config_.nameprefix.c_str());
        char *_buf = NULL;
        int _fd = -1;
        if (OpenMmapFile(mmap_file_path, kBufferBlockLength, &_buf, &_fd)) {
            log_buff_ = new mars::xlog::LogZlibBuffer(_buf, kBufferBlockLength, true, _config.pub_key.c_str());
            use_mmap = true;
        } else {
            char* buffer = new char[kBufferBlockLength];
            log_buff_ = new mars::xlog::LogZlibBuffer(buffer, kBufferBlockLength, true, _config.pub_key.c_str());
            use_mmap = false;
        }

        if (NULL == log_buff_->GetData().Ptr()) {
            if (use_mmap && (_buf != NULL && _fd > 0)) CloseMmapFile(&_fd, _buf, (int)kBufferBlockLength);
            return;
        }

        AutoBuffer buffer;
        log_buff_->Flush(buffer);

        {
            MutexGuard _lock(mutex_log_file_);
            log_close_ = false;
            SetMode(config_.mode);
        }
    }



    void XXLoggerAppender::Flush() {

    }

    void XXLoggerAppender::Close() {

    }

    void XXLoggerAppender::_WriteAsync(const XXLoggerInfo &_info, const char *_log) {

    }

    void XXLoggerAppender::_WriteSync(const XXLoggerInfo &_info, const char *_log) {
        char temp[16 * 1024] = {0};
        PtrBuffer log(temp, 0, sizeof(temp));
        log_formater(&_info, _log, log);

        AutoBuffer tmp_buff;
        if (!log_buff_->Write(log.Ptr(), log.Length(), tmp_buff)) return;
    }

    void XXLoggerAppender::SetConsoleLog(bool _is_open) {
        consolelog_open_ = _is_open;
    }
}