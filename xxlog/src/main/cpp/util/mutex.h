//
// Created by 罗旭维 on 2021/11/10.
//

#ifndef XXLOG_MUTEX_H
#define XXLOG_MUTEX_H
#include <pthread.h>

namespace xxlog {
    class Mutex {
    public:
        Mutex();
        ~Mutex();

        void Lock();
        void Unlock();
        bool IsLocked();

        pthread_mutex_t *GetMutex();
    private:
        pthread_mutex_t mutex_;
    };

    class MutexGuard {
    public:
        MutexGuard(Mutex &);
        ~MutexGuard();

    private:
        Mutex &mutex_;
    };
}



#endif //XXLOG_MUTEX_H
