#ifndef NEBU_SYSTEM_H
#define NEBU_SYSTEM_H

// Android stub for nebu_system.h
#include <android/log.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "GLTron", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "GLTron", __VA_ARGS__)

// Basic system functions
void nebu_System_Init(void);
void nebu_System_Shutdown(void);

#endif // NEBU_SYSTEM_H

