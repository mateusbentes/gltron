#pragma once
#include "gltron.h"
#ifdef __cplusplus
extern "C" {
#endif

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
// Optional input hooks
// gltron_on_key removed: touch-only control on Android
void gltron_on_touch(float x, float y, int action);
// Reset game callbacks initialization when starting a new game
void reset_game_callbacks_init(void);

#ifdef __cplusplus
}
#endif
