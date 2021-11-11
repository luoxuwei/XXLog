//
// Created by 罗旭维 on 2021/11/9.
//

#ifndef XXLOG_XXLOGGERAPPENDER_H
#define XXLOG_XXLOGGERAPPENDER_H
#include "config.h"
#include "thread.h"
#include "log_base_buffer.h"
#include "log_file.h"
namespace xxlog {

    class XXLoggerAppender {
    public:
        XXLoggerAppender(const XXLogConfig &_config);
        void SetMaxAliveDuration(long _max_time);
        void SetConsoleLog(bool _is_open);
        void Open(const XXLogConfig &_config);
        void Write(const XXLoggerInfo &_info, const char *_log);
        void _WriteSync(const XXLoggerInfo &_info, const char *_log);
        void _WriteAsync(const XXLoggerInfo &_info, const char *_log);
        void SetMode(AppenderMode _mode);
        void SetMaxFileSize(uint64_t _max_byte_size);
        void Close();
        void Flush();

    private:
        void _AsyncLogThread();

    private:
        Thread thread_async_;
        LogFile log_file_;
        Mutex mutex_buffer_async_;
        XXLogConfig config_;
        bool consolelog_open_ = false;
        bool log_close_ = true;
        Condition cond_buffer_async_;
        mars::xlog::LogBaseBuffer* log_buff_ = nullptr;

    };

    void log_formater(const XXLoggerInfo* _info, const char* _logbody, PtrBuffer& _log);
}

#endif //XXLOG_XXLOGGERAPPENDER_H
