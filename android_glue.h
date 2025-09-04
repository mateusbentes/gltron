#pragma once
#include "gltron.h"
#ifdef __cplusplus
extern "C" {
#endif

// Android-specific callback management
typedef struct {
    void (*display)(void);
    void (*idle)(void);
    void (*keyboard)(unsigned char, int, int);
    void (*special)(int, int, int);
    void (*init)(void);
    void (*initGL)(void);
} android_callbacks;

// Extern callback states
extern android_callbacks current_android_callbacks;
extern android_callbacks last_android_callbacks;
extern struct android_app* state;

// Initialize game structures and resources
void gltron_init(void);
// Resize callback from Android app when surface size changes
void gltron_resize(int width, int height);
// Render/update a single frame (to be called by Android render loop)
void gltron_frame(void);
// Provide AAssetManager* from Java/NativeActivity
void gltron_set_asset_manager(void* asset_mgr);
// Set base writable path for extracted assets (e.g., app files dir)
void gltron_set_base_path(const char* base_path);
// Expose base path buffer for asset extraction
#ifndef PATH_MAX
#define PATH_MAX 1024
#endif
extern char s_base_path[PATH_MAX];

extern int g_finish_requested;
extern struct android_app* state;
extern int did_first_render;
extern android_callbacks current_android_callbacks;
extern android_callbacks last_android_callbacks;
extern int initialized;

// Optional input hooks
// gltron_on_key removed: touch-only control on Android
void gltron_on_touch(float x, float y, int action);

// Reset game callbacks initialization when starting a new game
void reset_game_callbacks_init(void);

// Function declarations from android_glue.c used in android_main.c
extern void draw_android_overlay(void);

// Extern globals for access in other files
extern android_callbacks current_android_callbacks;
extern android_callbacks last_android_callbacks;

// Any other global variables or functions used in android_main.c should be declared here

// For logging and internal use
extern int initialized;

// Function to restore callbacks from last state
extern void android_restoreCallbacks(void);
extern void android_switchCallbacks(callbacks* new);

// Touch control state or overlay control (if used)
// extern int active_left, active_right, active_pause;

// Ensure all global vars are declared that android_main.c accesses

#ifdef __cplusplus
}
#endif
