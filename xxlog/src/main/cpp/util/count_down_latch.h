//
// Created by 罗旭维 on 2021/11/10.
//

#ifndef XXLOG_COUNT_DOWN_LATCH_H
#define XXLOG_COUNT_DOWN_LATCH_H

#include "mutex.h"
#include "condition.h"

namespace xxlog {
    class CountDownLatch {
    public:
        explicit CountDownLatch(int count);
        void Wait();
        void CountDown();
        inline int32_t GetCount() const {
            MutexGuard _lk(mutex_);
            return count_;
        }

    private:
        int32_t count_ = 0;
        mutable Mutex mutex_;
        Condition condition_;
    };
}



#endif //XXLOG_COUNT_DOWN_LATCH_H
