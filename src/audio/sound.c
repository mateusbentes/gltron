#include "game/gltron.h"
#include "filesystem/path.h"
#include "Nebu_filesystem.h"
#include "audio/audio.h"
#include "scripting/nebu_scripting.h"
#include "audio/sound_glue.h"
#include "configuration/settings.h"

#include "base/nebu_debug_memory.h"
#include "base/nebu_assert.h"

#include <SDL_mixer.h>

#define NUM_GAME_FX 3

// Global variables
Mix_Music *music = NULL;
Mix_Chunk *game_fx[NUM_GAME_FX] = {NULL};
static char *game_fx_names[] = {
  "game_engine.wav",
  "game_crash.wav",
  "game_engine.wav"  // Use engine sound for recognizer since game_recognizer.wav doesn't exist
};

// Function to stop all playing sounds
void Sound_stopAll(void) {
    printf("[audio] Stopping all sounds\n");

    // Stop all channels
    Mix_HaltChannel(-1);

    // Stop music if playing
    if (Mix_PlayingMusic()) {
        Mix_HaltMusic();
    }

    printf("[audio] All sounds stopped\n");
}

// Function to free all loaded samples
void Sound_freeAllSamples(void) {
    printf("[audio] Freeing all samples\n");

    // Free game sound effects
    for (int i = 0; i < NUM_GAME_FX; i++) {
        if (game_fx[i] != NULL) {
            Mix_FreeChunk(game_fx[i]);
            game_fx[i] = NULL;
        }
    }

    printf("[audio] All samples freed\n");
}

// Function to free loaded music
void Sound_freeMusic(void) {
    printf("[audio] Freeing music\n");

    // Free the current music
    if (music != NULL) {
        Mix_FreeMusic(music);
        music = NULL;
    }

    printf("[audio] Music freed\n");
}

// Function to update the audio system
void Sound_update(void) {
    printf("[audio] Updating audio system\n");

    // In SDL_mixer, updates happen automatically
    // This function can be used for any custom audio processing

    printf("[audio] Audio system updated\n");
}

// Function to check for completed sounds and free them
void Sound_checkCompleted(void) {
    printf("[audio] Checking for completed sounds\n");

    // In SDL_mixer, sound completion is handled automatically
    // This function can be used for any custom sound management

    printf("[audio] Completed sounds checked\n");
}

// Function to play a sound sample
void Sound_playSample(int sampleId, float volume) {
    printf("[audio] Playing sample %d with volume %f\n", sampleId, volume);

    // Validate sample ID
    if (sampleId < 0 || sampleId >= NUM_GAME_FX) {
        printf("[audio] Error: Invalid sample ID %d\n", sampleId);
        return;
    }

    // Check if sample is loaded
    if (game_fx[sampleId] == NULL) {
        printf("[audio] Error: Sample %d not loaded\n", sampleId);
        return;
    }

    // Set volume (0-128)
    int sdlVolume = (int)(volume * 128.0f);
    if (sdlVolume < 0) sdlVolume = 0;
    if (sdlVolume > 128) sdlVolume = 128;

    // Play the sample
    Mix_VolumeChunk(game_fx[sampleId], sdlVolume);
    Mix_PlayChannel(-1, game_fx[sampleId], 0);

    printf("[audio] Sample %d played\n", sampleId);
}

// Function to load sound effects
void Sound_loadFX(void) {
  int i;
  char *path;

  for(i = 0; i < NUM_GAME_FX; i++) {
    // Try to load from data directory directly
    path = getPossiblePath(PATH_DATA, game_fx_names[i]);

    if(path && nebu_FS_Test(path)) {
      printf("[audio] Loading sound from: %s\n", path);
      game_fx[i] = Mix_LoadWAV(path);
      if (game_fx[i] == NULL) {
        printf("[audio] Error loading sound %s: %s\n", path, Mix_GetError());
        printf("[audio] File exists but failed to load. Check file format.\n");
      }
      free(path);
    } else {
      if(path) free(path);
      fprintf(stderr, "[error] can't load sound fx file %s\n", game_fx_names[i]);
      fprintf(stderr, "[error] Check if file exists in: %s\n", getDirectory(PATH_DATA));
      // Don't exit, just continue without this sound
      printf("[audio] Continuing without sound file: %s\n", game_fx_names[i]);
    }
  }
}

// Function to reload the current music track
void Sound_reloadTrack(void) {
  const char *song = "song_revenge_of_cats.it";  // Use the actual music file that exists
  char *path = NULL;

#ifdef USE_SCRIPTING
  scripting_GetGlobal("settings", "current_track", NULL);
  scripting_GetStringResult(&song);
#endif

  fprintf(stderr, "[sound] loading song %s\n", song);

  path = getPossiblePath(PATH_MUSIC, song);

  if(path == NULL || !nebu_FS_Test(path)) {
    if(path) free(path);
    fprintf(stderr, "[sound] can't find song %s, disabling music\n", song);
    // Don't exit, just disable music
    return;
  }

  Sound_load(path);
  Sound_play();

  free(path);
}

// Function to shutdown the audio system
void Sound_shutdown(void) {
  printf("[audio] Shutting down audio system\n");

  // Stop all playing sounds
  Sound_stopAll();

  // Free loaded samples
  Sound_freeAllSamples();

  // Free loaded music
  Sound_freeMusic();

  // Shutdown the audio system
  Audio_Shutdown();

  printf("[audio] Audio system shutdown complete\n");
}

// Function to load music
void Sound_load(char *name) {
  printf("[audio] Loading music: %s\n", name);

  // Free any existing music
  if (music != NULL) {
    Mix_FreeMusic(music);
    music = NULL;
  }

  // Load the music
  music = Mix_LoadMUS(name);
  if (music == NULL) {
    printf("[audio] Error loading music %s: %s\n", name, Mix_GetError());
  } else {
    printf("[audio] Music loaded successfully\n");
  }
}

// Function to play music
void Sound_play(void) {
  printf("[audio] Playing music\n");

  if (music == NULL) {
    printf("[audio] Error: No music loaded\n");
    return;
  }

  // Set music volume
  Sound_setMusicVolume(getSettingf("musicVolume"));

  // Play the music
  if (Mix_PlayMusic(music, -1) == -1) {
    printf("[audio] Error playing music: %s\n", Mix_GetError());
  } else {
    printf("[audio] Music playing\n");
  }
}

// Function to stop music
void Sound_stop(void) {
  printf("[audio] Stopping music\n");

  if (Mix_PlayingMusic()) {
    Mix_HaltMusic();
    printf("[audio] Music stopped\n");
  } else {
    printf("[audio] No music is currently playing\n");
  }
}

// Function to update audio system
void Sound_idle(void) {
  // In SDL_mixer, updates happen automatically
  // This function can be used for any custom audio processing
}

// Function to set music volume
void Sound_setMusicVolume(float volume) {
  printf("[audio] Setting music volume to %f\n", volume);

  // Clamp volume between 0 and 1
  if(volume > 1) volume = 1;
  if(volume < 0) volume = 0;

  // Convert to SDL_mixer volume (0-128)
  int sdlVolume = (int)(volume * 128.0f);
  if (sdlVolume < 0) sdlVolume = 0;
  if (sdlVolume > 128) sdlVolume = 128;

  Mix_VolumeMusic(sdlVolume);
  printf("[audio] Music volume set to %d\n", sdlVolume);
}

// Function to set sound effects volume
void Sound_setFxVolume(float volume) {
  printf("[audio] Setting FX volume to %f\n", volume);

  // Clamp volume between 0 and 1
  if(volume > 1) volume = 1;
  if(volume < 0) volume = 0;

  // Convert to SDL_mixer volume (0-128)
  int sdlVolume = (int)(volume * 128.0f);
  if (sdlVolume < 0) sdlVolume = 0;
  if (sdlVolume > 128) sdlVolume = 128;

  // Check if samples are loaded before setting volume
  for (int i = 0; i < NUM_GAME_FX; i++) {
    if (game_fx[i] != NULL) {
      Mix_VolumeChunk(game_fx[i], sdlVolume);
    }
  }

  printf("[audio] FX volume set to %d\n", sdlVolume);
}

// Function to initialize music tracks
void Sound_initTracks(void) {
  const char *music_path;
  nebu_List *soundList;
  nebu_List *p;
  int i;

  music_path = getDirectory(PATH_MUSIC);
  soundList = readDirectoryContents(music_path, NULL);

  if(soundList->next == NULL) {
    fprintf(stderr, "[sound] no music files found, continuing without music\n");
    return;  // Don't exit, just continue without music
  }

  i = 1;
  for(p = soundList; p->next != NULL; p = p->next) {
    char *path = getPossiblePath(PATH_MUSIC, (char*)p->data);

    if(path != NULL && nebu_FS_Test(path)) {
#ifdef USE_SCRIPTING
      scripting_RunFormat("tracks[%d] = \"%s\"", i, (char*)p->data);
#else
      Sound_load(path);
#endif
      i++;
      free(path);
    }

    free(p->data);
  }

  nebu_List_Free(soundList);

#ifdef USE_SCRIPTING
  scripting_Run("setupSoundTrack()");
#endif
}

// Function to setup the audio system
void Sound_setup(void) {
  printf("[sound] initializing sound\n");

  // Initialize the audio system
  Audio_Init();

  // Load sound effects
  Sound_loadFX();

  // Set sound effects volume only after sounds are loaded
  Sound_setFxVolume(getSettingf("fxVolume"));

  // Load and play the current track
  Sound_reloadTrack();

  // Set music volume
  Sound_setMusicVolume(getSettingf("musicVolume"));

  // Start the audio system
  Audio_Start();
}

// Function to restart the music track
void restartTrack(void) {
  // Check if music is currently playing
  if (Mix_PlayingMusic()) {
    // Stop the current music and restart playback
    printf("[audio] Restarting the current track...\n");

    // Restart the music from the beginning
    Sound_stop();      // Stop the music
    Sound_play();      // Start playing the music from the beginning
  } else {
    // If no music is currently playing, log an error
    printf("[audio] No music is currently playing, cannot restart.\n");
  }
}
