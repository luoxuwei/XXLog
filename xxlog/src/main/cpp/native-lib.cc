#include <jni.h>
#include <string>
#include <android/log.h>
#include "util/singleton.h"
#include "util/autobuffer.h"
#include "log_crypt.h"
#include "config.h"
#include "scoped_jstring.h"
#include "logger.h"

extern "C" JNIEXPORT jstring JNICALL
Java_com_luoxuwei_xxlog_XXLog_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

extern "C" JNIEXPORT void JNICALL
Java_com_luoxuwei_xxlog_XXLog_LogWrite2
        (JNIEnv *env, jclass, jint _level, jstring _tag, jstring _filename,
         jstring _funcname, jint _line, jint _pid, jlong _tid, jlong _maintid, jstring _log) {
    if (!Singleton<xxlog::Logger>::getInstance()->IsEnabledFor((xxlog::LogLevel)_level)) {
        return;
    }

    xxlog::XXLoggerInfo xxlog_info;
    gettimeofday(&xxlog_info.timeval, NULL);
    xxlog_info.level = (xxlog::LogLevel)_level;
    xxlog_info.line = _line;
    xxlog_info.pid = _pid;
    xxlog_info.tid = _tid;
    xxlog_info.maintid = _maintid;

    const char* tag_cstr = NULL;
    const char* filename_cstr = NULL;
    const char* funcname_cstr = NULL;
    const char* log_cstr = NULL;

    if (NULL != _tag) {
        tag_cstr = env->GetStringUTFChars(_tag, NULL);
    }

    if (NULL != _filename) {
        filename_cstr = env->GetStringUTFChars(_filename, NULL);
    }

    if (NULL != _funcname) {
        funcname_cstr = env->GetStringUTFChars(_funcname, NULL);
    }

    if (NULL != _log) {
        log_cstr = env->GetStringUTFChars(_log, NULL);
    }

    xxlog_info.tag = NULL == tag_cstr ? "" : tag_cstr;
    xxlog_info.filename = NULL == filename_cstr ? "" : filename_cstr;
    xxlog_info.func_name = NULL == funcname_cstr ? "" : funcname_cstr;

    Singleton<xxlog::Logger>::getInstance()->Write(&xxlog_info, NULL == log_cstr? "NULL == log" : log_cstr);

    if (NULL != _tag) {
        env->ReleaseStringUTFChars(_tag, tag_cstr);
    }

    if (NULL != _filename) {
        env->ReleaseStringUTFChars(_filename, filename_cstr);
    }

    if (NULL != _funcname) {
        env->ReleaseStringUTFChars(_funcname, funcname_cstr);
    }

    if (NULL != _log) {
        env->ReleaseStringUTFChars(_log, log_cstr);
    }
}

extern "C" JNIEXPORT void JNICALL
Java_com_luoxuwei_xxlog_XXLog_AppenderOpen
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

    std::string cachedir_str;
    if (NULL != cachedir) {
        ScopedJstring cachedir_jstr(env, cachedir);
        cachedir_str = cachedir_jstr.SafeGetChar();
    }

    std::string pubkey_str;
    if (NULL != pubkey) {
        ScopedJstring pubkey_jstr(env, pubkey);
        pubkey_str = pubkey_jstr.SafeGetChar();
    }

    std::string logdir_str;
    if (NULL != logdir) {
        ScopedJstring logdir_jstr(env, logdir);
        logdir_str = logdir_jstr.SafeGetChar();
    }

    std::string nameprefix_str;
    if (NULL != nameprefix) {
        ScopedJstring nameprefix_jstr(env, nameprefix);
        nameprefix_str = nameprefix_jstr.SafeGetChar();
    }

    xxlog::XXLogConfig config = {(xxlog::AppenderMode)mode, logdir_str, nameprefix_str, pubkey_str,
                                 (xxlog::CompressMode)compressmode, compresslevel, cachedir_str, cachedays};
    Singleton<xxlog::Logger>::getInstance()->Open(config);
    Singleton<xxlog::Logger>::getInstance()->SetLogLevel((xxlog::LogLevel)level);
}

extern "C" JNIEXPORT jint JNICALL
Java_com_luoxuwei_xxlog_XXLog_GetLogLevel
        (JNIEnv *env, jclass clazz) {
    return Singleton<xxlog::Logger>::getInstance()->GetLogLevel();
}

extern "C" JNIEXPORT void JNICALL
Java_com_luoxuwei_xxlog_XXLog_SetConsoleLogOpen
        (JNIEnv *env, jclass clazz, jboolean _is_open) {
    Singleton<xxlog::Logger>::getInstance()->SetConsoleLogOpen(_is_open);
}

extern "C" JNIEXPORT void JNICALL
Java_com_luoxuwei_xxlog_XXLog_AppenderClose
        (JNIEnv *env, jclass clazz) {
    Singleton<xxlog::Logger>::getInstance()->Close();
}

extern "C" JNIEXPORT void JNICALL
Java_com_luoxuwei_xxlog_XXLog_SetMaxFileSize
        (JNIEnv *env, jclass clazz,  jlong _max_size) {
    Singleton<xxlog::Logger>::getInstance()->SetMaxFileSize(_max_size);
}

extern "C" JNIEXPORT void JNICALL
Java_com_luoxuwei_xxlog_XXLog_SetMaxAliveTime
        (JNIEnv *env, jclass clazz,  jlong _max_time) {
    Singleton<xxlog::Logger>::getInstance()->SetMaxAliveTime(_max_time);
}

extern "C" JNIEXPORT void JNICALL
Java_com_luoxuwei_xxlog_XXLog_AppenderFlush
        (JNIEnv *env, jclass clazz, jboolean _is_sync) {
    Singleton<xxlog::Logger>::getInstance()->Flush(_is_sync);
}