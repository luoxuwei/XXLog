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
        static void cleanup(void* arg);
        static void init(void* arg);
        static pid_t CurrentThreadTid();

    private:
        class ThreadReference {
        public:
            ThreadReference(Func _cb) : cb_(std::move(_cb)), mutex_(), condtime_(mutex_) {

            };
            void AddRef() { count_++;}
            void RemoveRef(Mutex& _lock) {
//                ASSERT(0 < count_);
//                ASSERT(_lock.IsLocked());

                bool willdel = false;
                count_--;

                if (0 == count_) willdel = true;

                _lock.Unlock();

                if (willdel) delete this;
            }
        private:
            ThreadReference(const ThreadReference&);
            ThreadReference& operator=(const ThreadReference&);

        public:
            int count_ = 0;
            bool started_ = false;
            bool joined_ = false;
            pthread_t tid_ = 0;
            std::string name_;
            Func cb_;
            Condition condtime_;
            Mutex mutex_;
            long aftertime_ = LONG_MAX;
        };
        static void *StartRoutine(void *_arg);
        static void *StartRoutineAfter(void *_arg);

        ThreadReference *thread_reference_ = nullptr;

    };
}

#endif //XXLOG_THREAD_H
