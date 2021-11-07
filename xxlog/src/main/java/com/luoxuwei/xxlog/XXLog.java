package com.luoxuwei.xxlog;

/**
 * Created by 罗旭维 on 2021/11/7.
 */
public class XXLog {
    static {
        System.loadLibrary("native-lib");
    }

    public native String stringFromJNI();
}
