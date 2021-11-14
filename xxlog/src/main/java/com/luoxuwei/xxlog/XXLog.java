package com.luoxuwei.xxlog;

import android.os.Looper;
import android.os.Process;

/**
 * Created by 罗旭维 on 2021/11/7.
 */
public class XXLog implements Log.LogImp {

    public static final int LEVEL_ALL = 0;
    public static final int LEVEL_VERBOSE = 0;
    public static final int LEVEL_DEBUG = 1;
    public static final int LEVEL_INFO = 2;
    public static final int LEVEL_WARNING = 3;
    public static final int LEVEL_ERROR = 4;
    public static final int LEVEL_FATAL = 5;
    public static final int LEVEL_NONE = 6;

    public static final int COMPRESS_LEVEL1 = 1;
    public static final int COMPRESS_LEVEL2 = 2;
    public static final int COMPRESS_LEVEL3 = 3;
    public static final int COMPRESS_LEVEL4 = 4;
    public static final int COMPRESS_LEVEL5 = 5;
    public static final int COMPRESS_LEVEL6 = 6;
    public static final int COMPRESS_LEVEL7 = 7;
    public static final int COMPRESS_LEVEL8 = 8;
    public static final int COMPRESS_LEVEL9 = 9;

    public static final int AppednerModeAsync = 0;
    public static final int AppednerModeSync = 1;

    public static final int ZLIB_MODE = 0;

    static class XXLoggerInfo {
        public int level;
        public String tag;
        public String filename;
        public String funcname;
        public int line;
        public long pid;
        public long tid;
        public long maintid;
    }

    public static class XXLogConfig {
        public int level = LEVEL_INFO;
        public int mode = AppednerModeAsync;
        public String logdir;
        public String nameprefix;
        public String pubkey = "";
        public int compressmode = ZLIB_MODE;
        public int compresslevel = 0;
        public String cachedir;
        public int cachedays = 0;
    }

    static {
        System.loadLibrary("c++_shared");
        System.loadLibrary("xxlog");
    }

    public native String stringFromJNI();

    public static void open(int level, int mode, String cacheDir, String logDir, String nameprefix, int cacheDays, String pubkey) {
        XXLogConfig logConfig = new XXLogConfig();
        logConfig.level = level;
        logConfig.mode = mode;
        logConfig.logdir = logDir;
        logConfig.nameprefix = nameprefix;
        logConfig.pubkey = pubkey;
        logConfig.compressmode = ZLIB_MODE;
        logConfig.cachedir = cacheDir;
        logConfig.cachedays = cacheDays;
        AppenderOpen(logConfig);
    }


    @Override
    public void logV(String tag, String filename, String funcname, int linuxTid, int pid, long tid, long maintid, String log) {
        LogWrite2(LEVEL_VERBOSE, tag, filename, funcname, linuxTid, pid, tid, maintid,  log);
    }

    @Override
    public void logI(String tag, String filename, String funcname, int linuxTid, int pid, long tid, long maintid, String log) {
        LogWrite2(LEVEL_INFO, tag, filename, funcname, linuxTid, pid, tid, maintid,  log);
    }

    @Override
    public void logD(String tag, String filename, String funcname, int linuxTid, int pid, long tid, long maintid, String log) {
        LogWrite2(LEVEL_DEBUG, tag, filename, funcname, linuxTid, pid, tid, maintid,  log);
    }

    @Override
    public void logW(String tag, String filename, String funcname, int linuxTid, int pid, long tid, long maintid, String log) {
        LogWrite2(LEVEL_WARNING, tag, filename, funcname, linuxTid, pid, tid, maintid,  log);
    }

    @Override
    public void logE(String tag, String filename, String funcname, int linuxTid, int pid, long tid, long maintid, String log) {
        LogWrite2(LEVEL_ERROR, tag, filename, funcname, linuxTid, pid, tid, maintid,  log);
    }

    @Override
    public void logF(String tag, String filename, String funcname, int linuxTid, int pid, long tid, long maintid, String log) {
        LogWrite2(LEVEL_FATAL, tag, filename, funcname, linuxTid, pid, tid, maintid,  log);
    }

    public static native void LogWrite2(int level, String tag, String filename, String funcname, int line, int pid, long tid, long maintid, String log);


    public int getLogLevel() {
        return GetLogLevel();
    }

    @Override
    public void setAppenderMode(int mode) {

    }

    @Override
    public void appenderOpen(int level, int mode, String cacheDir, String logDir, String nameprefix, int cacheDays, String pubkey) {
        XXLogConfig logConfig = new XXLogConfig();
        logConfig.level = level;
        logConfig.mode = mode;
        logConfig.logdir = logDir;
        logConfig.nameprefix = nameprefix;
        logConfig.pubkey = pubkey;
        logConfig.compressmode = ZLIB_MODE;
        logConfig.cachedir = cacheDir;
        logConfig.cachedays = cacheDays;
        AppenderOpen(logConfig);
    }

    @Override
    public void appenderClose() {
        AppenderClose();
    }

    @Override
    public void appenderFlush(boolean isSync) {
        AppenderFlush(isSync);
    }

    @Override
    public void setConsoleLogOpen(boolean isOpen) {
        SetConsoleLogOpen(isOpen);
    }

    @Override
    public void setMaxFileSize(long aliveSeconds) {
        SetMaxFileSize(aliveSeconds);
    }

    @Override
    public void setMaxAliveTime(long aliveSeconds) {
        SetMaxAliveTime(aliveSeconds);
    }

    public static native void AppenderOpen(XXLogConfig config);
    public static native int GetLogLevel();
    public static native void SetConsoleLogOpen(boolean open);
    public static native void AppenderClose();
    public static native void SetMaxFileSize(long maxFileSize);
    public static native void SetMaxAliveTime(long maxFileSize);
    public static native void AppenderFlush(boolean isSync);
}
