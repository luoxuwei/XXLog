//
// Created by 罗旭维 on 2021/11/10.
//

#ifndef XXLOG_CONDITION_H
#define XXLOG_CONDITION_H
#include <pthread.h>
#include <stdint.h>

#include "mutex.h"
namespace xxlog {
    //必须与Mutex配合使用，外部需要确保notify和wait调用都要加锁。
    class Condition {
    public:
        explicit Condition(Mutex &_mutex);
        ~Condition();

        void Wait();
        bool WaitForSeconds(int32_t _seconds);
        void NotifyOne();
        void NotifyAll();

    private:
        Mutex mutex_;
        pthread_cond_t condvar_;
    };
}



#endif //XXLOG_CONDITION_H
