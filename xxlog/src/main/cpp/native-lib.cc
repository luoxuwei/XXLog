#include <jni.h>
#include <string>
#include <android/log.h>
#include "util/singleton.h"
#include "util/autobuffer.h"
#include "log_crypt.h"
#include "config.h"

extern "C" JNIEXPORT jstring JNICALL
Java_com_luoxuwei_xxlog_XXLog_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

extern "C" JNIEXPORT void JNICALL
Java_com_luoxuwei_xxlog_XXLog_logWrite2
        (JNIEnv *env, jclass, jint _level, jstring _tag, jstring _filename,
         jstring _funcname, jint _line, jint _pid, jlong _tid, jlong _maintid, jstring _log) {

}

extern "C" JNIEXPORT void JNICALL
Java_com_luoxuwei_xxlog_XXLog_appenderOpen
        (JNIEnv *env, jclass clazz, jobject _log_config) {
    if (NULL == _log_config) {
        __android_log_write(ANDROID_LOG_ERROR, "", "logconfig is null");
    }

    jclass _config_class = env->GetObjectClass(_log_config);
    jfieldID  _config_field = env->GetFieldID(_config_class, "level", "I");
    jint level = env->GetIntField(_log_config, _config_field);

    _config_field = env->GetFieldID(_config_class, "mode", "I");
    jint mode = env->GetIntField(_log_config, _config_field);

    _config_field = env->GetFieldID(_config_class, "logdir", "Ljava/lang/String;");
    jstring logdir = (jstring)env->GetObjectField(_log_config, _config_field);

    _config_field = env->GetFieldID(_config_class, "nameprefix", "Ljava/lang/String;");
    jstring nameprefix = (jstring)env->GetObjectField(_log_config, _config_field);

    _config_field = env->GetFieldID(_config_class, "pubkey", "Ljava/lang/String;");
    jstring pubkey = (jstring)env->GetObjectField(_log_config, _config_field);

    _config_field = env->GetFieldID(_config_class, "compressmode", "I");
    jint compressmode = env->GetIntField(_log_config, _config_field);

    _config_field = env->GetFieldID(_config_class, "compresslevel", "I");
    jint compresslevel = env->GetIntField(_log_config, _config_field);

    _config_field = env->GetFieldID(_config_class, "cachedir", "Ljava/lang/String;");
    jstring cachedir = (jstring)env->GetObjectField(_log_config, _config_field);

    _config_field = env->GetFieldID(_config_class, "cachedays", "I");
    jint cachedays = env->GetIntField(_log_config, _config_field);




}

extern "C" JNIEXPORT jint JNICALL
Java_com_luoxuwei_xxlog_XXLog_getLogLevel
        (JNIEnv *env, jclass clazz) {
    return kLevelNone;
}