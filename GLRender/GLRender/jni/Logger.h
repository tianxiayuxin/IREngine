#include <android/log.h>
#include <assert.h>

#define LOG_TAG "IRBox"

#define LOG_INFO(Tag, ...) __android_log_print(ANDROID_LOG_INFO, Tag, __VA_ARGS__)
#define LOG_ERROR(Tag, ...) __android_log_print(ANDROID_LOG_ERROR, Tag, __VA_ARGS__)
#define LOG_WARNING(Tag, ...) __android_log_print(ANDROID_LOG_WARN, Tag, _VA_ARGS__)

#define ASSERT(exp) assert(exp)


