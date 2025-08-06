#include <math.h>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <android/log.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LOG_TAG "GLTron_Audio"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// Audio engine state
typedef struct {
    SLObjectItf engine_object;
    SLEngineItf engine_interface;
    SLObjectItf output_mix_object;
    SLObjectItf player_object;
    SLPlayItf player_interface;
    SLVolumeItf volume_interface;
    int initialized;
} AudioEngine;

static AudioEngine g_audio_engine = {0};

// Audio settings
typedef struct {
    float master_volume;
    float sfx_volume;
    float music_volume;
    int audio_enabled;
} AudioSettings;

static AudioSettings g_audio_settings = {
    .master_volume = 1.0f,
    .sfx_volume = 1.0f,
    .music_volume = 0.7f,
    .audio_enabled = 1
};

// Audio initialization
int android_audio_init() {
    SLresult result;
    
    // Create engine
    result = slCreateEngine(&g_audio_engine.engine_object, 0, NULL, 0, NULL, NULL);
    if (result != SL_RESULT_SUCCESS) {
        LOGE("Failed to create audio engine");
        return 0;
    }
    
    // Realize engine
    result = (*g_audio_engine.engine_object)->Realize(g_audio_engine.engine_object, SL_BOOLEAN_FALSE);
    if (result != SL_RESULT_SUCCESS) {
        LOGE("Failed to realize audio engine");
        return 0;
    }
    
    // Get engine interface
    result = (*g_audio_engine.engine_object)->GetInterface(g_audio_engine.engine_object, 
                                                          SL_IID_ENGINE, 
                                                          &g_audio_engine.engine_interface);
    if (result != SL_RESULT_SUCCESS) {
        LOGE("Failed to get engine interface");
        return 0;
    }
    
    // Create output mix
    result = (*g_audio_engine.engine_interface)->CreateOutputMix(g_audio_engine.engine_interface,
                                                                &g_audio_engine.output_mix_object,
                                                                0, NULL, NULL);
    if (result != SL_RESULT_SUCCESS) {
        LOGE("Failed to create output mix");
        return 0;
    }
    
    // Realize output mix
    result = (*g_audio_engine.output_mix_object)->Realize(g_audio_engine.output_mix_object, SL_BOOLEAN_FALSE);
    if (result != SL_RESULT_SUCCESS) {
        LOGE("Failed to realize output mix");
        return 0;
    }
    
    g_audio_engine.initialized = 1;
    LOGI("Android audio adapter initialized successfully");
    return 1;
}

void android_audio_cleanup() {
    if (g_audio_engine.player_object) {
        (*g_audio_engine.player_object)->Destroy(g_audio_engine.player_object);
        g_audio_engine.player_object = NULL;
    }
    
    if (g_audio_engine.output_mix_object) {
        (*g_audio_engine.output_mix_object)->Destroy(g_audio_engine.output_mix_object);
        g_audio_engine.output_mix_object = NULL;
    }
    
    if (g_audio_engine.engine_object) {
        (*g_audio_engine.engine_object)->Destroy(g_audio_engine.engine_object);
        g_audio_engine.engine_object = NULL;
    }
    
    g_audio_engine.initialized = 0;
    LOGI("Android audio adapter cleaned up");
}

// Audio settings functions
void android_audio_set_master_volume(float volume) {
    g_audio_settings.master_volume = volume;
    if (volume <= 0.0f) {
        volume = 0.001f; // Avoid complete silence which might cause issues
    }
    
    if (g_audio_engine.volume_interface) {
        SLmillibel mb_volume = (SLmillibel)(2000.0f * log10f(volume));
        (*g_audio_engine.volume_interface)->SetVolumeLevel(g_audio_engine.volume_interface, mb_volume);
    }
    
    LOGI("Master volume set to %.2f", volume);
}

void android_audio_set_sfx_volume(float volume) {
    g_audio_settings.sfx_volume = volume;
    LOGI("SFX volume set to %.2f", volume);
}

void android_audio_set_music_volume(float volume) {
    g_audio_settings.music_volume = volume;
    LOGI("Music volume set to %.2f", volume);
}

float android_audio_get_master_volume() {
    return g_audio_settings.master_volume;
}

float android_audio_get_sfx_volume() {
    return g_audio_settings.sfx_volume;
}

float android_audio_get_music_volume() {
    return g_audio_settings.music_volume;
}

void android_audio_enable(int enabled) {
    g_audio_settings.audio_enabled = enabled;
    LOGI("Audio %s", enabled ? "enabled" : "disabled");
}

int android_audio_is_enabled() {
    return g_audio_settings.audio_enabled;
}

// Sound effect functions
void android_audio_play_sound_effect(int sound_id) {
    if (!g_audio_settings.audio_enabled || !g_audio_engine.initialized) {
        return;
    }
    
    // This would play specific sound effects based on sound_id
    // For now, just log the request
    LOGI("Playing sound effect %d", sound_id);
    
    // Implementation would involve:
    // 1. Loading the appropriate sound file
    // 2. Creating an audio player for the sound
    // 3. Playing the sound with appropriate volume
}

void android_audio_play_engine_sound() {
    android_audio_play_sound_effect(1); // Engine sound
}

void android_audio_play_crash_sound() {
    android_audio_play_sound_effect(2); // Crash sound
}

void android_audio_play_turn_sound() {
    android_audio_play_sound_effect(3); // Turn sound
}

void android_audio_play_boost_sound() {
    android_audio_play_sound_effect(4); // Boost sound
}

// Music functions
void android_audio_play_music(const char* music_file) {
    if (!g_audio_settings.audio_enabled || !g_audio_engine.initialized) {
        return;
    }
    
    LOGI("Playing music: %s", music_file ? music_file : "default");
    
    // Implementation would involve:
    // 1. Loading the music file
    // 2. Creating a music player
    // 3. Setting up looping
    // 4. Starting playback
}

void android_audio_stop_music() {
    if (g_audio_engine.player_interface) {
        (*g_audio_engine.player_interface)->SetPlayState(g_audio_engine.player_interface, SL_PLAYSTATE_STOPPED);
    }
    LOGI("Music stopped");
}

void android_audio_pause_music() {
    if (g_audio_engine.player_interface) {
        (*g_audio_engine.player_interface)->SetPlayState(g_audio_engine.player_interface, SL_PLAYSTATE_PAUSED);
    }
    LOGI("Music paused");
}

void android_audio_resume_music() {
    if (g_audio_engine.player_interface) {
        (*g_audio_engine.player_interface)->SetPlayState(g_audio_engine.player_interface, SL_PLAYSTATE_PLAYING);
    }
    LOGI("Music resumed");
}

// Integration with GLTron audio system
void android_audio_update_gltron_audio() {
    // This function would integrate with the original GLTron audio system
    // by providing the Android-specific audio implementation
    
    // Example integration (would need to match GLTron's audio interface):
    /*
    gltron_audio_set_volume(g_audio_settings.master_volume);
    gltron_audio_set_sfx_volume(g_audio_settings.sfx_volume);
    gltron_audio_set_music_volume(g_audio_settings.music_volume);
    */
}

