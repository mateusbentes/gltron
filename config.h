/* GLTron Configuration Header */
#ifndef CONFIG_H
#define CONFIG_H

/* Version information */
#define VERSION "0.7.1"
#define PACKAGE "gltron"

/* Platform detection */
#ifdef __ANDROID__
#define ANDROID 1
#else
#define ANDROID 0
#endif

/* Feature flags */
#define USE_SCRIPTING 0
#define USE_OPENGL 1

/* Path separator */
#ifdef _WIN32
#define PATH_SEPARATOR '\\'
#else
#define PATH_SEPARATOR '/'
#endif

/* Debug macros for non-Android platforms */
#include <stdio.h>
#if ANDROID
#include <android/log.h>
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "GLTron", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "GLTron", __VA_ARGS__)
#else
#define LOGI(...) printf("[INFO] " __VA_ARGS__); printf("\n")
#define LOGE(...) fprintf(stderr, "[ERROR] " __VA_ARGS__); fprintf(stderr, "\n")
#endif

#endif /* CONFIG_H */
