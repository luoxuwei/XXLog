//
// Created by 罗旭维 on 2021/11/9.
//

#ifndef XXLOG_XXLOGGERAPPENDER_H
#define XXLOG_XXLOGGERAPPENDER_H
#include "config.h"
namespace xxlog {
    class XXLoggerAppender {
    public:
        XXLoggerAppender(const XXLogConfig &_config);

        void Open(const XXLogConfig &_config);

        void Close();

        void Flush();
    };
}

#endif //XXLOG_XXLOGGERAPPENDER_H
