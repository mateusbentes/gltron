#ifndef ANDROID_CONFIG_H
#define ANDROID_CONFIG_H

// Android configuration defines
#define ANDROID 1
#define GLES_VERSION 2
#define HAVE_GLES2 1

// Path separators and directories for Android
#define SEPARATOR '/'
#define DATA_DIR "/android_asset"
#define SNAP_DIR "~/snapshots"
#define PREF_DIR "~/gltron"
#define RC_NAME "gltronrc"

// Audio configuration
#define HAVE_OPENSL_ES 1

// Filesystem functions for Android
#include <sys/stat.h>
#include <unistd.h>
#include <android/log.h>

#define LOG_TAG "GLTron"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

static inline int makeDirectory(const char* path) {
    return mkdir(path, 0755);
}

static inline int nebu_FS_Test(const char* path) {
    return access(path, F_OK) == 0;
}

// Memory allocation tracking (disabled for Android)
#define malloc_track(size) malloc(size)
#define free_track(ptr) free(ptr)
#define realloc_track(ptr, size) realloc(ptr, size)

// Debug macros
#ifdef DEBUG
#define nebu_assert(condition) assert(condition)
#else
#define nebu_assert(condition) ((void)0)
#endif

#endif // ANDROID_CONFIG_H

