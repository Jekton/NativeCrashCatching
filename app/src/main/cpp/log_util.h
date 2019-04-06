//
// Created by Jekton Luo on 3/6/17.
//

#ifndef LOG_UTIL_H_
#define LOG_UTIL_H_

#define DEBUGGING

#include <android/log.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>


#define LOGE(tag, msg, ...) do {                        \
    __android_log_print(ANDROID_LOG_ERROR,              \
                        tag,                            \
                        "%s: " msg,                     \
                        __func__,                       \
                        ##__VA_ARGS__);                 \
} while (0)

#define LOGW(tag, msg, ...) do {                        \
    __android_log_print(ANDROID_LOG_WARN,               \
                        tag,                            \
                        "%s: " msg,                     \
                        __func__,                       \
                        ##__VA_ARGS__);                 \
} while (0)

#define LOGERRNO(tag, msg, ...) do {                        \
    __android_log_print(ANDROID_LOG_ERROR,                  \
                        tag,                                \
                        "%s: " msg ": %s",                  \
                        __func__,                           \
                        ##__VA_ARGS__,                      \
                        strerror(errno));                   \
} while (0)


#define LOGI(tag, msg, ...) do {                        \
    __android_log_print(ANDROID_LOG_INFO,               \
                        tag,                            \
                        "%s: " msg,                     \
                        __func__,                       \
                        ##__VA_ARGS__);                 \
} while (0)


#ifdef DEBUGGING

#define LOGD(tag, msg, ...) do {                        \
    __android_log_print(ANDROID_LOG_DEBUG,              \
                        tag,                            \
                        "%s: " msg,                     \
                        __func__,                       \
                        ##__VA_ARGS__);                 \
} while (0)

#else

#define LOGD(...) do {          \
} while (0)

#endif



#endif



