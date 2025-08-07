/* GLTron main entry point - Universal (Linux + Android) */

#include "config.h"

#ifdef ANDROID
#include "android_config.h"
#include "base/nebu_system.h"
#include "base/nebu_assert.h"
#include "base/nebu_debug_memory.h"
#include "base/nebu_util.h"
#include "Nebu_filesystem.h"
#include "Nebu_scripting.h"
#include "game/game.h"
#include "android_audio.h"
#include "android_resolution.h"
#include "android_settings_menu.h"
#else
#include "platform_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "game/init.h"
#include "game/menu.h"
#include "base/nebu_system.h"
#include "video/nebu_video_system.h"
#endif

#include "gltron-config.h"

#ifdef ANDROID
// Android main function
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
#include <jni.h>
JNIEXPORT jint JNICALL
Java_com_gltron_android_MainActivity_nativeMain(JNIEnv *env, jobject thiz) {
    return android_main(0, NULL);
}

#else
// Linux main function
int main(int argc, char *argv[]) {
    printf("GLTron Linux starting...\n");
    
    // Convert to const char** for compatibility
    const char **argv_const = (const char**)argv;
    
    // Initialize all subsystems (filesystem, scripting, config, video, audio, input, game data)
    printf("[main] Initializing subsystems...\n");
    initSubsystems(argc, argv_const);
    
    // Initialize the menu system
    printf("[main] Initializing menu system...\n");
    initMenu();
    
    // Set up menu callbacks with the Nebu system
    printf("[main] Setting up menu callbacks...\n");
    nebu_System_SetCallback_Display(drawMenu);
    nebu_System_SetCallback_Idle(menuIdle);
    nebu_System_SetCallback_Key(keyMenu);
    nebu_System_SetCallback_Mouse(mouseMenu);
    
    // Force a reasonable window size if needed
    printf("[main] Setting up video mode...\n");
    int screenWidth = 1024, screenHeight = 768;
    nebu_Video_GetDimension(&screenWidth, &screenHeight);
    printf("[main] Window size: %dx%d\n", screenWidth, screenHeight);
    
    // Run the main loop until exitGame() calls nebu_System_Exit()
    printf("[main] Starting main loop...\n");
    nebu_System_MainLoop();
    
    // Clean up all subsystems
    printf("[main] Cleaning up subsystems...\n");
    exitSubsystems();
    
    printf("GLTron Linux exiting...\n");
    return 0;
}
#endif


