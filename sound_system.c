/* Alternative sound system using system audio players */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Play a WAV file using system command */
int playWavSystem(const char* filename) {
    char command[512];
    
    /* Check if file exists */
    if (access(filename, F_OK) != 0) {
        return 1;
    }
    
#ifdef __linux__
    /* Try different Linux audio players in order of preference */
    
    /* Try aplay (ALSA) */
    if (system("which aplay > /dev/null 2>&1") == 0) {
        snprintf(command, sizeof(command), "aplay -q '%s' 2>/dev/null &", filename);
        system(command);
        return 0;
    }
    
    /* Try paplay (PulseAudio) */
    if (system("which paplay > /dev/null 2>&1") == 0) {
        snprintf(command, sizeof(command), "paplay '%s' 2>/dev/null &", filename);
        system(command);
        return 0;
    }
    
    /* Try sox play command */
    if (system("which play > /dev/null 2>&1") == 0) {
        snprintf(command, sizeof(command), "play -q '%s' 2>/dev/null &", filename);
        system(command);
        return 0;
    }
#endif

#ifdef _WIN32
    /* Windows: use PlaySound API or mmsystem */
    snprintf(command, sizeof(command), "powershell -c \"(New-Object Media.SoundPlayer '%s').PlaySync()\" &", filename);
    system(command);
    return 0;
#endif

#ifdef __APPLE__
    /* macOS: use afplay */
    snprintf(command, sizeof(command), "afplay '%s' &", filename);
    system(command);
    return 0;
#endif

    return 1;
}

/* Helper function to play sound effects with system player */
void playSystemSound(const char* soundName) {
    char paths[3][256];
    snprintf(paths[0], sizeof(paths[0]), "./%s.wav", soundName);
    snprintf(paths[1], sizeof(paths[1]), "/usr/share/games/gltron/%s.wav", soundName);
    snprintf(paths[2], sizeof(paths[2]), "/usr/local/share/games/gltron/%s.wav", soundName);
    
    for (int i = 0; i < 3; i++) {
        if (access(paths[i], F_OK) == 0) {
            playWavSystem(paths[i]);
            return;
        }
    }
}
