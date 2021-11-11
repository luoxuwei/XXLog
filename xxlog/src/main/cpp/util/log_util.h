//
// Created by 罗旭维 on 2021/11/9.
//

#ifndef XXLOG_LOG_UTIL_H
#define XXLOG_LOG_UTIL_H
#include "config.h"

namespace xxlog {
    const char* ExtractFileName(const char* _path);
    void ConsoleLog(const XXLoggerInfo* _info, const char* _log);
    void WriteTips2Console(const char* _tips_format, ...);
    uint64_t gettickcount();
    std::string MakeLogFileNamePrefix(const timeval& _tv, const char* _prefix);
}

#endif //XXLOG_LOG_UTIL_H
