//
// Created by 罗旭维 on 2021/11/11.
//

#ifndef XXLOG_LOG_FILE_H
#define XXLOG_LOG_FILE_H
#include <string>
#include "config.h"
#include "mutex.h"

namespace xxlog {
    enum class FileWriterType : uint8_t { MMAPFILE = 0, APPENDFILE };
    class FileWriter {
    public:
        FileWriter() = default;
        virtual ~FileWriter() = default;
        virtual void append(const char *msg, int32_t len) = 0;
        virtual void flush() = 0;
        virtual uint32_t writtenBytes() const = 0;
    };

    class LogFile {
    public:
        LogFile(const XXLogConfig &_config);
        void SetMaxAliveDuration(long _max_time);
        void Log2File(const void* _data, size_t _len, bool _move_file);
        void Open();
        void WriteTips2File(const char* _tips_format, ...);

    private:
        void _DelTimeoutFile(const std::string &_log_path);
        void _MoveOldFiles(const std::string& _src_path, const std::string& _dest_path,
                           const std::string& _nameprefix);
        bool _AppendFile(const std::string& _src_file, const std::string& _dst_file);
    private:
        XXLogConfig config_;
        Mutex mutex_log_file_;
        std::shared_ptr<FileWriter> file_;
        long max_alive_time_ = 10 * 24 * 60 * 60;    // 10 days in second
    };
}

#endif //XXLOG_LOG_FILE_H
