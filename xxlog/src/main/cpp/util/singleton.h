//
// Created by 罗旭维 on 2021/11/7.
//

#ifndef XXLOG_SINGLETON_H
#define XXLOG_SINGLETON_H

#include <pthread.h>

template <typename T>
class Singleton {
public:
    static T *getInstance() {
        pthread_once(&once_control, &Singleton::init);
        return value_;
    }

    static void destroy() {
        if (nullptr != value_) {
            delete value_;
        }
    }
private:
    Singleton();
    ~Singleton();
    //init方法必须是static，不然无法做pthread_once的参数
    static void init() {
        value_ = new T();
        ::atexit(destroy);
    }
    static T *value_;
    static pthread_once_t once_control;
};

template <typename T>
pthread_once_t Singleton<T>::once_control = PTHREAD_ONCE_INIT;

template <typename T>
T * Singleton<T>::value_ = nullptr;

#endif //XXLOG_SINGLETON_H
