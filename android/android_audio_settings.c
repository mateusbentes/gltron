#include "config.h"

#if defined(__ANDROID__)
#include "android_config.h"
#else
#include "platform_config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// External audio functions
extern void android_audio_set_master_volume(float volume);
extern void android_audio_set_sfx_volume(float volume);
extern void android_audio_set_music_volume(float volume);
extern void android_audio_enable(int enabled);
extern float android_audio_get_master_volume(void);
extern float android_audio_get_sfx_volume(void);
extern float android_audio_get_music_volume(void);
extern int android_audio_is_enabled(void);

// Audio settings structure for persistence
typedef struct {
    float master_volume;
    float sfx_volume;
    float music_volume;
    int audio_enabled;
    int use_3d_audio;
    int audio_quality; // 0=low, 1=medium, 2=high
} PersistentAudioSettings;

static PersistentAudioSettings g_persistent_settings = {
    .master_volume = 1.0f,
    .sfx_volume = 1.0f,
    .music_volume = 0.7f,
    .audio_enabled = 1,
    .use_3d_audio = 1,
    .audio_quality = 1
};

// Audio settings file path
static const char* AUDIO_SETTINGS_FILE = "/data/data/com.gltron.android/files/audio_settings.cfg";

// Load audio settings from file
void android_audio_load_settings() {
    FILE* file = fopen(AUDIO_SETTINGS_FILE, "rb");
    if (file) {
        size_t read = fread(&g_persistent_settings, sizeof(PersistentAudioSettings), 1, file);
        fclose(file);
        
        if (read == 1) {
            // Apply loaded settings
            android_audio_set_master_volume(g_persistent_settings.master_volume);
            android_audio_set_sfx_volume(g_persistent_settings.sfx_volume);
            android_audio_set_music_volume(g_persistent_settings.music_volume);
            android_audio_enable(g_persistent_settings.audio_enabled);
            
            LOGI("Audio settings loaded successfully");
        } else {
            LOGE("Failed to read audio settings file");
        }
    } else {
        LOGI("Audio settings file not found, using defaults");
        android_audio_save_settings(); // Create default settings file
    }
}

// Save audio settings to file
void android_audio_save_settings() {
    // Update persistent settings with current values
    g_persistent_settings.master_volume = android_audio_get_master_volume();
    g_persistent_settings.sfx_volume = android_audio_get_sfx_volume();
    g_persistent_settings.music_volume = android_audio_get_music_volume();
    g_persistent_settings.audio_enabled = android_audio_is_enabled();
    
    FILE* file = fopen(AUDIO_SETTINGS_FILE, "wb");
    if (file) {
        size_t written = fwrite(&g_persistent_settings, sizeof(PersistentAudioSettings), 1, file);
        fclose(file);
        
        if (written == 1) {
            LOGI("Audio settings saved successfully");
        } else {
            LOGE("Failed to write audio settings file");
        }
    } else {
        LOGE("Failed to open audio settings file for writing");
    }
}

// Audio settings interface functions for the game
void gltron_audio_settings_set_master_volume(float volume) {
    if (volume < 0.0f) volume = 0.0f;
    if (volume > 1.0f) volume = 1.0f;
    
    android_audio_set_master_volume(volume);
    g_persistent_settings.master_volume = volume;
    LOGI("Master volume changed to %.2f", volume);
}

void gltron_audio_settings_set_sfx_volume(float volume) {
    if (volume < 0.0f) volume = 0.0f;
    if (volume > 1.0f) volume = 1.0f;
    
    android_audio_set_sfx_volume(volume);
    g_persistent_settings.sfx_volume = volume;
    LOGI("SFX volume changed to %.2f", volume);
}

void gltron_audio_settings_set_music_volume(float volume) {
    if (volume < 0.0f) volume = 0.0f;
    if (volume > 1.0f) volume = 1.0f;
    
    android_audio_set_music_volume(volume);
    g_persistent_settings.music_volume = volume;
    LOGI("Music volume changed to %.2f", volume);
}

void gltron_audio_settings_toggle_audio() {
    int current_state = android_audio_is_enabled();
    int new_state = !current_state;
    
    android_audio_enable(new_state);
    g_persistent_settings.audio_enabled = new_state;
    LOGI("Audio %s", new_state ? "enabled" : "disabled");
}

void gltron_audio_settings_set_3d_audio(int enabled) {
    g_persistent_settings.use_3d_audio = enabled;
    LOGI("3D audio %s", enabled ? "enabled" : "disabled");
}

void gltron_audio_settings_set_quality(int quality) {
    if (quality < 0) quality = 0;
    if (quality > 2) quality = 2;
    
    g_persistent_settings.audio_quality = quality;
    
    const char* quality_names[] = {"Low", "Medium", "High"};
    LOGI("Audio quality set to %s", quality_names[quality]);
}

// Getter functions
float gltron_audio_settings_get_master_volume() {
    return g_persistent_settings.master_volume;
}

float gltron_audio_settings_get_sfx_volume() {
    return g_persistent_settings.sfx_volume;
}

float gltron_audio_settings_get_music_volume() {
    return g_persistent_settings.music_volume;
}

int gltron_audio_settings_is_audio_enabled() {
    return g_persistent_settings.audio_enabled;
}

int gltron_audio_settings_is_3d_audio_enabled() {
    return g_persistent_settings.use_3d_audio;
}

int gltron_audio_settings_get_quality() {
    return g_persistent_settings.audio_quality;
}

// Audio settings menu interface
typedef struct {
    const char* name;
    float* value;
    float min_value;
    float max_value;
    float step;
} AudioSliderSetting;

typedef struct {
    const char* name;
    int* value;
    const char** options;
    int num_options;
} AudioOptionSetting;

// Define audio settings for the menu
static AudioSliderSetting audio_sliders[] = {
    {"Master Volume", &g_persistent_settings.master_volume, 0.0f, 1.0f, 0.1f},
    {"SFX Volume", &g_persistent_settings.sfx_volume, 0.0f, 1.0f, 0.1f},
    {"Music Volume", &g_persistent_settings.music_volume, 0.0f, 1.0f, 0.1f}
};

static const char* quality_options[] = {"Low", "Medium", "High"};
static const char* enabled_options[] = {"Disabled", "Enabled"};

static AudioOptionSetting audio_options[] = {
    {"Audio", &g_persistent_settings.audio_enabled, enabled_options, 2},
    {"3D Audio", &g_persistent_settings.use_3d_audio, enabled_options, 2},
    {"Quality", &g_persistent_settings.audio_quality, quality_options, 3}
};

// Get number of audio settings
int gltron_audio_settings_get_slider_count() {
    return sizeof(audio_sliders) / sizeof(AudioSliderSetting);
}

int gltron_audio_settings_get_option_count() {
    return sizeof(audio_options) / sizeof(AudioOptionSetting);
}

// Get audio setting info
const char* gltron_audio_settings_get_slider_name(int index) {
    if (index >= 0 && index < gltron_audio_settings_get_slider_count()) {
        return audio_sliders[index].name;
    }
    return NULL;
}

const char* gltron_audio_settings_get_option_name(int index) {
    if (index >= 0 && index < gltron_audio_settings_get_option_count()) {
        return audio_options[index].name;
    }
    return NULL;
}

// Modify audio settings
void gltron_audio_settings_adjust_slider(int index, float delta) {
    if (index >= 0 && index < gltron_audio_settings_get_slider_count()) {
        AudioSliderSetting* setting = &audio_sliders[index];
        float new_value = *(setting->value) + delta * setting->step;
        
        if (new_value < setting->min_value) new_value = setting->min_value;
        if (new_value > setting->max_value) new_value = setting->max_value;
        
        *(setting->value) = new_value;
        
        // Apply the setting immediately
        if (index == 0) gltron_audio_settings_set_master_volume(new_value);
        else if (index == 1) gltron_audio_settings_set_sfx_volume(new_value);
        else if (index == 2) gltron_audio_settings_set_music_volume(new_value);
    }
}

void gltron_audio_settings_cycle_option(int index) {
    if (index >= 0 && index < gltron_audio_settings_get_option_count()) {
        AudioOptionSetting* setting = &audio_options[index];
        *(setting->value) = (*(setting->value) + 1) % setting->num_options;
        
        // Apply the setting immediately
        if (index == 0) android_audio_enable(*(setting->value));
        else if (index == 1) gltron_audio_settings_set_3d_audio(*(setting->value));
        else if (index == 2) gltron_audio_settings_set_quality(*(setting->value));
    }
}

// Get current setting values for display
float gltron_audio_settings_get_slider_value(int index) {
    if (index >= 0 && index < gltron_audio_settings_get_slider_count()) {
        return *(audio_sliders[index].value);
    }
    return 0.0f;
}

const char* gltron_audio_settings_get_option_value(int index) {
    if (index >= 0 && index < gltron_audio_settings_get_option_count()) {
        AudioOptionSetting* setting = &audio_options[index];
        int value = *(setting->value);
        if (value >= 0 && value < setting->num_options) {
            return setting->options[value];
        }
    }
    return "Unknown";
}

