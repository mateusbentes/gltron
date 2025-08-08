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

// FIXED: Add OpenSL ES error checking macro
#define CHECK_SLES_ERROR(result, operation) \
    do { \
        if ((result) != SL_RESULT_SUCCESS) { \
            LOGE("OpenSL ES error in %s: 0x%x", (operation), (result)); \
            return 0; \
        } \
    } while(0)

// Audio engine state
typedef struct {
    SLObjectItf engine_object;
    SLEngineItf engine_interface;
    SLObjectItf output_mix_object;
    SLObjectItf player_object;
    SLPlayItf player_interface;
    SLVolumeItf volume_interface;
    SLBufferQueueItf buffer_queue_interface; // FIXED: Add buffer queue interface
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

// FIXED: Add audio buffer management
#define AUDIO_BUFFER_SIZE 4096
#define AUDIO_BUFFER_COUNT 2

typedef struct {
    short* buffers[AUDIO_BUFFER_COUNT];
    int current_buffer;
    int buffer_size;
} AudioBufferManager;

static AudioBufferManager g_buffer_manager = {0};

// FIXED: Add volume conversion utility
static SLmillibel volume_to_millibel(float volume) {
    if (volume <= 0.0f) {
        return SL_MILLIBEL_MIN;
    }
    if (volume >= 1.0f) {
        return 0; // 0 dB (maximum)
    }
    // Convert linear volume to decibels
    return (SLmillibel)(2000.0f * log10f(volume));
}

// FIXED: Initialize audio buffers
static int init_audio_buffers() {
    g_buffer_manager.buffer_size = AUDIO_BUFFER_SIZE;
    g_buffer_manager.current_buffer = 0;
    
    for (int i = 0; i < AUDIO_BUFFER_COUNT; i++) {
        g_buffer_manager.buffers[i] = (short*)malloc(AUDIO_BUFFER_SIZE * sizeof(short));
        if (!g_buffer_manager.buffers[i]) {
            LOGE("Failed to allocate audio buffer %d", i);
            // Clean up previously allocated buffers
            for (int j = 0; j < i; j++) {
                free(g_buffer_manager.buffers[j]);
                g_buffer_manager.buffers[j] = NULL;
            }
            return 0;
        }
        // Initialize buffer with silence
        memset(g_buffer_manager.buffers[i], 0, AUDIO_BUFFER_SIZE * sizeof(short));
    }
    
    LOGI("Audio buffers initialized successfully");
    return 1;
}

// FIXED: Cleanup audio buffers
static void cleanup_audio_buffers() {
    for (int i = 0; i < AUDIO_BUFFER_COUNT; i++) {
        if (g_buffer_manager.buffers[i]) {
            free(g_buffer_manager.buffers[i]);
            g_buffer_manager.buffers[i] = NULL;
        }
    }
    g_buffer_manager.current_buffer = 0;
    LOGI("Audio buffers cleaned up");
}

// Audio initialization
int android_audio_init() {
    if (g_audio_engine.initialized) {
        LOGI("Audio engine already initialized");
        return 1;
    }
    
    SLresult result;
    
    // FIXED: Initialize audio buffers first
    if (!init_audio_buffers()) {
        LOGE("Failed to initialize audio buffers");
        return 0;
    }
    
    // Create engine
    result = slCreateEngine(&g_audio_engine.engine_object, 0, NULL, 0, NULL, NULL);
    CHECK_SLES_ERROR(result, "slCreateEngine");
    
    // FIXED: Check if engine object was created
    if (!g_audio_engine.engine_object) {
        LOGE("Engine object is NULL after creation");
        cleanup_audio_buffers();
        return 0;
    }
    
    // Realize engine
    result = (*g_audio_engine.engine_object)->Realize(g_audio_engine.engine_object, SL_BOOLEAN_FALSE);
    CHECK_SLES_ERROR(result, "engine Realize");
    
    // Get engine interface
    result = (*g_audio_engine.engine_object)->GetInterface(g_audio_engine.engine_object, 
                                                          SL_IID_ENGINE, 
                                                          &g_audio_engine.engine_interface);
    CHECK_SLES_ERROR(result, "get engine interface");
    
    // FIXED: Validate engine interface
    if (!g_audio_engine.engine_interface) {
        LOGE("Engine interface is NULL");
        android_audio_cleanup();
        return 0;
    }
    
    // Create output mix
    result = (*g_audio_engine.engine_interface)->CreateOutputMix(g_audio_engine.engine_interface,
                                                                &g_audio_engine.output_mix_object,
                                                                0, NULL, NULL);
    CHECK_SLES_ERROR(result, "CreateOutputMix");
    
    // FIXED: Check if output mix object was created
    if (!g_audio_engine.output_mix_object) {
        LOGE("Output mix object is NULL after creation");
        android_audio_cleanup();
        return 0;
    }
    
    // Realize output mix
    result = (*g_audio_engine.output_mix_object)->Realize(g_audio_engine.output_mix_object, SL_BOOLEAN_FALSE);
    CHECK_SLES_ERROR(result, "output mix Realize");
    
    // FIXED: Create audio player for sound effects
    SLDataLocator_AndroidSimpleBufferQueue buffer_queue_locator = {
        SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 
        AUDIO_BUFFER_COUNT
    };
    
    SLDataFormat_PCM pcm_format = {
        SL_DATAFORMAT_PCM,
        1, // mono
        SL_SAMPLINGRATE_44_1,
        SL_PCMSAMPLEFORMAT_FIXED_16,
        SL_PCMSAMPLEFORMAT_FIXED_16,
        SL_SPEAKER_FRONT_CENTER,
        SL_BYTEORDER_LITTLEENDIAN
    };
    
    SLDataSource audio_source = {&buffer_queue_locator, &pcm_format};
    
    SLDataLocator_OutputMix output_mix_locator = {SL_DATALOCATOR_OUTPUTMIX, g_audio_engine.output_mix_object};
    SLDataSink audio_sink = {&output_mix_locator, NULL};
    
    const SLInterfaceID interface_ids[] = {SL_IID_BUFFERQUEUE, SL_IID_VOLUME};
    const SLboolean interface_required[] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};
    
    result = (*g_audio_engine.engine_interface)->CreateAudioPlayer(
        g_audio_engine.engine_interface,
        &g_audio_engine.player_object,
        &audio_source,
        &audio_sink,
        2, // number of interfaces
        interface_ids,
        interface_required
    );
    CHECK_SLES_ERROR(result, "CreateAudioPlayer");
    
    // FIXED: Check if player object was created
    if (!g_audio_engine.player_object) {
        LOGE("Player object is NULL after creation");
        android_audio_cleanup();
        return 0;
    }
    
    // Realize audio player
    result = (*g_audio_engine.player_object)->Realize(g_audio_engine.player_object, SL_BOOLEAN_FALSE);
    CHECK_SLES_ERROR(result, "player Realize");
    
    // Get player interface
    result = (*g_audio_engine.player_object)->GetInterface(g_audio_engine.player_object,
                                                          SL_IID_PLAY,
                                                          &g_audio_engine.player_interface);
    CHECK_SLES_ERROR(result, "get player interface");
    
    // FIXED: Get volume interface (this was missing in original code)
    result = (*g_audio_engine.player_object)->GetInterface(g_audio_engine.player_object,
                                                          SL_IID_VOLUME,
                                                          &g_audio_engine.volume_interface);
    CHECK_SLES_ERROR(result, "get volume interface");
    
    // FIXED: Get buffer queue interface
    result = (*g_audio_engine.player_object)->GetInterface(g_audio_engine.player_object,
                                                          SL_IID_BUFFERQUEUE,
                                                          &g_audio_engine.buffer_queue_interface);
    CHECK_SLES_ERROR(result, "get buffer queue interface");
    
    // FIXED: Validate all interfaces
    if (!g_audio_engine.player_interface) {
        LOGE("Player interface is NULL");
        android_audio_cleanup();
        return 0;
    }
    
    if (!g_audio_engine.volume_interface) {
        LOGE("Volume interface is NULL");
        android_audio_cleanup();
        return 0;
    }
    
    if (!g_audio_engine.buffer_queue_interface) {
        LOGE("Buffer queue interface is NULL");
        android_audio_cleanup();
        return 0;
    }
    
    g_audio_engine.initialized = 1;
    LOGI("Android audio adapter initialized successfully");
    return 1;
}

void android_audio_cleanup() {
    if (!g_audio_engine.initialized) {
        return;
    }
    
    // FIXED: Stop playback before cleanup
    if (g_audio_engine.player_interface) {
        (*g_audio_engine.player_interface)->SetPlayState(g_audio_engine.player_interface, SL_PLAYSTATE_STOPPED);
    }
    
    // FIXED: Clear buffer queue
    if (g_audio_engine.buffer_queue_interface) {
        (*g_audio_engine.buffer_queue_interface)->Clear(g_audio_engine.buffer_queue_interface);
    }
    
    if (g_audio_engine.player_object) {
        (*g_audio_engine.player_object)->Destroy(g_audio_engine.player_object);
        g_audio_engine.player_object = NULL;
        g_audio_engine.player_interface = NULL;
        g_audio_engine.volume_interface = NULL;
        g_audio_engine.buffer_queue_interface = NULL;
    }
    
    if (g_audio_engine.output_mix_object) {
        (*g_audio_engine.output_mix_object)->Destroy(g_audio_engine.output_mix_object);
        g_audio_engine.output_mix_object = NULL;
    }
    
    if (g_audio_engine.engine_object) {
        (*g_audio_engine.engine_object)->Destroy(g_audio_engine.engine_object);
        g_audio_engine.engine_object = NULL;
        g_audio_engine.engine_interface = NULL;
    }
    
    cleanup_audio_buffers();
    g_audio_engine.initialized = 0;
    LOGI("Android audio adapter cleaned up");
}

// Audio settings functions
void android_audio_set_master_volume(float volume) {
    // FIXED: Clamp volume to valid range
    if (volume < 0.0f) volume = 0.0f;
    if (volume > 1.0f) volume = 1.0f;
    
    g_audio_settings.master_volume = volume;
    
    if (g_audio_engine.volume_interface) {
        SLmillibel mb_volume = volume_to_millibel(volume);
        SLresult result = (*g_audio_engine.volume_interface)->SetVolumeLevel(g_audio_engine.volume_interface, mb_volume);
        if (result != SL_RESULT_SUCCESS) {
            LOGE("Failed to set volume level: 0x%x", result);
        }
    }
    
    LOGI("Master volume set to %.2f", volume);
}

void android_audio_set_sfx_volume(float volume) {
    // FIXED: Clamp volume to valid range
    if (volume < 0.0f) volume = 0.0f;
    if (volume > 1.0f) volume = 1.0f;
    
    g_audio_settings.sfx_volume = volume;
    LOGI("SFX volume set to %.2f", volume);
}

void android_audio_set_music_volume(float volume) {
    // FIXED: Clamp volume to valid range
    if (volume < 0.0f) volume = 0.0f;
    if (volume > 1.0f) volume = 1.0f;
    
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
    g_audio_settings.audio_enabled = enabled ? 1 : 0;
    
    // FIXED: Actually mute/unmute the audio when disabled/enabled
    if (g_audio_engine.volume_interface) {
        if (enabled) {
            android_audio_set_master_volume(g_audio_settings.master_volume);
        } else {
            SLresult result = (*g_audio_engine.volume_interface)->SetVolumeLevel(g_audio_engine.volume_interface, SL_MILLIBEL_MIN);
            if (result != SL_RESULT_SUCCESS) {
                LOGE("Failed to mute audio: 0x%x", result);
            }
        }
    }
    
    LOGI("Audio %s", enabled ? "enabled" : "disabled");
}

int android_audio_is_enabled() {
    return g_audio_settings.audio_enabled;
}

// FIXED: Add buffer queue callback
static void buffer_queue_callback(SLAndroidSimpleBufferQueueItf bq, void *context) {
    // This callback is called when a buffer has finished playing
    // In a real implementation, you would queue the next buffer here
    LOGI("Buffer queue callback triggered");
}

// Sound effect functions
void android_audio_play_sound_effect(int sound_id) {
    if (!g_audio_settings.audio_enabled || !g_audio_engine.initialized) {
        return;
    }
    
    // FIXED: Validate sound_id
    if (sound_id < 0 || sound_id > 10) {
        LOGE("Invalid sound_id: %d", sound_id);
        return;
    }
    
    if (!g_audio_engine.buffer_queue_interface || !g_audio_engine.player_interface) {
        LOGE("Audio interfaces not available");
        return;
    }
    
    // FIXED: Generate a simple test tone based on sound_id
    short* buffer = g_buffer_manager.buffers[g_buffer_manager.current_buffer];
    int buffer_size = g_buffer_manager.buffer_size;
    
    // Generate different tones for different sound effects
    float frequency = 440.0f + (sound_id * 110.0f); // Different frequencies for different sounds
    float amplitude = 16000.0f * g_audio_settings.sfx_volume * g_audio_settings.master_volume;
    
    for (int i = 0; i < buffer_size; i++) {
        float sample = sinf(2.0f * M_PI * frequency * i / 44100.0f) * amplitude;
        buffer[i] = (short)sample;
    }
    
    // Queue the buffer
    SLresult result = (*g_audio_engine.buffer_queue_interface)->Enqueue(
        g_audio_engine.buffer_queue_interface,
        buffer,
        buffer_size * sizeof(short)
    );
    
    if (result != SL_RESULT_SUCCESS) {
        LOGE("Failed to enqueue audio buffer: 0x%x", result);
        return;
    }
    
    // Start playback if not already playing
    SLuint32 play_state;
    result = (*g_audio_engine.player_interface)->GetPlayState(g_audio_engine.player_interface, &play_state);
    if (result == SL_RESULT_SUCCESS && play_state != SL_PLAYSTATE_PLAYING) {
        result = (*g_audio_engine.player_interface)->SetPlayState(g_audio_engine.player_interface, SL_PLAYSTATE_PLAYING);
        if (result != SL_RESULT_SUCCESS) {
            LOGE("Failed to start playback: 0x%x", result);
        }
    }
    
    // Move to next buffer
    g_buffer_manager.current_buffer = (g_buffer_manager.current_buffer + 1) % AUDIO_BUFFER_COUNT;
    
    LOGI("Playing sound effect %d (freq: %.1f Hz)", sound_id, frequency);
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
    
    // FIXED: Validate music_file parameter
    if (!music_file) {
        LOGE("Music file path is NULL");
        return;
    }
    
    LOGI("Playing music: %s", music_file);
    
    // Implementation would involve:
    // 1. Loading the music file (MP3, OGG, etc.)
    // 2. Creating a separate music player
    // 3. Setting up looping
    // 4. Starting playback
    // For now, just log the request
}

void android_audio_stop_music() {
    if (g_audio_engine.player_interface) {
        SLresult result = (*g_audio_engine.player_interface)->SetPlayState(g_audio_engine.player_interface, SL_PLAYSTATE_STOPPED);
        if (result != SL_RESULT_SUCCESS) {
            LOGE("Failed to stop music: 0x%x", result);
        }
    }
    LOGI("Music stopped");
}

void android_audio_pause_music() {
    if (g_audio_engine.player_interface) {
        SLresult result = (*g_audio_engine.player_interface)->SetPlayState(g_audio_engine.player_interface, SL_PLAYSTATE_PAUSED);
        if (result != SL_RESULT_SUCCESS) {
            LOGE("Failed to pause music: 0x%x", result);
        }
    }
    LOGI("Music paused");
}

void android_audio_resume_music() {
    if (g_audio_engine.player_interface) {
        SLresult result = (*g_audio_engine.player_interface)->SetPlayState(g_audio_engine.player_interface, SL_PLAYSTATE_PLAYING);
        if (result != SL_RESULT_SUCCESS) {
            LOGE("Failed to resume music: 0x%x", result);
        }
    }
    LOGI("Music resumed");
}

// Integration with GLTron audio system
void android_audio_update_gltron_audio() {
    // This function would integrate with the original GLTron audio system
    // by providing the Android-specific audio implementation
    
    if (!g_audio_engine.initialized) {
        return;
    }
    
    // Example integration (would need to match GLTron's audio interface):
    #ifdef GLTRON_AUDIO_AVAILABLE
    /*
    gltron_audio_set_volume(g_audio_settings.master_volume);
    gltron_audio_set_sfx_volume(g_audio_settings.sfx_volume);
    gltron_audio_set_music_volume(g_audio_settings.music_volume);
    gltron_audio_set_enabled(g_audio_settings.audio_enabled);
    */
    #endif
    
    LOGI("GLTron audio system updated");
}

// FIXED: Add function to check if audio is initialized
int android_audio_is_initialized() {
    return g_audio_engine.initialized;
}

// FIXED: Add function to get audio engine status
void android_audio_get_status(int* initialized, int* playing, float* current_volume) {
    if (initialized) *initialized = g_audio_engine.initialized;
    
    if (playing && g_audio_engine.player_interface) {
        SLuint32 play_state;
        SLresult result = (*g_audio_engine.player_interface)->GetPlayState(g_audio_engine.player_interface, &play_state);
        *playing = (result == SL_RESULT_SUCCESS && play_state == SL_PLAYSTATE_PLAYING) ? 1 : 0;
    } else if (playing) {
        *playing = 0;
    }
    
    if (current_volume) *current_volume = g_audio_settings.master_volume;
}
