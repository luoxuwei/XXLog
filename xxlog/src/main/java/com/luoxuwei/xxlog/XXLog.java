package com.luoxuwei.xxlog;

import android.os.Looper;
import android.os.Process;

/**
 * Created by 罗旭维 on 2021/11/7.
 */
public class XXLog {

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

    public static void open(int level, int mode, String cacheDir, String logDir, String nameprefix, String pubkey) {
        XXLogConfig logConfig = new XXLogConfig();
        logConfig.level = level;
        logConfig.mode = mode;
        logConfig.logdir = logDir;
        logConfig.nameprefix = nameprefix;
        logConfig.pubkey = pubkey;
        logConfig.compressmode = ZLIB_MODE;
        logConfig.compresslevel = 0;
        logConfig.cachedir = cacheDir;
        logConfig.cachedays = 0;
        appenderOpen(logConfig);
    }

    public static void i(String tag, final String format, final Object... obj) {
        if (getLogLevel() <= LEVEL_INFO) {
            String log = obj == null ? format : String.format(format, obj);
            if (log == null) {
                log = "";
            }
            logI(tag, "", "", 0, Process.myPid(), Thread.currentThread().getId(), Looper.getMainLooper().getThread().getId(), log);
        }
    }

    public static void logI(String tag, String filename, String funcname, int line, int pid, long tid, long maintid, String log) {
        logWrite2(LEVEL_INFO, tag, filename, funcname, line, pid, tid, maintid,  log);
    }

    public static native void logWrite2(int level, String tag, String filename, String funcname, int line, int pid, long tid, long maintid, String log);

    private static native void appenderOpen(XXLogConfig logConfig);

    public static native int getLogLevel();

    public static native void setConsoleLogOpen(boolean open);
}
