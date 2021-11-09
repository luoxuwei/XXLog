//
// Created by 罗旭维 on 2021/11/7.
//

#ifndef XXLOG_LOGGER_H
#define XXLOG_LOGGER_H
#include "config.h"
#include "xxlogger_appender.h"

namespace xxlog {
    class Logger {
    public:
        void SetLogLevel(LogLevel level);
        LogLevel GetLogLevel();
        bool IsEnabledFor(LogLevel level);
        void SetAppenderMode(int _mode);
        void SetConsoleLogOpen(bool _isOpen);
        bool ConsoleLogOpen();
        void Open(const XXLogConfig &_config);
        void Close();
        void Flush();
        void SetMaxFileSize(long _size);
        void SetMaxAliveTime(long _aliveSeconds);
        void Write(const XXLoggerInfo &_loggerInfo, const char *_log);
        ~Logger();
    private:
        LogLevel log_level_ = kLevelNone;
        bool consolelog_open_ = false;
        XXLoggerAppender *xxlogger_appender_;
    };
}

#endif //XXLOG_LOGGER_H
