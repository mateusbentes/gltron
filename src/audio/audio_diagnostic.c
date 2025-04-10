#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <SDL2/SDL.h>

// Forward declarations of audio functions
extern void Audio_Init(void);
extern void Audio_LoadSample(char *name, int number);
extern void Audio_LoadPlayers(void);
extern void Audio_Start(void);
extern void Audio_EnableEngine(void);

// Check if a file exists
int file_exists(const char *path) {
    struct stat buffer;
    return (stat(path, &buffer) == 0);
}

// List files in a directory
void list_directory(const char *path) {
    DIR *dir;
    struct dirent *entry;
    
    printf("Checking directory: %s\n", path);
    
    if ((dir = opendir(path)) == NULL) {
        printf("  [ERROR] Cannot open directory: %s\n", path);
        return;
    }
    
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] != '.') { // Skip hidden files and . and ..
            char full_path[1024];
            snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
            
            struct stat st;
            if (stat(full_path, &st) == 0) {
                if (S_ISDIR(st.st_mode)) {
                    printf("  [DIR] %s\n", entry->d_name);
                } else {
                    printf("  [FILE] %s (%ld bytes)\n", entry->d_name, (long)st.st_size);
                }
            } else {
                printf("  [???] %s (cannot stat)\n", entry->d_name);
            }
        }
    }
    
    closedir(dir);
}

// Check SDL audio subsystem
void check_sdl_audio() {
    printf("\n=== SDL Audio Subsystem ===\n");
    
    if (SDL_WasInit(SDL_INIT_AUDIO)) {
        printf("SDL Audio is initialized\n");
        
        // Get current audio driver
        const char *driver = SDL_GetCurrentAudioDriver();
        printf("Current audio driver: %s\n", driver ? driver : "None");
        
        // Get audio device info
        int num_devices = SDL_GetNumAudioDevices(0);
        printf("Number of audio playback devices: %d\n", num_devices);
        
        for (int i = 0; i < num_devices; i++) {
            printf("  Device %d: %s\n", i, SDL_GetAudioDeviceName(i, 0));
        }
        
        // Check if audio is opened
        SDL_AudioStatus status = SDL_GetAudioStatus();
        printf("Audio status: ");
        switch (status) {
            case SDL_AUDIO_STOPPED: printf("Stopped\n"); break;
            case SDL_AUDIO_PLAYING: printf("Playing\n"); break;
            case SDL_AUDIO_PAUSED: printf("Paused\n"); break;
            default: printf("Unknown (%d)\n", status); break;
        }
    } else {
        printf("SDL Audio is NOT initialized\n");
        
        // Try to initialize audio
        printf("Attempting to initialize SDL audio subsystem...\n");
        if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) {
            printf("  [ERROR] Failed to initialize SDL audio: %s\n", SDL_GetError());
        } else {
            printf("  Successfully initialized SDL audio\n");
            SDL_QuitSubSystem(SDL_INIT_AUDIO);
        }
    }
}

// Test playing a sound file directly with SDL
void test_play_sound(const char *filename) {
    printf("\n=== Testing Sound Playback: %s ===\n", filename);
    
    if (!file_exists(filename)) {
        printf("  [ERROR] File does not exist: %s\n", filename);
        return;
    }
    
    // Initialize SDL Audio
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) {
        printf("  [ERROR] Failed to initialize SDL audio: %s\n", SDL_GetError());
        return;
    }
    
    // Load WAV file
    SDL_AudioSpec wav_spec;
    Uint32 wav_length;
    Uint8 *wav_buffer;
    
    if (SDL_LoadWAV(filename, &wav_spec, &wav_buffer, &wav_length) == NULL) {
        printf("  [ERROR] Failed to load WAV file: %s\n", SDL_GetError());
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        return;
    }
    
    // Print audio specs
    printf("  Audio format: %d\n", wav_spec.format);
    printf("  Channels: %d\n", wav_spec.channels);
    printf("  Frequency: %d Hz\n", wav_spec.freq);
    printf("  Sample size: %d bytes\n", wav_length);
    
    // Open audio device
    SDL_AudioDeviceID audio_device;
    audio_device = SDL_OpenAudioDevice(NULL, 0, &wav_spec, NULL, 0);
    if (audio_device == 0) {
        printf("  [ERROR] Failed to open audio device: %s\n", SDL_GetError());
        SDL_FreeWAV(wav_buffer);
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        return;
    }
    
    // Play audio
    printf("  Playing sound...\n");
    SDL_PauseAudioDevice(audio_device, 0);
    if (SDL_QueueAudio(audio_device, wav_buffer, wav_length) != 0) {
        printf("  [ERROR] Failed to queue audio: %s\n", SDL_GetError());
    }
    
    // Wait for audio to finish
    SDL_Delay(1000);  // Wait 1 second
    
    // Clean up
    SDL_CloseAudioDevice(audio_device);
    SDL_FreeWAV(wav_buffer);
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
    
    printf("  Sound test complete\n");
}

// Main diagnostic function
void run_audio_diagnostic() {
    char cwd[1024];
    
    printf("\n=== GLtron Audio Diagnostic ===\n");
    
    // Get current working directory
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("Current working directory: %s\n", cwd);
    } else {
        printf("Failed to get current working directory\n");
    }
    
    // Check sound files
    printf("\n=== Sound Files ===\n");
    list_directory("sounds");
    
    // Check specific sound files
    printf("\n=== Critical Sound Files ===\n");
    const char *critical_sounds[] = {
        "sounds/engine.wav",
        "sounds/crash.wav",
        "sounds/recognizer.wav"
    };
    
    const char *alt_sounds[] = {
        "sounds/game_engine.wav",
        "sounds/game_crash.wav",
        "sounds/game_recognizer.wav"
    };
    
    for (int i = 0; i < sizeof(critical_sounds)/sizeof(critical_sounds[0]); i++) {
        int found = file_exists(critical_sounds[i]);
        int alt_found = file_exists(alt_sounds[i]);
        
        printf("%s: %s", critical_sounds[i], found ? "Found" : "MISSING");
        if (!found && alt_found) {
            printf(" (but alternative exists: %s)", alt_sounds[i]);
        }
        printf("\n");
        
        // Try to play the sound file if it exists
        if (found) {
            test_play_sound(critical_sounds[i]);
        } else if (alt_found) {
            test_play_sound(alt_sounds[i]);
        }
    }
    
    // Check SDL audio
    check_sdl_audio();
    
    printf("\n=== Audio Diagnostic Complete ===\n");
}
