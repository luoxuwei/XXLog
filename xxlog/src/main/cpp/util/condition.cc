//
// Created by 罗旭维 on 2021/11/10.
//
#include <errno.h>
#include "condition.h"

namespace xxlog {
    Condition::Condition(Mutex &_mutex) : mutex_(_mutex) {
        pthread_cond_init(&condvar_, nullptr);
    }

    Condition::~Condition() {
        pthread_cond_destroy(&condvar_);
    }

    void Condition::Wait() {
        pthread_cond_wait(&condvar_, mutex_.GetMutex());
    }

    bool Condition::WaitForSeconds(int32_t _seconds) {
        struct timespec abstime;
        clock_gettime(CLOCK_REALTIME, &abstime);

        const int64_t KNanoSecondsPerSecond = 1000000000;
        int64_t nanoseconds = static_cast<int64_t>(_seconds * KNanoSecondsPerSecond);

        abstime.tv_sec += static_cast<time_t>((abstime.tv_nsec + nanoseconds) / KNanoSecondsPerSecond);
        abstime.tv_nsec += static_cast<long>((abstime.tv_nsec + nanoseconds) % KNanoSecondsPerSecond);

        //时钟类型是CLOCK_REALTIME，系统范围内的实时时钟,是个软件时钟,可以通过命令等方式修改该系统时间.
        //在极端的情况下(如在获取当前系统时间后系统时间马上被修订)时,这个超时时间就不正确了,不过这是小概率事件.
        return ETIMEDOUT == pthread_cond_timedwait(&condvar_, mutex_.GetMutex(), &abstime);
    }

    void Condition::NotifyOne() {
        pthread_cond_signal(&condvar_);
    }

    void Condition::NotifyAll() {
        pthread_cond_broadcast(&condvar_);
    }
}