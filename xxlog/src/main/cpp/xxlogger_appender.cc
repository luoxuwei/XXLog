//
// Created by 罗旭维 on 2021/11/9.
//

#include "xxlogger_appender.h"

namespace xxlog {
    XXLoggerAppender::XXLoggerAppender(const XXLogConfig &_config) {
        Open(_config);
    }

    void XXLoggerAppender::Open(const XXLogConfig &_config) {

    }

    void XXLoggerAppender::Flush() {

    }

    void XXLoggerAppender::Close() {

    }
}