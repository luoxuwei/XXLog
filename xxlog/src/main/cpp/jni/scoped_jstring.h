//
// Created by 罗旭维 on 2021/11/9.
//

#ifndef XXLOG_SCOPED_JSTRING_H
#define XXLOG_SCOPED_JSTRING_H
#include <jni.h>

class ScopedJstring {
public:
    ScopedJstring(JNIEnv *_env, jstring _jstr);
    ScopedJstring(JNIEnv *_env, const char *_char);
    ~ScopedJstring();
    const char * GetChar() const;
    const char * SafeGetChar() const;
    jstring GetJstr() const;

private:
    ScopedJstring();
    ScopedJstring(const ScopedJstring &);
    ScopedJstring & operator=(const ScopedJstring&);

private:
    JNIEnv *env_;
    jstring jstr_;
    const char *char_;
    bool jstr2char_;
};


#endif //XXLOG_SCOPED_JSTRING_H
