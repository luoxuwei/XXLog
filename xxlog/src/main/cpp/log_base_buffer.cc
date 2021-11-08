// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.

/*
 * log_base_buffer.cpp
 *
 *  Created on: 2015-7-28
 *      Author: yanguoyue
 */

#include <cstdio>
#include <time.h>
#include <algorithm>
#include <sys/time.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <stdio.h>

#include "log_base_buffer.h"
#include "log_crypt.h"
#include "log_magic_num.h"

#ifdef WIN32
#define snprintf _snprintf
#endif

namespace mars {
namespace xlog {

bool LogBaseBuffer::GetPeriodLogs(const char* _log_path, int _begin_hour, int _end_hour, unsigned long& _begin_pos, unsigned long& _end_pos, std::string& _err_msg) {

    char msg[1024] = {0};
    char magic_end = LogMagicNum::kMagicEnd;


    if (NULL == _log_path || _end_hour <= _begin_hour) {
        snprintf(msg, sizeof(msg), "NULL == _logPath || _endHour <= _beginHour, %d, %d", _begin_hour, _end_hour);
        _err_msg += msg;
        return false;
    }

    FILE* file = fopen(_log_path, "rb");
    if (NULL == file) {
        snprintf(msg, sizeof(msg), "open file fail:%s", strerror(errno));
        _err_msg += msg;
        return false;
    }

    if (0 != fseek(file, 0, SEEK_END)) {
        snprintf(msg, sizeof(msg), "fseek(file, 0, SEEK_END):%s", strerror(ferror(file)));
        _err_msg += msg;
        fclose(file);
        return false;
    }

    long file_size = ftell(file);

    if (0 != fseek(file, 0, SEEK_SET)) {
        snprintf(msg, sizeof(msg), "fseek(file, 0, SEEK_SET) error:%s", strerror(ferror(file)));
        _err_msg += msg;
        fclose(file);
        return false;
    }

    _begin_pos = _end_pos = 0;

    bool find_begin_pos = false;
    int last_end_hour = -1;
    unsigned long last_end_pos = 0;

    char* header_buff = new char[LogCrypt::GetHeaderLen()];

    while (!feof(file) && !ferror(file)) {

        if ((long)(ftell(file) + LogCrypt::GetHeaderLen() + LogCrypt::GetTailerLen()) > file_size) {
            snprintf(msg, sizeof(msg), "ftell(file) + __GetHeaderLen() + sizeof(kMagicEnd)) > file_size error");
            break;
        }

        long before_len = ftell(file);
        if (LogCrypt::GetHeaderLen() != fread(header_buff, 1, LogCrypt::GetHeaderLen(), file)) {
            snprintf(msg, sizeof(msg), "fread(buff.Ptr(), 1, __GetHeaderLen(), file) error:%s, before_len:%ld.", strerror(ferror(file)), before_len);
            break;
        }


        bool fix = false;

        char start = *header_buff;
        if (!LogMagicNum::MagicStartIsValid(start)) {
            fix = true;
        } else {
            uint32_t len = LogCrypt::GetLogLen(header_buff, LogCrypt::GetHeaderLen());
            if ((long)(ftell(file) + len + sizeof(magic_end)) > file_size) {
                fix = true;
            } else {
                if (0 != fseek(file, len, SEEK_CUR)) {
                    snprintf(msg, sizeof(msg), "fseek(file, len, SEEK_CUR):%s, before_len:%ld, len:%u.", strerror(ferror(file)), before_len, len);
                    break;
                }
                char end;
                if (1 != fread(&end, 1, 1, file)) {
                    snprintf(msg, sizeof(msg), "fread(&end, 1, 1, file) err:%s, before_len:%ld, len:%u.", strerror(ferror(file)), before_len, len);
                    break;
                }
                if (end != magic_end) {
                    fix = true;
                }
            }
        }

        if (fix) {
            if (0 !=fseek(file, before_len+1, SEEK_SET)) {
                snprintf(msg, sizeof(msg), "fseek(file, before_len+1, SEEK_SET) err:%s, before_len:%ld.", strerror(ferror(file)), before_len);
                break;
            }
            continue;
        }


        int begin_hour = 0;
        int end_hour = 0;
        if (!LogCrypt::GetLogHour(header_buff, LogCrypt::GetHeaderLen(), begin_hour, end_hour)) {
            snprintf(msg, sizeof(msg), "__GetLogHour(buff.Ptr(), buff.Length(), beginHour, endHour) err, before_len:%ld.", before_len);
            break;
        }

        if (begin_hour > end_hour)  begin_hour = end_hour;

        if (_err_msg.size() < 1024 * 1024) {
            _err_msg += std::to_string(begin_hour) + "-" + std::to_string(end_hour) + " ";
        }

        if (!find_begin_pos) {
            if (_begin_hour > begin_hour && _begin_hour <= end_hour) {
                _begin_pos = before_len;
                find_begin_pos = true;
            }

            if (_begin_hour > last_end_hour && _begin_hour <= begin_hour) {
                _begin_pos = before_len;
                find_begin_pos = true;
            }
        }

        if (find_begin_pos) {
            if (_end_hour > begin_hour && _end_hour <= end_hour) {
                _end_pos = ftell(file);
            }

            if (_end_hour > last_end_hour && _end_hour <= begin_hour) {
                _end_pos = last_end_pos;
            }
        }

        last_end_hour = end_hour;
        last_end_pos = ftell(file);
    }

    delete[] header_buff;

    if (find_begin_pos && _end_hour > last_end_hour) {
        _end_pos = file_size;
    }

    fclose(file);

    bool ret = _end_pos > _begin_pos;

    if (!ret) {
        _err_msg += msg;
        memset(msg, 0, sizeof(msg));
        snprintf(msg, sizeof(msg), "begintpos:%lu, endpos:%lu, filesize:%ld.", _begin_pos, _end_pos, file_size);
        _err_msg += msg;
    }

    return ret;
    
}

LogBaseBuffer::LogBaseBuffer(void* _pbuffer, size_t _len, bool _isCompress, const char* _pubkey)
: is_compress_(_isCompress), log_crypt_(new LogCrypt(_pubkey))
, is_crypt_(log_crypt_->IsCrypt()) {
    buff_.Attach(_pbuffer, _len);
    __Fix();//如果buff_中有加密过的数据，就提取出seq和len，并更新buff_的pos和lenght
}

LogBaseBuffer::~LogBaseBuffer() {
    delete log_crypt_;
}

PtrBuffer& LogBaseBuffer::GetData() {
    return buff_;
}

void LogBaseBuffer::Flush(AutoBuffer& _buff) {
    if (log_crypt_->GetLogLen((char*)buff_.Ptr(), buff_.Length()) == 0) {
        __Clear();
        return;
    }
    
    __Flush();
    _buff.Write(buff_.Ptr(), buff_.Length());
    
    __Clear();
}

bool LogBaseBuffer::Write(const void* _data, size_t _inputlen, AutoBuffer& _out_buff) {
    if (NULL == _data || 0 == _inputlen) {
        return false;
    }

    log_crypt_->CryptSyncLog((char*) _data, _inputlen, _out_buff, __GetMagicSyncStart(), __GetMagicEnd());

    return true;
}

bool LogBaseBuffer::Write(const void* _data, size_t _length) {
    if (NULL == _data || 0 == _length) {
        return false;
    }

    //写入头部信息, 在_Flush方法里写尾部信息
    if (buff_.Length() == 0) {
        if (!__Reset()) return false;
    }

    size_t before_len = buff_.Length();
    size_t write_len = _length;

    //先把日志压缩并写入buff_, 这一步只是暂存，接着会把这段数据加密后再重新写入
    if (is_compress_) {
        uInt avail_out = (uInt)(buff_.MaxLength() - buff_.Length());
        write_len = Compress(_data, _length, buff_.PosPtr(), avail_out);
        if (write_len == (size_t)-1) {
            return false;
        }
        
    } else {
        buff_.Write(_data, _length);
    }

    //加密起始位置需要减去上次剩余没加密的数据(加密时是循环每次加密一个固定大小的块，所以最终加密的数据是这个块的整数倍，剩下的是余数)
    before_len -= remain_nocrypt_len_;

    std::string out_buffer;
    size_t last_remain_len = remain_nocrypt_len_;

    log_crypt_->CryptAsyncLog((char*)buff_.Ptr() + before_len, write_len + remain_nocrypt_len_, out_buffer, remain_nocrypt_len_);

    buff_.Write(out_buffer.data(), out_buffer.size(), before_len);

    before_len += out_buffer.size();
    buff_.Length(before_len, before_len);

    //更新log的长度，包括加密的和剩余没加密的, 这次加密是从上次剩余没加密的数据开始的，所以要减去last_remain_len
    log_crypt_->UpdateLogLen((char*)buff_.Ptr(), (uint32_t)(out_buffer.size() - last_remain_len));

    return true;
}

bool LogBaseBuffer::__Reset() {
    __Clear();

    //magic number + seq + hour + hour + len + pub_key
    //   1         +  2  +  1   +  1   +  4  +   64
    log_crypt_->SetHeaderInfo((char*)buff_.Ptr(), is_compress_, __GetMagicAsyncStart());
    buff_.Length(log_crypt_->GetHeaderLen(), log_crypt_->GetHeaderLen());
    return true;
}

//写入尾部信息，准备刷盘， 和_clear()配套使用
void LogBaseBuffer::__Flush() {
    assert(buff_.Length() >= log_crypt_->GetHeaderLen());

    log_crypt_->UpdateLogHour((char*)buff_.Ptr());
    log_crypt_->SetTailerInfo((char*)buff_.Ptr() + buff_.Length(), __GetMagicEnd());
    buff_.Length(buff_.Length() + log_crypt_->GetTailerLen(), buff_.Length() + log_crypt_->GetTailerLen());
}

void LogBaseBuffer::__Clear() {
    memset(buff_.Ptr(), 0, buff_.MaxLength());
    buff_.Length(0, 0);
    remain_nocrypt_len_ = 0;
}

//如果buff_中有加密过的数据，就提取出seq和len，并更新buff_的pos和lenght
void LogBaseBuffer::__Fix() {
    uint32_t raw_log_len = 0;
    if (log_crypt_->Fix((char*) buff_.Ptr(), buff_.Length(), raw_log_len)) {
        buff_.Length(raw_log_len + log_crypt_->GetHeaderLen(), raw_log_len + log_crypt_->GetHeaderLen());
    } else {
        buff_.Length(0, 0);
    }
}

char LogBaseBuffer::__GetMagicEnd() {
    return LogMagicNum::kMagicEnd;
}

}
}

