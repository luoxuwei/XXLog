package com.luoxuwei.xxlog;
import android.os.Process;
import android.os.Handler;
import android.os.Looper;

import static com.luoxuwei.xxlog.XXLog.LEVEL_DEBUG;
import static com.luoxuwei.xxlog.XXLog.LEVEL_ERROR;
import static com.luoxuwei.xxlog.XXLog.LEVEL_FATAL;
import static com.luoxuwei.xxlog.XXLog.LEVEL_INFO;
import static com.luoxuwei.xxlog.XXLog.LEVEL_NONE;
import static com.luoxuwei.xxlog.XXLog.LEVEL_VERBOSE;
import static com.luoxuwei.xxlog.XXLog.LEVEL_WARNING;

/**
 * Created by 罗旭维 on 2021/11/12.
 */
public class Log {
    private static final String TAG = "xxlog.log";
    public interface LogImp {
        void logV(String tag, String filename, String funcname, int linuxTid, int pid, long tid, long maintid, String log);

        void logI(String tag, String filename, String funcname, int linuxTid, int pid, long tid, long maintid, String log);

        void logD(String tag, String filename, String funcname, int linuxTid, int pid, long tid, long maintid, String log);

        void logW(String tag, String filename, String funcname, int linuxTid, int pid, long tid, long maintid, String log);

        void logE(String tag, String filename, String funcname, int linuxTid, int pid, long tid, long maintid, String log);

        void logF( String tag, String filename, String funcname, int linuxTid, int pid, long tid, long maintid, String log);

        int getLogLevel();

        void setAppenderMode( int mode);

        void appenderOpen(int level, int mode, String cacheDir, String logDir, String nameprefix, int cacheDays, String pubkey);

        void appenderClose();

        void appenderFlush(boolean isSync);

        void setConsoleLogOpen(boolean isOpen);

        void setMaxFileSize(long aliveSeconds);

        void setMaxAliveTime(long aliveSeconds);
    }

    // defaults to LEVEL_NONE
    private static int level = LEVEL_NONE;

    private static LogImp debugLog = new LogImp() {
        private Handler handler = new Handler(Looper.getMainLooper());

        @Override
        public void logV(String tag, String filename, String funcname, int line, int pid, long tid, long maintid, String log) {
            if (level <= LEVEL_VERBOSE) {
                android.util.Log.v(tag, log);
            }
        }

        @Override
        public void logI(String tag, String filename, String funcname, int line, int pid, long tid, long maintid, String log) {
            if (level <= LEVEL_INFO) {
                android.util.Log.i(tag, log);
            }
        }

        @Override
        public void logD(String tag, String filename, String funcname, int line, int pid, long tid, long maintid, String log) {
            if (level <= LEVEL_DEBUG) {
                android.util.Log.d(tag, log);
            }

        }

        @Override
        public void logW(String tag, String filename, String funcname, int line, int pid, long tid, long maintid, String log) {
            if (level <= LEVEL_WARNING) {
                android.util.Log.w(tag, log);
            }

        }

        @Override
        public void logE(String tag, String filename, String funcname, int line, int pid, long tid, long maintid, String log) {
            if (level <= LEVEL_ERROR) {
                android.util.Log.e(tag, log);
            }
        }


        @Override
        public void logF(String tag, String filename, String funcname, int line, int pid, long tid, long maintid, final String log) {
            if (level > LEVEL_FATAL) {
                return;
            }
            android.util.Log.e(tag, log);
        }

        @Override
        public int getLogLevel() {
            return level;
        }

        @Override
        public void setAppenderMode(int mode) {

        }

        @Override
        public void appenderOpen(int level, int mode, String cacheDir, String logDir, String nameprefix, int cacheDays, String pubkey) {

        }

        @Override
        public void appenderClose() {

        }

        @Override
        public void appenderFlush(boolean isSync) {
        }

        @Override
        public void setConsoleLogOpen(boolean isOpen) {

        }

        @Override
        public void setMaxAliveTime(long aliveSeconds) {

        }

        @Override
        public void setMaxFileSize(long aliveSeconds) {

        }

    };

    private static LogImp logImp = debugLog;

    public static void setLogImp(LogImp imp) {
        logImp = imp;
    }

    public static LogImp getImpl() {
        return logImp;
    }

    public static void appenderOpen(int level, int mode, String cacheDir, String logDir, String nameprefix, int cacheDays, String pubkey) {
        if (logImp != null) {
            logImp.appenderOpen(level, mode, cacheDir, logDir, nameprefix, cacheDays, pubkey);
        }
    }

    public static void appenderClose() {
        if (logImp != null) {
            logImp.appenderClose();
        }
    }

    public static void appenderFlush() {
        if (logImp != null) {
            logImp.appenderFlush(false);
        }
    }

    public static void appenderFlushSync(boolean isSync) {
        if (logImp != null) {
            logImp.appenderFlush(isSync);

        }
    }

    public static int getLogLevel() {
        if (logImp != null) {
            return logImp.getLogLevel();
        }
        return LEVEL_NONE;
    }

    public static void setLevel(final int level, final boolean jni) {
        Log.level = level;
        android.util.Log.w(TAG, "new log level: " + level);

        if (jni) {
            android.util.Log.e(TAG, "no jni log level support");
        }
    }

    public static void setConsoleLogOpen(boolean isOpen) {
        if (logImp != null) {
            logImp.setConsoleLogOpen(isOpen);
        }
    }

    /**
     * use f(tag, format, obj) instead
     *
     * @param tag
     * @param msg
     */
    public static void f(final String tag, final String msg) {
        f(tag, msg, (Object[]) null);
    }

    /**
     * use e(tag, format, obj) instead
     *
     * @param tag
     * @param msg
     */
    public static void e(final String tag, final String msg) {
        e(tag, msg, (Object[]) null);
    }

    /**
     * use w(tag, format, obj) instead
     *
     * @param tag
     * @param msg
     */
    public static void w(final String tag, final String msg) {
        w(tag, msg, (Object[]) null);
    }

    /**
     * use i(tag, format, obj) instead
     *
     * @param tag
     * @param msg
     */
    public static void i(final String tag, final String msg) {
        i(tag, msg, (Object[]) null);
    }

    /**
     * use d(tag, format, obj) instead
     *
     * @param tag
     * @param msg
     */
    public static void d(final String tag, final String msg) {
        d(tag, msg, (Object[]) null);
    }

    /**
     * use v(tag, format, obj) instead
     *
     * @param tag
     * @param msg
     */
    public static void v(final String tag, final String msg) {
        v(tag, msg, (Object[]) null);
    }

    public static void f(String tag, final String format, final Object... obj) {
        if (logImp != null && logImp.getLogLevel() <= LEVEL_FATAL) {
            final String log = obj == null ? format : String.format(format, obj);
            logImp.logF( tag, "", "", 0, Process.myPid(), Thread.currentThread().getId(), Looper.getMainLooper().getThread().getId(), log);
        }
    }

    public static void e(String tag, final String format, final Object... obj) {
        if (logImp != null && logImp.getLogLevel() <= LEVEL_ERROR) {
            String log = obj == null ? format : String.format(format, obj);
            if (log == null) {
                log = "";
            }
            logImp.logE(tag, "", "", 0, Process.myPid(), Thread.currentThread().getId(), Looper.getMainLooper().getThread().getId(), log);
        }
    }

    public static void w(String tag, final String format, final Object... obj) {
        if (logImp != null && logImp.getLogLevel() <= LEVEL_WARNING) {
            String log = obj == null ? format : String.format(format, obj);
            if (log == null) {
                log = "";
            }
            logImp.logW(tag, "", "", 0, Process.myPid(), Thread.currentThread().getId(), Looper.getMainLooper().getThread().getId(), log);
        }
    }

    public static void i(String tag, final String format, final Object... obj) {
        if (logImp != null && logImp.getLogLevel() <= LEVEL_INFO) {
            String log = obj == null ? format : String.format(format, obj);
            if (log == null) {
                log = "";
            }
            logImp.logI(tag, "", "", 0, Process.myPid(), Thread.currentThread().getId(), Looper.getMainLooper().getThread().getId(), log);
        }
    }

    public static void d(String tag, final String format, final Object... obj) {
        if (logImp != null && logImp.getLogLevel() <= LEVEL_DEBUG) {
            String log = obj == null ? format : String.format(format, obj);
            if (log == null) {
                log = "";
            }
            logImp.logD(tag, "", "", 0, Process.myPid(), Thread.currentThread().getId(), Looper.getMainLooper().getThread().getId(), log);
        }
    }

    public static void v(String tag, final String format, final Object... obj) {
        if (logImp != null && logImp.getLogLevel() <= LEVEL_VERBOSE) {
            String log = obj == null ? format : String.format(format, obj);
            if (log == null) {
                log = "";
            }
            logImp.logV(tag, "", "", 0, Process.myPid(), Thread.currentThread().getId(), Looper.getMainLooper().getThread().getId(), log);
        }
    }

    public static void printErrStackTrace(String tag, Throwable tr, final String format, final Object... obj) {
        if (logImp != null && logImp.getLogLevel() <= LEVEL_ERROR) {
            String log = obj == null ? format : String.format(format, obj);
            if (log == null) {
                log = "";
            }
            log += "  " + android.util.Log.getStackTraceString(tr);
            logImp.logE(tag, "", "", 0, Process.myPid(), Thread.currentThread().getId(), Looper.getMainLooper().getThread().getId(), log);
        }
    }

    private static final String SYS_INFO;

    static {
        final StringBuilder sb = new StringBuilder();
        try {
            sb.append("VERSION.RELEASE:[" + android.os.Build.VERSION.RELEASE);
            sb.append("] VERSION.CODENAME:[" + android.os.Build.VERSION.CODENAME);
            sb.append("] VERSION.INCREMENTAL:[" + android.os.Build.VERSION.INCREMENTAL);
            sb.append("] BOARD:[" + android.os.Build.BOARD);
            sb.append("] DEVICE:[" + android.os.Build.DEVICE);
            sb.append("] DISPLAY:[" + android.os.Build.DISPLAY);
            sb.append("] FINGERPRINT:[" + android.os.Build.FINGERPRINT);
            sb.append("] HOST:[" + android.os.Build.HOST);
            sb.append("] MANUFACTURER:[" + android.os.Build.MANUFACTURER);
            sb.append("] MODEL:[" + android.os.Build.MODEL);
            sb.append("] PRODUCT:[" + android.os.Build.PRODUCT);
            sb.append("] TAGS:[" + android.os.Build.TAGS);
            sb.append("] TYPE:[" + android.os.Build.TYPE);
            sb.append("] USER:[" + android.os.Build.USER + "]");
        } catch (Throwable e) {
            e.printStackTrace();
        }

        SYS_INFO = sb.toString();
    }

    public static String getSysInfo() {
        return SYS_INFO;
    }

}
