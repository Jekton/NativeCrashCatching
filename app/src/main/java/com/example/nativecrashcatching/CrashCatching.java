package com.example.nativecrashcatching;

import android.util.Log;

/**
 * @author luojitong
 */
public abstract class CrashCatching {
    private static final String TAG = "CrashCatching";

    static {
        System.loadLibrary("native-lib");
    }

    private CrashCatching() {
    }

    public static void init() {
        initNative();
    }

    public static void die() {
        Log.d("Jekton", TAG + ".die: ");
        dieNative();
    }


    private native static void initNative();
    private native static void dieNative();
}
