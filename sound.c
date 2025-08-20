#include "sound.h"
#include "globals.h"
#include <stdio.h>
#include <string.h>

// Global sound module and sound effect variables
MODULE* sound_module = NULL;
SAMPLE* crash_sfx = NULL;
SAMPLE* lose_sfx = NULL;
SAMPLE* win_sfx = NULL;
SAMPLE* highlight_sfx = NULL;
SAMPLE* engine_sfx = NULL;
SAMPLE* start_sfx = NULL;
SAMPLE* action_sfx = NULL;

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
    // Set sound mode and frequency
    md_mode |= DMODE_SOFT_MUSIC | DMODE_SOFT_SNDFX;
    md_mixfreq = 44100;

    // Register appropriate drivers based on platform
#ifdef WIN32
    MikMod_RegisterDriver(&drv_win);
#else
    MikMod_RegisterAllDrivers();
#endif

    printf("%s\n", MikMod_InfoDriver());

    // Set sound device from game settings
    md_device = game->settings->sound_driver;

    // Register all sound file loaders
    MikMod_RegisterAllLoaders();

    // Initialize MikMod
    if (MikMod_Init("")) {
        printf("Could not initialize sound: %s\n", MikMod_strerror(MikMod_errno));
        return 1;
    }

    // Load sound effects if sound is enabled
    if (game->settings->playSound) {
        if (loadSampleEffect("game_crash", &crash_sfx)) {
            printf("Warning: Could not load crash sample\n");
        }
        if (loadSampleEffect("game_lose", &lose_sfx)) {
            printf("Warning: Could not load lose sample\n");
        }
        if (loadSampleEffect("game_win", &win_sfx)) {
            printf("Warning: Could not load win sample\n");
        }
        if (loadSampleEffect("menu_highlight", &highlight_sfx)) {
            printf("Warning: Could not load highlight sample\n");
        }
        if (loadSampleEffect("game_engine", &engine_sfx)) {
            printf("Warning: Could not load engine sample\n");
        }
        if (loadSampleEffect("game_start", &start_sfx)) {
            printf("Warning: Could not load start sample\n");
        }
        if (loadSampleEffect("menu_action", &action_sfx)) {
            printf("Warning: Could not load action sample\n");
        }
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
    const char* paths[] = {
        "./",
        "/usr/share/games/gltron/",
        "/usr/local/share/games/gltron/",
        NULL
    };
    // Prefer wav for SFX; allow a few other common sample formats
    const char* exts[] = {".wav", ".aiff", ".aif", ".au", NULL};
    char full[512];
    *sfx_out = NULL;
    for (int i=0; paths[i]; ++i) {
        for (int e=0; exts[e]; ++e) {
            snprintf(full, sizeof(full), "%s%s%s", paths[i], name, exts[e]);
            printf("Attempting to load sample from: %s\n", full);
            SAMPLE* s = Sample_Load(full);
            if (s) {
                *sfx_out = s;
                printf("Successfully loaded sample: %s\n", full);
                return 0;
            }
        }
    }
    printf("Could not load sample %s: %s\n", name, MikMod_strerror(MikMod_errno));
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

// Load a sound module
int loadSound(char* name) {
    sound_module = Player_Load(name, 64, 0);
    if (!sound_module) {
        printf("Could not load module: %s\n", MikMod_strerror(MikMod_errno));
        return 1;
    }
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
    if (Player_Active()) {
        Player_Stop();
        printf("sound stopped\n");
        return 0;
    }
    return 1;
}

// Clean up sound system
void deleteSound(void) {
    // Stop any playing sound
    if (Player_Active())
        Player_Stop();

    // Free the main sound module
    if (sound_module)
        Player_Free(sound_module);

    // Free sound effects
    if (crash_sfx) Sample_Free(crash_sfx);
    if (lose_sfx) Sample_Free(lose_sfx);
    if (win_sfx) Sample_Free(win_sfx);
    if (highlight_sfx) Sample_Free(highlight_sfx);
    if (engine_sfx) Sample_Free(engine_sfx);
    if (start_sfx) Sample_Free(start_sfx);
    if (action_sfx) Sample_Free(action_sfx);

    // Exit MikMod
    MikMod_Exit();
}

// Update sound system
void soundIdle(void) {
    if (Player_Active())
        MikMod_Update();
}
