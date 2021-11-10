//
// Created by 罗旭维 on 2021/11/9.
//
#include <stdio.h>
#include <string.h>
#include <android/log.h>
#include "config.h"
#include "log_util.h"
namespace xxlog {

    //这里不能加日志，会导致循环调用
    void ConsoleLog(const XXLoggerInfo* _info, const char* _log) {
        char result_log[16*1024] = {0};
        if (_info) {
            const char* filename = ExtractFileName(_info->filename);
            const char* strFuncName  = NULL == _info->func_name ? "" : _info->func_name;

            snprintf(result_log,  sizeof(result_log), "[%s:%d, %s]:%s", filename, _info->line, strFuncName, _log?_log:"NULL==log!!!");
            __android_log_write(_info->level+2, _info->tag?_info->tag:"", (const char*)result_log);
        } else {
            snprintf(result_log,  sizeof(result_log) , "%s", _log?_log:"NULL==log!!!");
            __android_log_write(ANDROID_LOG_WARN, "", (const char*)result_log);
        }

    }

    void WriteTips2Console(const char* _tips_format, ...) {

        if (nullptr == _tips_format) {
            return;
        }

        XXLoggerInfo info;
        memset(&info, 0, sizeof(XXLoggerInfo));
        info.level = kLevelError;

        char tips_info[4096] = {0};
        va_list ap;
        va_start(ap, _tips_format);
        vsnprintf(tips_info, sizeof(tips_info), _tips_format, ap);
        va_end(ap);
        ConsoleLog(&info, tips_info);
    }

    const char* ExtractFileName(const char* _path) {
        if (NULL == _path) return "";

        const char* pos = strrchr(_path, '\\');

        if (NULL == pos) {
            pos = strrchr(_path, '/');
        }

        if (NULL == pos || '\0' == *(pos + 1)) {
            return _path;
        } else {
            return pos + 1;
        }
    }

}