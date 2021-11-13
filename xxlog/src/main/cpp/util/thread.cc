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

    Thread::Thread(Func _cb, const std::string &_name) {
        thread_reference_ = new ThreadReference(std::move(_cb));
        MutexGuard _guard(thread_reference_->mutex_);
        thread_reference_->AddRef();
        if (_name.empty()) {
            int num = KThread_count.fetch_add(1);
            char buf[30];
            snprintf(buf, sizeof(buf), "Thread-%d", num);
            thread_reference_->name_ = buf;
        } else {
            thread_reference_->name_ = _name;
        }
    }

    Thread::~Thread() {
        thread_reference_->mutex_.Lock();
        //回收线程资源，如果是joined 不需要detach
        if (0 != thread_reference_->tid_ && !thread_reference_->joined_) {
            pthread_detach(thread_reference_->tid_);
        }
        thread_reference_->RemoveRef(thread_reference_->mutex_);
    }

    bool Thread::IsStarted() {
        return thread_reference_->started_;
    }

    void Thread::Start() {
        thread_reference_->mutex_.Lock();
        if (IsStarted()) return;
        thread_reference_->started_ = true;
        thread_reference_->AddRef();
        if (pthread_create(&thread_reference_->tid_, nullptr, Thread::StartRoutine, thread_reference_) != thrd_success) {
            thread_reference_->started_ = false;
            thread_reference_->RemoveRef(thread_reference_->mutex_);
            return;
        }
        thread_reference_->mutex_.Unlock();
    }

    void *Thread::StartRoutine(void *_arg) {
        init(_arg);
        volatile ThreadReference *thread_reference_ = static_cast<ThreadReference *>(_arg);
        pthread_cleanup_push(&cleanup, _arg);
        Func cb;
        cb.swap(const_cast<ThreadReference*>(thread_reference_)->cb_);
        cb();
        pthread_cleanup_pop(1);
        return 0;
    }

    void Thread::StartAfter(long after) {
        thread_reference_->mutex_.Lock();
        if (IsStarted()) return;
        thread_reference_->started_ = true;
        thread_reference_->aftertime_ = after;
        thread_reference_->AddRef();

        if (pthread_create(&thread_reference_->tid_, nullptr, Thread::StartRoutineAfter, thread_reference_) != thrd_success) {
            thread_reference_->started_ = false;
            thread_reference_->RemoveRef(thread_reference_->mutex_);
            return;
        }
        thread_reference_->mutex_.Unlock();
    }

    void *Thread::StartRoutineAfter(void *_arg) {
        init(_arg);
        volatile ThreadReference *thread_reference_ = static_cast<ThreadReference *>(_arg);
        pthread_cleanup_push(&cleanup, _arg);

        const_cast<ThreadReference*>(thread_reference_)->condtime_.WaitForSeconds(thread_reference_->aftertime_);

        Func cb;
        cb.swap(const_cast<ThreadReference*>(thread_reference_)->cb_);
        cb();
        pthread_cleanup_pop(1);
        return 0;
    }

    void Thread::Join() {
        MutexGuard _lock(thread_reference_->mutex_);
        if (thread_reference_->tid_ == pthread_self()) return ;//只能在其他线程调用本线程的join
        if (IsStarted()) {
            thread_reference_->joined_ = true;
            if (pthread_join(thread_reference_->tid_, nullptr) != thrd_success) {
                thread_reference_->joined_ = false;
            }
        }
    }

    void Thread::init(void *arg) {
        volatile ThreadReference* threadReference = static_cast<ThreadReference*>(arg);
        MutexGuard lock((const_cast<ThreadReference*>(threadReference))->mutex_);
        pthread_setname_np(threadReference->tid_, const_cast<ThreadReference*>(threadReference)->name_.c_str());
    }

    void Thread::cleanup(void *arg) {
        volatile ThreadReference* threadReference = static_cast<ThreadReference*>(arg);
        const_cast<ThreadReference*>(threadReference)->mutex_.Lock();
        threadReference->started_ = false;
        (const_cast<ThreadReference*>(threadReference))->RemoveRef(const_cast<ThreadReference*>(threadReference)->mutex_);
    }

}