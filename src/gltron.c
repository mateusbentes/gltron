/*
  gltron
  Copyright (C) 1999 by Andreas Umbach <marvin@dataway.ch>
*/

#include "config.h"
#include "gltron-config.h"
#include <stdlib.h>
#include <string.h>

#include "base/nebu_system.h"
#include "base/nebu_assert.h"
#include "base/nebu_debug_memory.h"
#include "base/nebu_util.h"
#include "game/init.h"
#include "game/menu.h"
#include "video/nebu_video_system.h"
#include "game/menu.h"

// Platform-specific includes and definitions
#if GLTRON_BUILD_ANDROID
#if defined(HAVE_JNI_H)
#include <jni.h>
#include <android/log.h>
#endif
#include "android_config.h"
#include "Nebu_filesystem.h"
#include "Nebu_scripting.h"
#include "game/game.h"
#include "android/android_audio.h"
#include "android/android_resolution.h"
#include "android/android_settings_menu.h"

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "GLTron", __VA_ARGS__))
#define LOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, "GLTron", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "GLTron", __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "GLTron", __VA_ARGS__))

#else // PC (not Android)
#include <stdio.h>

#define LOGI(...) printf("[INFO] " __VA_ARGS__); printf("\n")
#define LOGD(...) printf("[DEBUG] " __VA_ARGS__); printf("\n")
#define LOGW(...) printf("[WARN] " __VA_ARGS__); printf("\n")
#define LOGE(...) fprintf(stderr, "[ERROR] " __VA_ARGS__); fprintf(stderr, "\n")
#endif

// Main function that handles both Android and PC builds
int main(int argc, char *argv[]) {
    // Platform-specific initialization
#if GLTRON_BUILD_ANDROID
    LOGI("GLTron Android starting...");
    android_audio_init();
    android_resolution_init(1920, 1080);
    android_settings_menu_init();
    android_audio_load_settings();
    android_resolution_load_settings();
#else
    LOGI("GLTron PC starting...");
    nebu_debug_memory_CheckLeaksOnExit();
    initSubsystems(argc, (const char**)argv);

    // Get screen dimensions
    int screenWidth = 800, screenHeight = 600;
#endif
#if GLTRON_BUILD_ANDROID
    screenWidth = 1920;
    screenHeight = 1080;
    nebu_Video_GetDimension(&screenWidth, &screenHeight);
    LOGI("[main] Window size: %dx%d", screenWidth, screenHeight);

    // Initialize menu system
    initMenu();
    initGuiMenuItems();
    activateMenu();

    // Set appropriate callbacks
    nebu_System_SetCallback_Display(displayMenuCallback);
    nebu_System_SetCallback_Idle(menuIdle);
#endif
#if GLTRON_BUILD_ANDROID
    nebu_System_SetCallback_Key((void*)keyGuiMenu);
    nebu_System_SetCallback_Touch((void*)touchGuiMenu);
#else
    nebu_System_SetCallback_Key((void*)keyGuiMenu);
    nebu_System_SetCallback_Mouse((void*)mouseGuiMenu);
#endif

    LOGI("GLTron menu initialized successfully");

    // Main game loop
    int result = nebu_System_MainLoop();

    // Platform-specific cleanup
#if !GLTRON_BUILD_ANDROID
    exitSubsystems();
#endif

    LOGI("GLTron exiting with code %d", result);
    return result;
}

// Android JNI entry point (only compiled for Android)
#if GLTRON_BUILD_ANDROID
JNIEXPORT jint JNICALL
Java_com_gltron_android_MainActivity_nativeMain(JNIEnv *env, jobject thiz) {
    return main(0, NULL);
}
#endif
