//
// Created by 罗旭维 on 2021/11/10.
//

#include "config.h"
#include "util/ptrbuffer.h"
#include <string>
#include <util/log_util.h>

namespace xxlog {
    void log_formater(const XXLoggerInfo* _info, const char* _logbody, PtrBuffer& _log) {
        static const char* levelStrings[] = {
                "V",
                "D",  // debug
                "I",  // info
                "W",  // warn
                "E",  // error
                "F"  // fatal
        };

        static int error_count = 0;
        static int error_size = 0;

        if (_log.MaxLength() <= _log.Length() + 5 * 1024) {// allowed len(_log) <= 11K(16K - 5K)
            ++error_count;
            error_size = (int) strnlen(_logbody, 1024 * 1024);
            if (_log.MaxLength() >= _log.Length() + 128) {
                int ret = snprintf((char *)_log.Ptr(), 1024, "[F]log_size <= 5*1024, err(%d, %d)\n", error_count, error_size);  // **CPPLINT SKIP**
                _log.Length(_log.Pos() + ret, _log.Length() + ret);
                _log.Write("");
                error_count = 0;
                error_size = 0;
            }

            return;
        }

        if (NULL != _info) {
            const char* filename = ExtractFileName(_info->filename);
            const char* strFuncName = NULL == _info->func_name ? "" : _info->func_name;
            char temp_time[64] = {0};
            if (_info->timeval.tv_sec) {
                time_t sec = _info->timeval.tv_sec;
                tm tm = *localtime((const time_t*)&sec);
                snprintf(temp_time, sizeof(temp_time), "%d-%02d-%02d %+.1f %02d:%02d:%02d.%.3d", 1900 + tm.tm_year, 1 + tm.tm_mon, tm.tm_mday,
                         tm.tm_gmtoff / 3600.0, tm.tm_hour, tm.tm_min, tm.tm_sec, _info->timeval.tv_usec / 1000);
            }
            //[I][2012-11-11 0.0 21:30:12.011][11,21*][xlog][xlog.java:11, Write2][
            int ret = snprintf((char*)_log.PosPtr(), 1024, "[%s][%s][%d,%d%s][%s][%s:%d, %s][",  // **CPPLINT SKIP**
                    _logbody ? levelStrings[_info->level] : levelStrings[kLevelFatal], temp_time,
                    _info->pid, _info->tid, _info->tid == _info->maintid ? "*" : "", _info->tag ? _info->tag : "",
                    filename, _info->line, strFuncName);
            _log.Length(_log.Pos() + ret, _log.Length() + ret);
        }

        if (NULL != _logbody) {
            // in android 64bit, in strnlen memchr,  const unsigned char*  end = p + n;  > 4G!!!!! in stack array

            size_t bodylen =  _log.MaxLength() - _log.Length() > 130 ? _log.MaxLength() - _log.Length() - 130 : 0;
            bodylen = bodylen > 0xFFFFU ? 0xFFFFU : bodylen;
            bodylen = strnlen(_logbody, bodylen);
            bodylen = bodylen > 0xFFFFU ? 0xFFFFU : bodylen;
            _log.Write(_logbody, bodylen);
        } else {
            _log.Write("error!! NULL==_logbody");
        }
        char nextline = '\n';

        if (*((char*)_log.PosPtr() - 1) != nextline) _log.Write(&nextline, 1);
    }
}