#include <android/log.h>
#include <assert.h>

#define LOG_INFO(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOG_ERROR(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOG_WARNING(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, _VA_ARGS__)

#define ASSERT(exp) assert(exp)
