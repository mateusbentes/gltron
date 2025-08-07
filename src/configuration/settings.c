#include <stdio.h>
#include <string.h>
#include <stdlib.h>
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
#define MAX_SETTINGS 100

/* Simple settings storage */
typedef struct {
    char name[MAX_VAR_NAME_LEN];
    float value;
    int is_set;
} Setting;

static Setting settings_storage[MAX_SETTINGS];
static int settings_count = 0;

/* Forward declarations */
static Setting* find_setting(const char *name);
static void set_setting(const char *name, float value);
static float get_setting(const char *name);

/* Find a setting by name */
static Setting* find_setting(const char *name) {
    int i;
    for (i = 0; i < settings_count; i++) {
        if (strcmp(settings_storage[i].name, name) == 0) {
            return &settings_storage[i];
        }
    }
    return NULL;
}

/* Create or update a setting */
static void set_setting(const char *name, float value) {
    Setting *setting = find_setting(name);
    
    if (setting) {
        setting->value = value;
    } else if (settings_count < MAX_SETTINGS) {
        strncpy(settings_storage[settings_count].name, name, MAX_VAR_NAME_LEN - 1);
        settings_storage[settings_count].name[MAX_VAR_NAME_LEN - 1] = '\0';
        settings_storage[settings_count].value = value;
        settings_storage[settings_count].is_set = 1;
        settings_count++;
    } else {
        printf("[settings] Warning: Maximum number of settings reached\n");
    }
}

/* Get a setting value */
static float get_setting(const char *name) {
    Setting *setting = find_setting(name);
    if (setting) {
        return setting->value;
    }
    
    /* Return reasonable defaults for unknown settings */
    if (strcmp(name, "width") == 0) return 800.0f;
    if (strcmp(name, "height") == 0) return 600.0f;
    if (strcmp(name, "display_type") == 0) return 0.0f;
    if (strcmp(name, "players") == 0) return 4.0f;
    if (strcmp(name, "ai_opponents") == 0) return 3.0f;
    if (strcmp(name, "speed") == 0) return 6.0f;
    if (strcmp(name, "fov") == 0) return 90.0f;
    if (strcmp(name, "znear") == 0) return 0.5f;
    
    return 1.0f; /* Default for most boolean settings */
}

/* Initialize default settings */
static void init_default_settings(void) {
    static int initialized = 0;
    if (initialized) return;
    
    initialized = 1; /* Set this first to prevent recursion */
    
    /* Set default resolution and display settings directly */
    set_setting("width", 800.0f);
    set_setting("height", 600.0f);
    set_setting("display_type", 0.0f);  /* Single viewport */
    set_setting("windowMode", 1.0f);    /* Windowed mode */
    set_setting("fullscreen", 0.0f);    /* Not fullscreen */
    
    /* Other default settings */
    set_setting("ai_level", 2.0f);
    set_setting("show_fps", 0.0f);
    set_setting("players", 4.0f);
    set_setting("ai_opponents", 3.0f);
    set_setting("wireframe", 0.0f);
    set_setting("playMusic", 1.0f);
    set_setting("playEffects", 1.0f);
    set_setting("mouse_lock_ingame", 1.0f);
    set_setting("mouse_invert_x", 0.0f);
    set_setting("mouse_invert_y", 0.0f);
    set_setting("camType", 0.0f);
    set_setting("debug_output", 0.0f);
    
    set_setting("speed", 6.0f);
    set_setting("fov", 90.0f);
    set_setting("znear", 0.5f);
    set_setting("fxVolume", 1.0f);
    set_setting("musicVolume", 1.0f);
    
    printf("[settings] Default settings initialized\n");
}

// Define all settings variables
int settings_booster_on = 1;
int settings_wall_accel_on = 1;
int settings_wall_buster_on = 1;
float settings_speed = 6.5f;
int settings_ai_level = 2;
int settings_erase_crashed = 1;
int settings_fast_finish = 0;
int settings_camType = 0;
int settings_display_type = 0;
float settings_map_scale = 1.5f;
int settings_players = 2;
int settings_ai_opponents = 0;
int settings_ai_player1 = 0;
int settings_ai_player2 = 1;
int settings_ai_player3 = 1;
int settings_ai_player4 = 1;
int settings_mouse_lock_ingame = 1;
int settings_mouse_invert_y = 0;
int settings_mouse_invert_x = 0;
int settings_mipmap_filter = 1;
int settings_alpha_trails = 1;
int settings_show_glow = 1;
int settings_cycle_sharp_edges = 1;
float settings_reflection = 0.5f;
int settings_show_model = 1;
int settings_shadow_volumes_cycle = 1;
int settings_arena_outlines = 1;
int settings_show_recognizer = 0;
int settings_lod = 1;
int settings_show_fps = 0;
int settings_show_ai_status = 0;
int settings_show_scores = 1;
int settings_show_speed = 1;
int settings_show_console = 0;
int settings_show_2d = 1;
int settings_fov = 60;
int settings_width = 1024;
int settings_height = 768;
int settings_windowMode = 0;
int settings_playMusic = 1;
int settings_playEffects = 1;
float settings_musicVolume = 0.7f;
float settings_fxVolume = 0.7f;
int settings_loopMusic = 1;
char settings_current_level[256] = "levels/arena1.lvl";
char settings_current_artpack[256] = "artpacks/default";
char settings_current_track[256] = "music/gltron.mp3";

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
    init_default_settings();
    int value = (int)get_setting(name);
    printf("[settings] Getting integer setting: %s = %d\n", name, value);
    return value;
}

int getVideoSettingi(const char *name) {
    init_default_settings();
    int value = (int)get_setting(name);
    printf("[settings] Getting video integer setting: %s = %d\n", name, value);
    return value;
}

float getSettingf(const char *name) {
    init_default_settings();
    float value = get_setting(name);
    printf("[settings] Getting float setting: %s = %f\n", name, value);
    return value;
}

float getVideoSettingf(const char *name) {
    init_default_settings();
    float value = get_setting(name);
    printf("[settings] Getting video float setting: %s = %f\n", name, value);
    return value;
}

int isSetting(const char *name) {
    init_default_settings();
    Setting *setting = find_setting(name);
    int exists = (setting != NULL);
    printf("[settings] Checking if setting exists: %s = %s\n", name, exists ? "yes" : "no");
    return exists;
}

void setSettingf(const char *name, float f) {
    init_default_settings();
    set_setting(name, f);
    printf("[settings] Setting float setting: %s = %f\n", name, f);
}

void setSettingi(const char *name, int i) {
    init_default_settings();
    set_setting(name, (float)i);
    printf("[settings] Setting integer setting: %s = %d\n", name, i);
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