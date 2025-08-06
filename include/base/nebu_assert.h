#ifndef NEBU_ASSERT_H
#define NEBU_ASSERT_H

// Android stub for nebu_assert.h
#include <android/log.h>
#include <assert.h>

#ifndef LOG_TAG
#define LOG_TAG "GLTron_Assert"
#endif
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

#ifdef DEBUG
#define nebu_assert(condition) \
    do { \
        if (!(condition)) { \
            LOGE("Assertion failed: %s at %s:%d", #condition, __FILE__, __LINE__); \
            assert(condition); \
        } \
    } while(0)
#else
#define nebu_assert(condition) ((void)0)
#endif

#endif // NEBU_ASSERT_H

