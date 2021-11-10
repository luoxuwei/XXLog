//
// Created by 罗旭维 on 2021/11/10.
//

#include "file_operations.h"
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>

namespace xxlog {
    const char * const separators = "/\\";
    const char separator = '/';
    const char preferred_separator = '\\';
    const char colon = ':';

    bool is_letter(char c)
    {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
    }

    bool is_separator(char c)
    {
        return c == separator || c == preferred_separator;
    }

    bool is_root_separator(const std::string & str, int pos)
    // pos is position of the separator
    {
        // subsequent logic expects pos to be for leftmost slash of a set
        while (pos > 0 && is_separator(str[pos - 1]))
            --pos;

        //  "/" [...]
        if (pos == 0)
            return true;

        //  "c:/" [...]
        if (pos == 2 && is_letter(str[0]) && str[1] == colon)
            return true;

        //  "//" name "/"
        if (pos < 3 || !is_separator(str[0]) || !is_separator(str[1]))
            return false;

        return str.find_first_of(separators, 2) == pos;
    }

    file_type FileStatus(const char *_p) {
        struct stat path_stat;
        if (::stat(_p, &path_stat) != 0) {
            if (errno == ENOENT || errno == ENOTDIR) {
                return file_not_found;
            }
            return status_error;
        }

        if (S_IFDIR == path_stat.st_mode) {
            return directory_file;
        }

        if (S_IFREG == path_stat.st_mode) {
            return regular_file;
        }

        return type_unknown;
    }

    bool FileExists(const char *_p) {
        file_type _type = FileStatus(_p);
        return _type != status_error && _type != file_not_found;
    }

    time_t LastWriteTime(const char *_p) {
        struct stat path_stat;
        if (::stat(_p, &path_stat)!= 0) {
            return time_t(-1);
        }
        return path_stat.st_mtime;
    }

    bool RemoveFileOrDirectory(const char *_p) {
        file_type type = FileStatus(_p);
        if (file_not_found == type || status_error == type) {
            return false;
        }

        if (directory_file == type) {
            return (rmdir(_p) == 0);
        } else {
            return (unlink(_p) == 0);
        }
    }

    std::string FileExtension(const char *_p) {
        if (strcmp(_p, ".") == 0 || strcmp(_p, "..") == 0) return "";
        auto pos = std::string(_p).rfind('.');
        if (pos != std::string::npos) {
            return std::string(_p + pos);
        }
        return "";
    }

    int FileNamePos(const std::string &str) {
        int end_pos = str.size();
        // case: "//"
        if (end_pos == 2
            && is_separator(str[0])
            && is_separator(str[1])) return 0;

        // case: ends in "/"
        if (end_pos && is_separator(str[end_pos - 1]))
            return end_pos - 1;

        // set pos to start of last element
        int pos = str.find_last_of(separators, end_pos - 1);

        if (pos == std::string::npos && end_pos > 1)
            pos = str.find_last_of(colon, end_pos - 2);

        return (pos == std::string::npos // path itself must be a filename (or empty)
                || (pos == 1 && is_separator(str[0]))) // or net
               ? 0 // so filename is entire string
               : pos + 1; // or starts after delimiter
    }

    std::string FileName(const std::string &_path) {
        int pos = FileNamePos(_path);
        return (_path.size()
                && pos
                && is_separator(_path[pos])
                && !is_root_separator(_path, pos))
               ? (".")
               : (_path.c_str() + pos);
    }

    long FileSize(const std::string &_path) {
        struct stat path_stat;
        if (::stat(_path.c_str(), &path_stat) != 0) {
            return -1;
        }
        return path_stat.st_size;
    }

    bool OpenMmapFile(const char* _filepath, unsigned int _size, char **_buf, int *_fd) {
        int fd_ = open(_filepath, O_RDWR | O_CREAT,
                   S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        if (fd_ < 0) {
            fprintf(stderr, "open new file failed,errno=%d", errno);
            return false;
        } else {
            *_fd = fd_;
            int n = ftruncate(fd_, _size);
            (void)n;
            *_buf = (char *)mmap(NULL, _size, PROT_READ | PROT_WRITE,
                                   MAP_SHARED, fd_, 0);
            if (*_buf == MAP_FAILED) {
                fprintf(stderr, "mmap file failed,errno=%d", errno);
                return false;
            }
        }
        return true;
    }

    void CloseMmapFile(int *_fd, char *_buf, int _buf_size) {
        if (*_fd >= 0) {
            close(*_fd);
            *_fd = -1;
        }
        if (_buf != MAP_FAILED) {
            munmap(_buf, _buf_size);
        }
    }
}