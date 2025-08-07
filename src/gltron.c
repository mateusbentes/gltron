/* GLTron main entry point - Universal (Linux + Android) */
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "base/nebu_system.h"
#include "base/nebu_assert.h"
#include "base/nebu_debug_memory.h"
#include "base/nebu_util.h"

#include "game/init.h"
#include "game/menu.h"
#include "video/nebu_video_system.h"

#ifdef ANDROID
#include "android_config.h"
#include "Nebu_filesystem.h"
#include "Nebu_scripting.h"
#include "game/game.h"
#include "android_audio.h"
#include "android_resolution.h"
#include "android_settings_menu.h"
#include <jni.h>
#include <android/log.h>

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "GLTron", __VA_ARGS__))
#endif

#include "gltron-config.h"

// Android main function
#ifdef ANDROID
int android_main(int argc, char *argv[]) {
    LOGI("GLTron Android starting...");

    // Initialize Android systems
    android_audio_init();
    android_resolution_init(1920, 1080); // Default resolution
    android_settings_menu_init();

    // Load settings
    android_audio_load_settings();
    android_resolution_load_settings();

    LOGI("GLTron Android initialized successfully");

    // Main game loop would go here
    // For now, just return success
    return 0;
}

// JNI entry point for Android
JNIEXPORT jint JNICALL
Java_com_gltron_android_MainActivity_nativeMain(JNIEnv *env, jobject thiz) {
    return android_main(0, NULL);
}
#endif

// Linux main function
#ifndef ANDROID
int main(int argc, char *argv[]) {
    printf("GLTron Linux starting...\n");

    // Convert to const char** for compatibility
    const char **argv_const = (const char**)argv;

    // Initialize all subsystems (filesystem, scripting, config, video, audio, input, game data)
    printf("[main] Initializing subsystems...\n");
    initSubsystems(argc, argv_const);

    // Force a reasonable window size if needed
    printf("[main] Setting up video mode...\n");
    int screenWidth = 1024, screenHeight = 768;
    nebu_Video_GetDimension(&screenWidth, &screenHeight);
    printf("[main] Window size: %dx%d\n", screenWidth, screenHeight);

    // Initialize the menu system
    printf("[main] Initializing menu system...\n");
    initMenu();

    // Set up menu callbacks with the Nebu system
    printf("[main] Setting up menu callbacks...\n");
    nebu_System_SetCallback_Display(drawMenu);
    nebu_System_SetCallback_Idle(menuIdle);  // Now properly defined
    nebu_System_SetCallback_Key(keyMenu);    // Now properly defined
    nebu_System_SetCallback_Mouse(mouseMenu);  // Now properly defined

    // Run the main loop until exitGame() calls nebu_System_Exit()
    printf("[main] Starting main loop...\n");
    int result = nebu_System_MainLoop();

    // Clean up all subsystems
    printf("[main] Cleaning up subsystems...\n");
    exitSubsystems();

    printf("[main] Main loop exited with code: %d\n", result);

    printf("GLTron Linux exiting...\n");
    return 0;
}
#endif