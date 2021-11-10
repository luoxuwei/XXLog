//
// Created by 罗旭维 on 2021/11/10.
//

#ifndef XXLOG_FILE_OPERATIONS_H
#define XXLOG_FILE_OPERATIONS_H
#include <time.h>
#include <string>

namespace xxlog {
    enum file_type {
        status_error,
#   ifndef BOOST_FILESYSTEM_NO_DEPRECATED
        status_unknown = status_error,
#   endif
        file_not_found,
        regular_file,
        directory_file,
        type_unknown,
    };

    file_type FileStatus(const char *p);
    bool FileExists(const char *p);
    time_t LastWriteTime(const char *_p);
    bool RemoveFileOrDirectory(const char *_p);
    std::string FileExtension(const char *_p);
    int FileNamePos(const std::string &_path);
    std::string FileName(const std::string &_path);
    long FileSize(const std::string &_path);
    bool OpenMmapFile(const char* _filepath, unsigned int _size, char **_buf, int *_fd);
    void CloseMmapFile(int *_fd, char *_buf, int _buf_size);
}


#endif //XXLOG_FILE_OPERATIONS_H
