//
// Created by 罗旭维 on 2021/11/10.
//

#include "thread.h"
#include <atomic>
#include <sys/syscall.h>
#include <unistd.h>
#include <string>

namespace xxlog {

    std::atomic<int> KThread_count(0);
    static __thread pid_t t_tid = 0;//GCC内置的线程局部存储,__thread变量每一个线程有一份独立实体

    pid_t Thread::CurrentThreadTid() {
        if (t_tid == 0) {
            t_tid = ::syscall(SYS_gettid);//真实的线程id唯一标识
        }
        return t_tid;
    }

    Thread::Thread(Func _cb, const std::string &_name)
    : started_(false)
    , joined_(false)
    , cb_(std::move(_cb))
    , mutex_()
    , condtime_(mutex_)
    , aftertime_(0) {
        if (_name.empty()) {
            int num = KThread_count.fetch_add(1);
            char buf[30];
            snprintf(buf, sizeof(buf), "Thread-%d", num);
            name_ = buf;
        } else {
            name_ = _name;
        }
    }

    Thread::~Thread() {
        MutexGuard _guard(mutex_);
        //回收线程资源，如果是joined 不需要detach
        if (started_ && !joined_) {
            pthread_detach(tid_);
        }
    }

    bool Thread::IsStarted() {
        MutexGuard _guard(mutex_);
        return started_;
    }

    void Thread::Start() {
        MutexGuard _guard(mutex_);
        started_ = true;
        if (pthread_create(&tid_, nullptr, Thread::StartRoutine, this) != thrd_success) {
            started_ = false;
        }
    }

    void *Thread::StartRoutine(void *_arg) {
        Thread *_thread = static_cast<Thread *>(_arg);
        Func cb;
        cb.swap(_thread->cb_);
        cb();
        return 0;
    }

    void Thread::StartAfter(long after) {
        MutexGuard _guard(mutex_);
        started_ = true;
        aftertime_ = after;
        if (pthread_create(&tid_, nullptr, Thread::StartRoutineAfter, this) != thrd_success) {
            started_ = false;
        }
    }

    void *Thread::StartRoutineAfter(void *_arg) {
        Thread *_thread = static_cast<Thread *>(_arg);

        {
            MutexGuard _guard(_thread->mutex_);
            _thread->condtime_.WaitForSeconds(_thread->aftertime_);
        }

        Func cb;
        cb.swap(_thread->cb_);
        cb();
        return 0;
    }

    void Thread::Join() {
        MutexGuard _guard(mutex_);
        joined_ = true;
        if (pthread_join(tid_, nullptr) != thrd_success) {
            joined_ = false;
        }
    }

}