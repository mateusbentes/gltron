#include "sound.h"
#include "globals.h"
#include <stdio.h>
#include <string.h>

// Global sound module and sound effect variables
MODULE* sound_module = NULL;
MODULE* crash_sound = NULL;
MODULE* lose_sound = NULL;
MODULE* win_sound = NULL;
MODULE* highlight_sound = NULL;
MODULE* engine_sound = NULL;
MODULE* start_sound = NULL;
MODULE* action_sound = NULL;

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
    md_device = game->settings->sound_driver;  // Correct pointer dereferencing

    // Register all sound file loaders
    MikMod_RegisterAllLoaders();

    // Initialize MikMod
    if (MikMod_Init("")) {
        printf("Could not initialize sound: %s\n", MikMod_strerror(MikMod_errno));
        return 1;
    }

    // Initialize music setting
    game->settings->playMusic = 1;  // Correct pointer dereferencing

    // Load sound effects if sound is enabled
    if (game->settings->playSound) {
        if (loadSoundEffect("game_crash", &crash_sound)) {
            printf("Warning: Could not load crash sound effect\n");
        }
        if (loadSoundEffect("game_lose", &lose_sound)) {
            printf("Warning: Could not load lose sound effect\n");
        }
        if (loadSoundEffect("game_win", &win_sound)) {
            printf("Warning: Could not load win sound effect\n");
        }
        if (loadSoundEffect("menu_highlight", &highlight_sound)) {
            printf("Warning: Could not load highlight sound effect\n");
        }
        if (loadSoundEffect("game_engine", &engine_sound)) {
            printf("Warning: Could not load engine sound effect\n");
        }
        if (loadSoundEffect("game_start", &start_sound)) {
            printf("Warning: Could not load start sound effect\n");
        }
        if (loadSoundEffect("menu_action", &action_sound)) {
            printf("Warning: Could not load action sound effect\n");
        }
    }

    return 0;
}

// Load a sound effect from file
int loadSoundEffect(char* name, MODULE** sound_effect) {
    // Try loading from different locations
    const char* paths[] = {
        "./",  // Current directory
        "/usr/share/games/gltron/",  // Default system location
        "/usr/local/share/games/gltron/",  // Alternative system location
        NULL
    };

    // Try different file extensions
    const char* extensions[] = {".mod", ".xm", ".s3m", ".it", ".wav", NULL};

    char full_path[256];
    int i = 0;

    // Clear the sound effect pointer
    *sound_effect = NULL;

    while (paths[i] != NULL) {
        int j = 0;
        while (extensions[j] != NULL) {
            sprintf(full_path, "%s%s%s", paths[i], name, extensions[j]);
            printf("Attempting to load sound effect from: %s\n", full_path);

            // Try to load the file
            *sound_effect = Player_Load(full_path, 64, 0);

            if (*sound_effect) {
                printf("Successfully loaded sound effect from: %s\n", full_path);
                return 0;
            }
            j++;
        }
        i++;
    }

    printf("Could not load sound effect %s: %s\n",
           name, MikMod_strerror(MikMod_errno));

    // If we get here, all attempts failed
    if (MikMod_errno == MMERR_NOT_A_MODULE) {
        printf("The file is not in a recognized sound format.\n");
        printf("Please convert to MOD, XM, S3M, or IT format.\n");
    }

    return 1;
}

// Play a sound effect
int playSoundEffect(MODULE* sound_effect) {
    if (sound_effect && game->settings->playSound) {
        Player_Start(sound_effect);
        return 0;
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
    if (sound_module && game->settings->playSound && game->settings->playMusic) {
        Player_Start(sound_module);
        printf("sound started\n");
        return 0;
    }
    return 1;
}

// Stop the current sound module
int stopSound(void) {
    if (game->settings->playMusic) {
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
    if (crash_sound) Player_Free(crash_sound);
    if (lose_sound) Player_Free(lose_sound);
    if (win_sound) Player_Free(win_sound);
    if (highlight_sound) Player_Free(highlight_sound);
    if (engine_sound) Player_Free(engine_sound);
    if (start_sound) Player_Free(start_sound);
    if (action_sound) Player_Free(action_sound);

    // Exit MikMod
    MikMod_Exit();
}

// Update sound system
void soundIdle(void) {
    if (Player_Active())
        MikMod_Update();
}
