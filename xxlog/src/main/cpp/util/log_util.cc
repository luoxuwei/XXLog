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

    /*
     * linux系统取系统时间的api函数有两个gettimeofday和clock_gettime
     *区别：
      1、clock_gettime 相比 gettimeofday的精度更高一些，前者精度到 纳秒，而后者精度到微秒。
      2、clock_gettime可以通过 时钟选项而 得到不同参考下的时间，而gettimeofday则只有一种用途（获取当前系统时间）。
      常规应用下，使用gettimeofday 即可获取 当前系统时间，对精度要求高，而且有不同需求的，可以使用clock_gettime
     * */
    //CLOCK_MONOTONIC 获取系统启动后运行的时间
    uint64_t gettickcount() {//todoyy
        struct timespec ts;
        if (0==clock_gettime(CLOCK_MONOTONIC, &ts)){
            return (ts.tv_sec * 1000ULL + ts.tv_nsec / 1000000);
        }
        return 0;
    }

    std::string MakeLogFileNamePrefix(const timeval& _tv, const char* _prefix) {
        time_t sec = _tv.tv_sec;
        tm tcur = *localtime((const time_t*)&sec);

        char temp [64] = {0};
        snprintf(temp, 64, "_%d%02d%02d", 1900 + tcur.tm_year, 1 + tcur.tm_mon, tcur.tm_mday);

        std::string filenameprefix = _prefix;
        filenameprefix += temp;

        return filenameprefix;
    }

}