//
// Created by 罗旭维 on 2021/11/10.
//

#include "count_down_latch.h"


namespace xxlog {
    CountDownLatch::CountDownLatch(int count) : count_(count), mutex_(), condition_(mutex_) {}

    void CountDownLatch::Wait() {
        MutexGuard _guard(mutex_);

        while (count_ > 0) {
            condition_.Wait();
        }
    }

    void CountDownLatch::CountDown() {
        MutexGuard _guard(mutex_);

        count_--;
        if (count_ == 0) {
            condition_.NotifyOne();
        }
    }
}