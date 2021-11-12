//
// Created by 罗旭维 on 2021/11/9.
//

#ifndef XXLOG_LOG_UTIL_H
#define XXLOG_LOG_UTIL_H
#include "config.h"

namespace xxlog {
    struct space_info
    {
        // all values are byte counts
        uintmax_t capacity;
        uintmax_t free;      // <= capacity
        uintmax_t available; // <= free
    };

    const char* ExtractFileName(const char* _path);
    void ConsoleLog(const XXLoggerInfo* _info, const char* _log);
    void WriteTips2Console(const char* _tips_format, ...);
    uint64_t gettickcount();
    std::string MakeLogFileNamePrefix(const timeval& _tv, const char* _prefix);
    void GetMarkInfo(char* _info, size_t _info_len);
    space_info Space(const char *path);
}

#endif //XXLOG_LOG_UTIL_H
