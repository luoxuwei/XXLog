//
// Created by 罗旭维 on 2021/11/10.
//

#ifndef XXLOG_THREAD_H
#define XXLOG_THREAD_H
#include <pthread.h>
#include <functional>
#include <string>
#include "mutex.h"
#include "condition.h"

#define thrd_success (0)
namespace xxlog {
    class Thread {
    public:
        using Func = std::function<void(void)>;
        Thread(Func _cb, const std::string &_name);
        ~Thread();

        bool IsStarted();
        void Start();
        void StartAfter(long after);
        void Join();
        const std::string &GetName() const;

        static pid_t CurrentThreadTid();

    private:
        static void *StartRoutine(void *_arg);
        static void *StartRoutineAfter(void *_arg);
        bool started_ = false;
        bool joined_ = false;
        pthread_t tid_;
        std::string name_;
        Func cb_;
        Condition condtime_;
        Mutex mutex_;
        long aftertime_;
    };
}

#endif //XXLOG_THREAD_H
