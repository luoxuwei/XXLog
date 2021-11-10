//
// Created by 罗旭维 on 2021/11/10.
//

#include "strutil.h"

namespace strutil {
    bool StartsWith(const std::string& str, const std::string& substr) {
        return str.find(substr) == 0;
    }

    bool EndsWith(const std::string& str, const std::string& substr) {
        size_t i = str.rfind(substr);
        return (i != std::string::npos) && (i == (str.length() - substr.length()));
    }
}