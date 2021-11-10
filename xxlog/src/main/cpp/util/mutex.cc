//
// Created by 罗旭维 on 2021/11/10.
//

#include "mutex.h"

namespace xxlog {
    Mutex::Mutex() {
        pthread_mutex_init(&mutex_, NULL);
    }

    Mutex::~Mutex() {
        pthread_mutex_destroy(&mutex_);
    }

    pthread_mutex_t * Mutex::GetMutex() {
        return &mutex_;
    }

    void Mutex::Lock() {
        pthread_mutex_lock(&mutex_);
    }

    void Mutex::Unlock() {
        pthread_mutex_unlock(&mutex_);
    }

    bool Mutex::IsLocked() {
        int ret = pthread_mutex_trylock(&mutex_);

        if (0 == ret) Unlock();

        return 0 != ret;
    }

    MutexGuard::MutexGuard(Mutex &_mutex) : mutex_(_mutex) {
        mutex_.Lock();
    }

    MutexGuard::~MutexGuard() {
        mutex_.Unlock();
    }
}