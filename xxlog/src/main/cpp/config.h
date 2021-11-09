//
// Created by 罗旭维 on 2021/11/8.
//

#ifndef XXLOG_CONFIG_H
#define XXLOG_CONFIG_H
#include <string>
namespace xxlog {
    enum AppenderMode {
        kAppenderAsync,
        kAppenderSync,
    };

    enum CompressMode {
        KZlib,
    };

    struct XXLogConfig {
        AppenderMode mode = kAppenderAsync;
        std::string logdir;
        std::string nameprefix;
        std::string pub_key;
        CompressMode compress_mode = KZlib;
        int compress_level = 6;
        std::string cachedir;
        int cache_days = 0;
    };

    typedef enum {
        kLevelAll = 0,
        kLevelVerbose = 0,
        kLevelDebug,    // Detailed information on the flow through the system.
        kLevelInfo,     // Interesting runtime events (startup/shutdown), should be conservative and keep to a minimum.
        kLevelWarn,     // Other runtime situations that are undesirable or unexpected, but not necessarily "wrong".
        kLevelError,    // Other runtime errors or unexpected conditions.
        kLevelFatal,    // Severe errors that cause premature termination.
        kLevelNone,     // Special level used to disable all log messages.
    } LogLevel;

    typedef struct {
        LogLevel level;
        const char *tag;
        const char *filename;
        const char *func_name;
        int line;

        struct timeval timeval;
        intmax_t pid;
        intmax_t tid;
        intmax_t maintid;
    } XXLoggerInfo;
}
#endif //XXLOG_CONFIG_H
