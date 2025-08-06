#include "config.h"

#ifdef ANDROID
#include "android_config.h"
#else
#include "platform_config.h"
#endif
#include <GLES2/gl2.h>

// OpenGL stubs for compatibility
void glColor4f(float r, float g, float b, float a) {
    // OpenGL ES doesn't have glColor4f, this is a stub
    LOGI("glColor4f stub called: %f, %f, %f, %f", r, g, b, a);
}

// Font rendering stub
void nebu_Font_RenderToBox(void* font, const char* text, int flags, void* box, int align) {
    LOGI("nebu_Font_RenderToBox stub called: %s", text);
}

// System stubs
void nebu_System_SwapBuffers(void) {
    LOGI("nebu_System_SwapBuffers stub called");
}

void nebu_System_PostRedisplay(void) {
    LOGI("nebu_System_PostRedisplay stub called");
}

// Input constants
#define NEBU_INPUT_KEYSTATE_DOWN 1
#define SYSTEM_KEY_ESCAPE 27

// Memory debug stub
void nebu_debug_memory_CheckLeaksOnExit(void) {
    LOGI("nebu_debug_memory_CheckLeaksOnExit stub called");
}

