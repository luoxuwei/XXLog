//
// Created by 罗旭维 on 2021/11/7.
//
#include "config.h"
#include "logger.h"
#include "log_util.h"
namespace xxlog {

    Logger::~Logger() {
        if (NULL != xxlogger_appender_ ) {
            xxlogger_appender_->Close();
            delete xxlogger_appender_;
            xxlogger_appender_ = NULL;
        }
    }

    void Logger::SetLogLevel(LogLevel level) {
        log_level_ = level;
    }

    LogLevel Logger::GetLogLevel() {
        return log_level_;
    }

    bool  Logger::IsEnabledFor(LogLevel level) {
        return log_level_ <= level;
    }

    void Logger::Open(const XXLogConfig &_config) {
        xxlogger_appender_ = new XXLoggerAppender(_config);
    }

    void Logger::Write(const XXLoggerInfo &_loggerInfo, const char *_log) {
        if (consolelog_open_) ConsoleLog(&_loggerInfo, _log);
    }

    void Logger::SetConsoleLogOpen(bool _isOpen) {
        consolelog_open_ = _isOpen;
    }

    bool Logger::ConsoleLogOpen() {
        return consolelog_open_;
    }

    void Logger::Flush() {
        xxlogger_appender_->Flush();
    }

}