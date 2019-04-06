//
// Created by Jekton Luo on 2019/4/3.
//
#include <jni.h>

#include "crash_catching.h"
#include "dlopen.h"

extern "C" JNIEXPORT void JNICALL
Java_com_example_nativecrashcatching_CrashCatching_initNative(
    JNIEnv* env, jclass clazz) {
  ndk_init(env);
  InitCrashCaching();
}

void foo() {
  volatile int* p = nullptr;
  *p = 1;
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_nativecrashcatching_CrashCatching_dieNative(JNIEnv* env, jclass clazz) {
  foo();
}

