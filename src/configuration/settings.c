#include <stdio.h>
#include <string.h>
#include "base/nebu_assert.h"

#include "configuration/settings.h"
#include "configuration/configuration.h"
#include "game/gltron.h"  /* Added for RC_NAME definition */

#include "scripting/nebu_scripting.h"
#include "base/util.h"
#include "game/game.h"
#include "filesystem/path.h"

#include "base/nebu_debug_memory.h"

#define BUFSIZE 100
#define MAX_VAR_NAME_LEN 64

void checkSettings(void) {
	/* sanity check: speed */
	if(getSettingf("speed") <= 0) {
	  fprintf(stderr, "[gltron] sanity check failed: speed = %.2ff\n",
		  getSettingf("speed"));
	  setSettingf("speed", 6.0);  // This calls scripting_SetFloat()
	  fprintf(stderr, "[gltron] reset speed: speed = %.2f\n",
		  getSettingf("speed"));
	}
}

void saveSettings(void) {
	printf("[debug] Saving settings (stub)\n");
	/* In a real implementation, you would save settings to a file */
}

int getSettingi(const char *name) {
    printf("[debug] Getting integer setting: %s (stub)\n", name);
    /* Return default values based on the setting name */
    if(strcmp(name, "ai_level") == 0) return 2;
    if(strcmp(name, "show_fps") == 0) return 0;
    /* Add more defaults as needed */
    return 1; /* Default for most boolean settings */
}

int getVideoSettingi(const char *name) {
    printf("[debug] Getting video integer setting: %s (stub)\n", name);
    /* Return default values based on the setting name */
    if(strcmp(name, "width") == 0) return 800;
    if(strcmp(name, "height") == 0) return 600;
    /* Add more defaults as needed */
    return 1; /* Default for most boolean settings */
}

float getSettingf(const char *name) {
    printf("[debug] Getting float setting: %s (stub)\n", name);
    /* Return default values based on the setting name */
    if(strcmp(name, "fov") == 0) return 90.0f;
    if(strcmp(name, "znear") == 0) return 0.5f;
    if(strcmp(name, "speed") == 0) return 6.0f;
    /* Add more defaults as needed */
    return 1.0f; /* Default for most settings */
}

float getVideoSettingf(const char *name) {
    printf("[debug] Getting video float setting: %s (stub)\n", name);
    /* Return default values based on the setting name */
    /* Add specific defaults as needed */
    return 1.0f; /* Default for most settings */
}

int isSetting(const char *name) {
    printf("[debug] Checking if setting exists: %s (stub)\n", name);
    return 1; /* Assume all settings exist */
}

void setSettingf(const char *name, float f) {
	printf("[debug] Setting float setting: %s = %f (stub)\n", name, f);
	/* In a real implementation, you would update the setting in Lua */
}

void setSettingi(const char *name, int i) {
	setSettingf(name, (float)i);
}

void updateSettingsCache(void) {
    printf("[debug] Updating settings cache with default values\n");
    
    /* Initialize with default values instead of reading from Lua */
    gSettingsCache.use_stencil = 1;
    gSettingsCache.show_scores = 1;
    gSettingsCache.show_ai_status = 1;
    gSettingsCache.ai_level = 2;
    gSettingsCache.show_fps = 0;
    gSettingsCache.show_console = 1;
    gSettingsCache.softwareRendering = 0;
    gSettingsCache.line_spacing = 1;
    gSettingsCache.alpha_trails = 1;
    gSettingsCache.antialias_lines = 1;
    gSettingsCache.turn_cycle = 1;
    gSettingsCache.light_cycles = 1;
    gSettingsCache.lod = 1;
    gSettingsCache.fov = 90.0f;
    
    gSettingsCache.show_floor_texture = 1;
    gSettingsCache.show_skybox = 1;
    gSettingsCache.show_wall = 1;
    gSettingsCache.stretch_textures = 0;
    gSettingsCache.show_decals = 1;
    
    gSettingsCache.show_impact = 1;
    gSettingsCache.show_glow = 1;
    gSettingsCache.show_recognizer = 1;
    
    gSettingsCache.fast_finish = 0;
    gSettingsCache.znear = 0.5f;
    gSettingsCache.camType = 0;
    gSettingsCache.playEffects = 1;
    gSettingsCache.playMusic = 1;
    
    /* Set default clear color */
    gSettingsCache.clear_color[0] = 0.0f;
    gSettingsCache.clear_color[1] = 0.0f;
    gSettingsCache.clear_color[2] = 0.0f;
    gSettingsCache.clear_color[3] = 0.0f;
    
    printf("[debug] Settings cache updated with default values\n");
}
