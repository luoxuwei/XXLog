//
// Created by 罗旭维 on 2021/11/11.
//

#ifndef XXLOG_LOG_FILE_H
#define XXLOG_LOG_FILE_H
#include <string>
#include "config.h"
#include "mutex.h"
#include "log_base_buffer.h"

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
        void CloseLogFile();
        void WriteTips2File(const char* _tips_format, ...);
        void SetMaxFileSize(uint64_t _max_byte_size);
        void SetMode(AppenderMode _mode);
    private:
        void _DelTimeoutFile(const std::string &_log_path);
        void _MoveOldFiles(const std::string& _src_path, const std::string& _dest_path,
                           const std::string& _nameprefix);
        bool _OpenLogFile(const std::string& _log_dir);
        void _MakeLogFileName(const timeval& _tv,
                             const std::string& _logdir,
                             const char* _prefix,
                             const std::string& _fileext,
                             char* _filepath,
                             unsigned int _len);
        long _GetNextFileIndex(const std::string& _fileprefix, const std::string& _fileext);
        void _GetFileNamesByPrefix(const std::string& _logdir,
                                   const std::string& _fileprefix,
                                   const std::string& _fileext,
                                   std::vector<std::string>& _filename_vec);
        bool _WriteFile(const void* _data, size_t _len, FILE* _file);

        bool _CacheLogs();
    private:
        XXLogConfig config_;
        //所有文件操作都要加锁，相关方法的调用主要集中在Log2File中，所以这个方法加锁可以保护和覆盖其他多个方法，
        // 另外两个需要加锁的是CLoseLogFile和SetMode
        Mutex mutex_log_file_;
        FILE* logfile_ = nullptr;
        time_t openfiletime_ = 0;
        std::shared_ptr<FileWriter> file_;
        long max_alive_time_ = 10 * 24 * 60 * 60;    // 10 days in second
        uint64_t max_file_size_ = 0; // 0, will not split log file.
        time_t last_time_ = 0;
        uint64_t last_tick_ = 0;
        char last_file_path_[1024] = {0};
        mars::xlog::LogBaseBuffer* log_buff_ = nullptr;
    };
}

#endif //XXLOG_LOG_FILE_H
