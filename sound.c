#include "sound.h"
#include "globals.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>  // for free() used with getFullPath

// Original MikMod implementation (desktop)
#include <mikmod.h>

// Global sound module and sound effect variables
MODULE* sound_module = NULL;
SAMPLE* crash_sfx = NULL;
SAMPLE* lose_sfx = NULL;
SAMPLE* win_sfx = NULL;
SAMPLE* highlight_sfx = NULL;
SAMPLE* engine_sfx = NULL;
SAMPLE* start_sfx = NULL;
SAMPLE* action_sfx = NULL;

static int sound_effects_loaded = 0;

// helper to load music module by common names/paths
static int loadMusicModule(void) {
    if (sound_module) return 0; // already loaded
    const char* paths[] = {"./", "/usr/share/games/gltron/", "/usr/local/share/games/gltron/", NULL};
    const char* names[] = {"gltron", "music", NULL};
    const char* exts[] = {".it", ".xm", ".s3m", ".mod", NULL};
    char full[512];
    for (int i=0; paths[i]; ++i) {
        for (int n=0; names[n]; ++n) {
            for (int e=0; exts[e]; ++e) {
                snprintf(full, sizeof(full), "%s%s%s", paths[i], names[n], exts[e]);
                printf("Attempting to load music from: %s\n", full);
                sound_module = Player_Load(full, 64, 0);
                if (sound_module) {
                    printf("Successfully loaded music: %s\n", full);
                    return 0;
                }
            }
        }
    }
    printf("Could not load music module: %s\n", MikMod_strerror(MikMod_errno));
    return 1;
}

// Initialize sound system
int initSound(void) {
    printf("=== Initializing sound system ===\n");
    
    // Set sound mode and frequency
    md_mode |= DMODE_SOFT_MUSIC | DMODE_SOFT_SNDFX;
    md_mixfreq = 44100;

    // Register appropriate drivers based on platform
#ifdef WIN32
    MikMod_RegisterDriver(&drv_win);
#else
    MikMod_RegisterAllDrivers();
#endif

    printf("MikMod driver info: %s\n", MikMod_InfoDriver());

    // Set sound device from game settings
    md_device = game->settings->sound_driver;
    printf("Using sound device: %d\n", md_device);

    // Register all sound file loaders
    MikMod_RegisterAllLoaders();

    // Initialize MikMod
    if (MikMod_Init("")) {
        printf("ERROR: Could not initialize sound: %s\n", MikMod_strerror(MikMod_errno));
        printf("Sound system will be disabled.\n");
        game->settings->playSound = 0;
        return 1;
    }
    
    printf("MikMod initialized successfully.\n");

    // Load sound effects if sound is enabled
    if (game->settings->playSound && !sound_effects_loaded) {
        printf("Loading sound effects...\n");
        
        if (loadSampleEffect("game_crash", &crash_sfx)) {
            printf("Warning: Could not load crash sample\n");
        } else {
            printf("Loaded: game_crash.wav\n");
        }
        
        if (loadSampleEffect("game_lose", &lose_sfx)) {
            printf("Warning: Could not load lose sample\n");
        } else {
            printf("Loaded: game_lose.wav\n");
        }
        
        if (loadSampleEffect("game_win", &win_sfx)) {
            printf("Warning: Could not load win sample\n");
        } else {
            printf("Loaded: game_win.wav\n");
        }
        
        if (loadSampleEffect("menu_highlight", &highlight_sfx)) {
            printf("Warning: Could not load highlight sample\n");
        } else {
            printf("Loaded: menu_highlight.wav\n");
        }
        
        if (loadSampleEffect("game_engine", &engine_sfx)) {
            printf("Warning: Could not load engine sample\n");
        } else {
            printf("Loaded: game_engine.wav\n");
        }
        
        if (loadSampleEffect("game_start", &start_sfx)) {
            printf("Warning: Could not load start sample\n");
        } else {
            printf("Loaded: game_start.wav\n");
        }
        
        if (loadSampleEffect("menu_action", &action_sfx)) {
            printf("Warning: Could not load action sample\n");
        } else {
            printf("Loaded: menu_action.wav\n");
        }
        
        sound_effects_loaded = 1;
        printf("Sound effects loading complete.\n");
    }

    // Try to load music and auto-play if enabled
    loadMusicModule();
    if (game->settings->playSound && game->settings->playMusic && sound_module) {
        Player_Start(sound_module);
        printf("music started\n");
    }

    return 0;
}

// Load a sample (SFX) from file
int loadSampleEffect(char* name, SAMPLE** sfx_out) {
    *sfx_out = NULL;
    
    // First, check if 'name' is already a full path (contains '/' or ends with .wav)
    if (strchr(name, '/') != NULL || strstr(name, ".wav") != NULL) {
        // It's already a full path, try to load directly
        printf("  Trying full path: %s\n", name);
        
        // Check if file exists first
        FILE* test = fopen(name, "rb");
        if (test) {
            fclose(test);
            SAMPLE* s = Sample_Load(name);
            if (s) {
                *sfx_out = s;
                printf("  ✓ Loaded: %s\n", name);
                return 0;
            }
            printf("  ✗ MikMod failed to load %s: %s\n", name, MikMod_strerror(MikMod_errno));
        } else {
            printf("  ✗ File not found: %s\n", name);
        }
        return 1;
    }
    
    // Otherwise, search for the file in standard paths
    const char* paths[] = {
        "./",
        "/usr/share/games/gltron/",
        "/usr/local/share/games/gltron/",
        NULL
    };
    // Prefer wav for SFX; allow a few other common sample formats
    const char* exts[] = {".wav", ".aiff", ".aif", ".au", NULL};
    char full[512];
    
    for (int i=0; paths[i]; ++i) {
        for (int e=0; exts[e]; ++e) {
            snprintf(full, sizeof(full), "%s%s%s", paths[i], name, exts[e]);
            
            // Check if file exists first
            FILE* test = fopen(full, "rb");
            if (test) {
                fclose(test);
                printf("  Found file: %s\n", full);
                SAMPLE* s = Sample_Load(full);
                if (s) {
                    *sfx_out = s;
                    printf("  ✓ Loaded: %s\n", full);
                    return 0;
                }
                printf("  ✗ MikMod failed: %s\n", MikMod_strerror(MikMod_errno));
            }
        }
    }
    printf("  ✗ Could not find %s in any search path\n", name);
    return 1;
}

// Play a sample (SFX)
int playSampleEffect(SAMPLE* sfx) {
    if (!sfx) {
        if (game->settings->playSound)
            printf("Warning: SFX not loaded for this event (NULL sample)\n");
        return 1;
    }
    if (game->settings->playSound) {
        int voice = Sample_Play(sfx, 0, 0);
        if (voice >= 0) return 0;
    }
    return 1;
}

// Load a sound module (music only - sound effects are loaded in initSound)
int loadSound(char *name) {
    // Desktop implementation
    if (sound_module) {
        Player_Free(sound_module);
        sound_module = NULL;
    }

    sound_module = Player_Load(name, 64, 0);
    if (!sound_module) {
        printf("Could not load module %s: %s\n", name, MikMod_strerror(MikMod_errno));
        return 1;
    }

    // Sound effects are now loaded in initSound() only
    return 0;
}

// Play the current sound module
int playSound(void) {
    if (!sound_module) {
        loadMusicModule();
    }
    if (sound_module && game->settings->playSound && game->settings->playMusic) {
        Player_Start(sound_module);
        printf("sound started\n");
        return 0;
    }
    return 1;
}

// Stop the current sound module
int stopSound(void) {
#ifdef ANDROID
    sb_stop_music();
    printf("sound stopped (OpenMPT)\n");
    return 0;
#else
    if (Player_Active()) {
        Player_Stop();
        printf("sound stopped\n");
        return 0;
    }
    return 1;
#endif
}

// Clean up sound system
void deleteSound(void) {
    // Stop any playing sound
    if (Player_Active())
        Player_Stop();

    // Free the main sound module
    if (sound_module) {
        Player_Free(sound_module);
        sound_module = NULL;
    }

    // Free sound effects
    if (crash_sfx) { Sample_Free(crash_sfx); crash_sfx = NULL; }
    if (lose_sfx) { Sample_Free(lose_sfx); lose_sfx = NULL; }
    if (win_sfx) { Sample_Free(win_sfx); win_sfx = NULL; }
    if (highlight_sfx) { Sample_Free(highlight_sfx); highlight_sfx = NULL; }
    if (engine_sfx) { Sample_Free(engine_sfx); engine_sfx = NULL; }
    if (start_sfx) { Sample_Free(start_sfx); start_sfx = NULL; }
    if (action_sfx) { Sample_Free(action_sfx); action_sfx = NULL; }

    // Reset the loaded flag
    sound_effects_loaded = 0;

    // Exit MikMod
    MikMod_Exit();
}

// Update sound system
void soundIdle(void) {
    if (Player_Active())
        MikMod_Update();
}

void playCrashSound(void) {
    if (game->settings->playSound) {
        playSampleEffect(crash_sfx);
    }
}

void playLoseSound(void) {
    if (game->settings->playSound) {
        playSampleEffect(lose_sfx);
    }
}

void playWinSound(void) {
    if (game->settings->playSound) {
        playSampleEffect(win_sfx);
    }
}

void playHighlightSound(void) {
    if (game->settings->playSound) {
        playSampleEffect(highlight_sfx);
    }
}

void playEngineSound(void) {
    if (game->settings->playSound) {
        playSampleEffect(engine_sfx);
    }
}

void playStartSound(void) {
    if (game->settings->playSound) {
        playSampleEffect(start_sfx);
    }
}

void playActionSound(void) {
    if (game->settings->playSound) {
        playSampleEffect(action_sfx);
    }
}
