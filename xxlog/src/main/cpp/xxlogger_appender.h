//
// Created by 罗旭维 on 2021/11/9.
//

#ifndef XXLOG_XXLOGGERAPPENDER_H
#define XXLOG_XXLOGGERAPPENDER_H
#include "config.h"
#include "thread.h"
#include "log_base_buffer.h"

namespace xxlog {

    class XXLoggerAppender {
    public:
        XXLoggerAppender(const XXLogConfig &_config);
        void SetMaxAliveDuration(long _max_time);

        void Open(const XXLogConfig &_config);

        void Close();

        void Flush();

    private:
        void _AsyncLogThread();
        void _DelTimeoutFile(const std::string &_log_path);
        void _MoveOldFiles(const std::string& _src_path, const std::string& _dest_path,
                           const std::string& _nameprefix);
        bool _AppendFile(const std::string& _src_file, const std::string& _dst_file);
    private:
        Thread thread_async_;
        Mutex mutex_log_file_;
        XXLogConfig config_;
        mars::xlog::LogBaseBuffer* log_buff_ = nullptr;
        uint64_t max_file_size_ = 0; // 0, will not split log file.
        long max_alive_time_ = 10 * 24 * 60 * 60;    // 10 days in second
    };
}

#endif //XXLOG_XXLOGGERAPPENDER_H
