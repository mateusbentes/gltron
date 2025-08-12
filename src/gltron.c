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
#ifdef __ANDROID__
#include <jni.h>
#include <android/log.h>
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

int android_main(int argc, char *argv[]) {
  LOGI("GLTron Android starting...");
  android_audio_init();
  android_resolution_init(1920, 1080);
  android_settings_menu_init();
  android_audio_load_settings();
  android_resolution_load_settings();

  // --- Menu initialization for Android ---
  int screenWidth = 1920, screenHeight = 1080;
  // If you have a way to get actual screen size, use it here
  nebu_Video_GetDimension(&screenWidth, &screenHeight);
  LOGI("[main] Window size: %dx%d", screenWidth, screenHeight);

  initMenu();
  initGuiMenuItems();
  activateMenu();

  nebu_System_SetCallback_Display(displayMenuCallback);
  nebu_System_SetCallback_Idle(menuIdle);
  nebu_System_SetCallback_Key((void*)keyGuiMenu);
  nebu_System_SetCallback_Touch((void*)touchGuiMenu);

  LOGI("GLTron Android menu initialized successfully");

  int result = nebu_System_MainLoop();

  LOGI("GLTron Android exiting with code %d", result);
  return result;
}

JNIEXPORT jint JNICALL
Java_com_gltron_android_MainActivity_nativeMain(JNIEnv *env, jobject thiz) {
  return android_main(0, NULL);
}
#endif

#ifndef __ANDROID__
#include <stdio.h>

#define LOGI(...) printf("[INFO] " __VA_ARGS__); printf("\n")
#define LOGD(...) printf("[DEBUG] " __VA_ARGS__); printf("\n")
#define LOGW(...) printf("[WARN] " __VA_ARGS__); printf("\n")
#define LOGE(...) fprintf(stderr, "[ERROR] " __VA_ARGS__); fprintf(stderr, "\n")

int main(int argc, char *argv[]) {
  LOGI("GLTron PC starting...");
  nebu_debug_memory_CheckLeaksOnExit();

  initSubsystems(argc, (const char**)argv);

  int screenWidth = 800, screenHeight = 600;
  nebu_Video_GetDimension(&screenWidth, &screenHeight);
  LOGI("[main] Window size: %dx%d", screenWidth, screenHeight);

  initMenu();
  initGuiMenuItems();
  activateMenu();

  nebu_System_SetCallback_Display(displayMenuCallback);
  nebu_System_SetCallback_Idle(menuIdle);
  nebu_System_SetCallback_Key((void*)keyGuiMenu);
  nebu_System_SetCallback_Mouse((void*)mouseGuiMenu);

  int result = nebu_System_MainLoop();

  exitSubsystems();

  LOGI("GLTron exiting with code %d", result);
  return result;
}
#endif
