//
// Created by 罗旭维 on 2021/11/10.
//

#ifndef XXLOG_STRUTIL_H
#define XXLOG_STRUTIL_H
#include <string>

namespace strutil {
    bool StartsWith(const std::string& str, const std::string& substr);
    bool EndsWith(const std::string& str, const std::string& substr);
};


#endif //XXLOG_STRUTIL_H
