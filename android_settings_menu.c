#include "config.h"

#ifdef ANDROID
#include "android_config.h"
#else
#include "platform_config.h"
#endif
#include "android_audio.h"
#include "android_resolution.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Menu system types
typedef enum {
    MENU_MAIN_SETTINGS,
    MENU_AUDIO_SETTINGS,
    MENU_VIDEO_SETTINGS,
    MENU_CONTROLS_SETTINGS,
    MENU_PERFORMANCE_INFO
} MenuType;

typedef enum {
    SETTING_TYPE_SLIDER,
    SETTING_TYPE_OPTION,
    SETTING_TYPE_BUTTON,
    SETTING_TYPE_INFO
} SettingType;

typedef struct {
    const char* name;
    SettingType type;
    int menu_id;
    int setting_id;
    const char* description;
} MenuSetting;

// Current menu state
typedef struct {
    MenuType current_menu;
    int selected_item;
    int num_items;
    int menu_visible;
    float menu_alpha;
} MenuState;

static MenuState g_menu_state = {
    .current_menu = MENU_MAIN_SETTINGS,
    .selected_item = 0,
    .num_items = 0,
    .menu_visible = 0,
    .menu_alpha = 0.0f
};

// Main settings menu
static MenuSetting main_settings[] = {
    {"Audio Settings", SETTING_TYPE_BUTTON, MENU_AUDIO_SETTINGS, 0, "Configure sound and music"},
    {"Video Settings", SETTING_TYPE_BUTTON, MENU_VIDEO_SETTINGS, 0, "Configure graphics and resolution"},
    {"Controls Settings", SETTING_TYPE_BUTTON, MENU_CONTROLS_SETTINGS, 0, "Configure touch controls"},
    {"Performance Info", SETTING_TYPE_BUTTON, MENU_PERFORMANCE_INFO, 0, "View performance statistics"},
    {"Save Settings", SETTING_TYPE_BUTTON, -1, 0, "Save all settings to file"},
    {"Reset to Defaults", SETTING_TYPE_BUTTON, -1, 1, "Reset all settings to default values"},
    {"Back to Game", SETTING_TYPE_BUTTON, -1, 2, "Return to the game"}
};

// Audio settings menu
static MenuSetting audio_settings[] = {
    {"Master Volume", SETTING_TYPE_SLIDER, 0, 0, "Overall volume level"},
    {"SFX Volume", SETTING_TYPE_SLIDER, 0, 1, "Sound effects volume"},
    {"Music Volume", SETTING_TYPE_SLIDER, 0, 2, "Background music volume"},
    {"Audio Enabled", SETTING_TYPE_OPTION, 0, 0, "Enable or disable all audio"},
    {"3D Audio", SETTING_TYPE_OPTION, 0, 1, "Enable spatial audio effects"},
    {"Audio Quality", SETTING_TYPE_OPTION, 0, 2, "Audio processing quality"},
    {"Back", SETTING_TYPE_BUTTON, MENU_MAIN_SETTINGS, 0, "Return to main settings"}
};

// Video settings menu
static MenuSetting video_settings[] = {
    {"Resolution", SETTING_TYPE_OPTION, 1, 0, "Rendering resolution"},
    {"Graphics Quality", SETTING_TYPE_OPTION, 1, 1, "Overall graphics quality"},
    {"VSync", SETTING_TYPE_OPTION, 1, 2, "Vertical synchronization"},
    {"Auto-Detect Optimal", SETTING_TYPE_BUTTON, -1, 10, "Automatically detect best settings"},
    {"Back", SETTING_TYPE_BUTTON, MENU_MAIN_SETTINGS, 0, "Return to main settings"}
};

// Controls settings menu
static MenuSetting controls_settings[] = {
    {"Touch Sensitivity", SETTING_TYPE_SLIDER, 2, 0, "Touch input sensitivity"},
    {"Swipe Threshold", SETTING_TYPE_SLIDER, 2, 1, "Minimum swipe distance"},
    {"Vibration", SETTING_TYPE_OPTION, 2, 0, "Haptic feedback on actions"},
    {"Show Touch Areas", SETTING_TYPE_OPTION, 2, 1, "Display touch control areas"},
    {"Back", SETTING_TYPE_BUTTON, MENU_MAIN_SETTINGS, 0, "Return to main settings"}
};

// Get current menu settings
static MenuSetting* get_current_menu_settings(int* count) {
    switch (g_menu_state.current_menu) {
        case MENU_MAIN_SETTINGS:
            *count = sizeof(main_settings) / sizeof(MenuSetting);
            return main_settings;
        case MENU_AUDIO_SETTINGS:
            *count = sizeof(audio_settings) / sizeof(MenuSetting);
            return audio_settings;
        case MENU_VIDEO_SETTINGS:
            *count = sizeof(video_settings) / sizeof(MenuSetting);
            return video_settings;
        case MENU_CONTROLS_SETTINGS:
            *count = sizeof(controls_settings) / sizeof(MenuSetting);
            return controls_settings;
        default:
            *count = 0;
            return NULL;
    }
}

// Menu navigation functions
void android_settings_menu_show() {
    g_menu_state.menu_visible = 1;
    g_menu_state.current_menu = MENU_MAIN_SETTINGS;
    g_menu_state.selected_item = 0;
    
    int count;
    get_current_menu_settings(&count);
    g_menu_state.num_items = count;
    
    LOGI("Settings menu opened");
}

void android_settings_menu_hide() {
    g_menu_state.menu_visible = 0;
    LOGI("Settings menu closed");
}

int android_settings_menu_is_visible() {
    return g_menu_state.menu_visible;
}

void android_settings_menu_navigate_up() {
    if (g_menu_state.selected_item > 0) {
        g_menu_state.selected_item--;
    } else {
        g_menu_state.selected_item = g_menu_state.num_items - 1;
    }
    LOGI("Menu navigation: item %d selected", g_menu_state.selected_item);
}

void android_settings_menu_navigate_down() {
    if (g_menu_state.selected_item < g_menu_state.num_items - 1) {
        g_menu_state.selected_item++;
    } else {
        g_menu_state.selected_item = 0;
    }
    LOGI("Menu navigation: item %d selected", g_menu_state.selected_item);
}

void android_settings_menu_navigate_left() {
    int count;
    MenuSetting* settings = get_current_menu_settings(&count);
    if (!settings || g_menu_state.selected_item >= count) return;
    
    MenuSetting* setting = &settings[g_menu_state.selected_item];
    
    if (setting->type == SETTING_TYPE_SLIDER) {
        // Decrease slider value
        if (setting->menu_id == 0) { // Audio settings
            gltron_audio_settings_adjust_slider(setting->setting_id, -1.0f);
        } else if (setting->menu_id == 2) { // Controls settings
            // Handle controls sliders
            LOGI("Decreasing controls setting %d", setting->setting_id);
        }
    } else if (setting->type == SETTING_TYPE_OPTION) {
        // Cycle option backwards
        if (setting->menu_id == 0) { // Audio settings
            gltron_audio_settings_cycle_option(setting->setting_id);
        } else if (setting->menu_id == 1) { // Video settings
            if (setting->setting_id == 0) { // Resolution
                int current = android_resolution_get_current_index();
                int count = android_resolution_get_count();
                android_resolution_set_resolution((current - 1 + count) % count);
            } else if (setting->setting_id == 1) { // Quality
                int current = android_resolution_get_current_quality_index();
                int count = android_resolution_get_quality_count();
                android_resolution_set_quality((current - 1 + count) % count);
            } else if (setting->setting_id == 2) { // VSync
                android_resolution_set_vsync(!android_resolution_get_vsync());
            }
        }
    }
}

void android_settings_menu_navigate_right() {
    int count;
    MenuSetting* settings = get_current_menu_settings(&count);
    if (!settings || g_menu_state.selected_item >= count) return;
    
    MenuSetting* setting = &settings[g_menu_state.selected_item];
    
    if (setting->type == SETTING_TYPE_SLIDER) {
        // Increase slider value
        if (setting->menu_id == 0) { // Audio settings
            gltron_audio_settings_adjust_slider(setting->setting_id, 1.0f);
        } else if (setting->menu_id == 2) { // Controls settings
            // Handle controls sliders
            LOGI("Increasing controls setting %d", setting->setting_id);
        }
    } else if (setting->type == SETTING_TYPE_OPTION) {
        // Cycle option forwards
        if (setting->menu_id == 0) { // Audio settings
            gltron_audio_settings_cycle_option(setting->setting_id);
        } else if (setting->menu_id == 1) { // Video settings
            if (setting->setting_id == 0) { // Resolution
                int current = android_resolution_get_current_index();
                int count = android_resolution_get_count();
                android_resolution_set_resolution((current + 1) % count);
            } else if (setting->setting_id == 1) { // Quality
                int current = android_resolution_get_current_quality_index();
                int count = android_resolution_get_quality_count();
                android_resolution_set_quality((current + 1) % count);
            } else if (setting->setting_id == 2) { // VSync
                android_resolution_set_vsync(!android_resolution_get_vsync());
            }
        }
    }
}

void android_settings_menu_select() {
    int count;
    MenuSetting* settings = get_current_menu_settings(&count);
    if (!settings || g_menu_state.selected_item >= count) return;
    
    MenuSetting* setting = &settings[g_menu_state.selected_item];
    
    if (setting->type == SETTING_TYPE_BUTTON) {
        if (setting->menu_id >= 0) {
            // Navigate to submenu
            g_menu_state.current_menu = (MenuType)setting->menu_id;
            g_menu_state.selected_item = 0;
            get_current_menu_settings(&g_menu_state.num_items);
            LOGI("Navigated to submenu %d", setting->menu_id);
        } else {
            // Handle special buttons
            switch (setting->setting_id) {
                case 0: // Save settings
                    android_audio_save_settings();
                    android_resolution_save_settings();
                    LOGI("All settings saved");
                    break;
                case 1: // Reset to defaults
                    android_resolution_auto_detect_optimal();
                    gltron_audio_settings_set_master_volume(1.0f);
                    gltron_audio_settings_set_sfx_volume(1.0f);
                    gltron_audio_settings_set_music_volume(0.7f);
                    LOGI("Settings reset to defaults");
                    break;
                case 2: // Back to game
                    android_settings_menu_hide();
                    break;
                case 10: // Auto-detect optimal
                    android_resolution_auto_detect_optimal();
                    LOGI("Auto-detected optimal video settings");
                    break;
            }
        }
    }
}

// Get current menu info for rendering
const char* android_settings_menu_get_current_title() {
    switch (g_menu_state.current_menu) {
        case MENU_MAIN_SETTINGS: return "Settings";
        case MENU_AUDIO_SETTINGS: return "Audio Settings";
        case MENU_VIDEO_SETTINGS: return "Video Settings";
        case MENU_CONTROLS_SETTINGS: return "Controls Settings";
        case MENU_PERFORMANCE_INFO: return "Performance Info";
        default: return "Unknown";
    }
}

int android_settings_menu_get_item_count() {
    return g_menu_state.num_items;
}

int android_settings_menu_get_selected_item() {
    return g_menu_state.selected_item;
}

const char* android_settings_menu_get_item_name(int index) {
    int count;
    MenuSetting* settings = get_current_menu_settings(&count);
    if (settings && index >= 0 && index < count) {
        return settings[index].name;
    }
    return NULL;
}

const char* android_settings_menu_get_item_value(int index) {
    int count;
    MenuSetting* settings = get_current_menu_settings(&count);
    if (!settings || index < 0 || index >= count) return NULL;
    
    MenuSetting* setting = &settings[index];
    static char value_buffer[64];
    
    if (setting->type == SETTING_TYPE_SLIDER) {
        if (setting->menu_id == 0) { // Audio settings
            float value = gltron_audio_settings_get_slider_value(setting->setting_id);
            snprintf(value_buffer, sizeof(value_buffer), "%.1f", value);
            return value_buffer;
        }
    } else if (setting->type == SETTING_TYPE_OPTION) {
        if (setting->menu_id == 0) { // Audio settings
            return gltron_audio_settings_get_option_value(setting->setting_id);
        } else if (setting->menu_id == 1) { // Video settings
            if (setting->setting_id == 0) { // Resolution
                int current = android_resolution_get_current_index();
                return android_resolution_get_name(current);
            } else if (setting->setting_id == 1) { // Quality
                int current = android_resolution_get_current_quality_index();
                return android_resolution_get_quality_name(current);
            } else if (setting->setting_id == 2) { // VSync
                return android_resolution_get_vsync() ? "Enabled" : "Disabled";
            }
        }
    }
    
    return "";
}

const char* android_settings_menu_get_item_description(int index) {
    int count;
    MenuSetting* settings = get_current_menu_settings(&count);
    if (settings && index >= 0 && index < count) {
        return settings[index].description;
    }
    return NULL;
}

// Performance info display
void android_settings_menu_show_performance_info() {
    g_menu_state.current_menu = MENU_PERFORMANCE_INFO;
    g_menu_state.selected_item = 0;
    g_menu_state.num_items = 1; // Just a back button
    
    float fps, frame_time;
    android_resolution_get_performance_stats(&fps, &frame_time);
    
    int render_width, render_height;
    android_resolution_get_current_resolution(&render_width, &render_height);
    
    LOGI("Performance Info:");
    LOGI("  FPS: %.1f", fps);
    LOGI("  Frame Time: %.2f ms", frame_time);
    LOGI("  Render Resolution: %dx%d", render_width, render_height);
    LOGI("  Audio Enabled: %s", gltron_audio_settings_is_audio_enabled() ? "Yes" : "No");
}

// Update menu animation
void android_settings_menu_update(float delta_time) {
    if (g_menu_state.menu_visible) {
        if (g_menu_state.menu_alpha < 1.0f) {
            g_menu_state.menu_alpha += delta_time * 3.0f; // Fade in
            if (g_menu_state.menu_alpha > 1.0f) {
                g_menu_state.menu_alpha = 1.0f;
            }
        }
    } else {
        if (g_menu_state.menu_alpha > 0.0f) {
            g_menu_state.menu_alpha -= delta_time * 3.0f; // Fade out
            if (g_menu_state.menu_alpha < 0.0f) {
                g_menu_state.menu_alpha = 0.0f;
            }
        }
    }
}

float android_settings_menu_get_alpha() {
    return g_menu_state.menu_alpha;
}

