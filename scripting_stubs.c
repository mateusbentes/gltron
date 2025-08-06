#include "scripting/nebu_scripting.h"
#include "android_config.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

int scripting_Init(void) {
    LOGI("scripting_Init stub called");
    return 0;
}

void scripting_Shutdown(void) {
    LOGI("scripting_Shutdown stub called");
}

int scripting_RunScript(const char* script) {
    LOGI("scripting_RunScript stub called with script: %s", script);
    return 0;
}

int scripting_LoadFile(const char* filename) {
    LOGI("scripting_LoadFile stub called for file: %s", filename);
    return 0;
}

void scripting_Run(const char* script) {
    LOGI("scripting_Run stub called with script: %s", script);
}

void scripting_RunFormat(const char* format, ...) {
    char buffer[1024];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    LOGI("scripting_RunFormat stub called with formatted script: %s", buffer);
}

int scripting_GetGlobal(const char* name, ...) {
    LOGI("scripting_GetGlobal stub called for name: %s", name);
    return 0;
}

int scripting_GetFloatResult(float* result) {
    LOGI("scripting_GetFloatResult stub called");
    if (result) *result = 0.0f; // Default value
    return 0;
}

int scripting_IsNil(void) {
    LOGI("scripting_IsNil stub called");
    return 1; // Assume nil for now
}

void scripting_Pop(void) {
    LOGI("scripting_Pop stub called");
}

void scripting_SetFloat(float value, const char* name, ...) {
    LOGI("scripting_SetFloat stub called for %s with value %f", name, value);
}

int scripting_GetFloatArrayResult(float* result, int count) {
    LOGI("scripting_GetFloatArrayResult stub called for %d elements", count);
    if (result) {
        for (int i = 0; i < count; ++i) {
            result[i] = 0.0f; // Default value
        }
    }
    return 0;
}

void scripting_RegisterSettings(void) {
    LOGI("scripting_RegisterSettings stub called");
}


