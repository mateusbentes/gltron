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


Mix_Music *music = NULL;

static char *game_fx_names[] = {

#if 1
  "game_engine.wav",
  "game_crash.wav",
  "game_recognizer.wav"
#else
  "game_engine.ogg",
  "game_crash.ogg",
  "game_recognizer.ogg"
#endif
};

void Sound_loadFX(void) {
  int i;
  char *path;


  for(i = 0; i < NUM_GAME_FX; i++) {
    path = getPath(PATH_DATA, game_fx_names[i]);
    if(path) {
      Audio_LoadSample(path, i);
      free(path);
    } else {
      fprintf(stderr, "[error] can't load sound fx file %s\n",
	     game_fx_names[i]);
      nebu_assert(0); exit(1); // TODO: handle missing fx somewhere else
    }
  }
}

void Sound_reloadTrack(void) {
  const char *song = "default_song.mp3";
  char *path = NULL;

#ifdef USE_SCRIPTING
  scripting_GetGlobal("settings", "current_track", NULL);
  scripting_GetStringResult(&song);
#endif

  fprintf(stderr, "[sound] loading song %s\n", song);
  
  path = getPath(PATH_MUSIC, song);
  
  if(path == NULL) {
    fprintf(stderr, "[sound] can't find song...exiting\n");
    nebu_assert(0); exit(1);
  }

  Sound_load(path);
  Sound_play();

  free(path);
}

void Sound_shutdown(void) {
	Audio_UnloadPlayers();
	Audio_Quit();
}
  

void Sound_load(char *name) {
  Audio_LoadMusic(name);
}

void Sound_play(void) {
  Audio_SetMusicVolume(getSettingf("musicVolume"));
  Audio_PlayMusic();
  return;
}

void Sound_stop(void) {
  Audio_StopMusic();
}

void Sound_idle(void) {
  Audio_Idle();
}

void Sound_setMusicVolume(float volume) {
  if(volume > 1) volume = 1;
  if(volume < 0) volume = 0;
  Audio_SetMusicVolume(volume);
}

void Sound_setFxVolume(float volume) {
  if(volume > 1) volume = 1;
  if(volume < 0) volume = 0;
  Audio_SetFxVolume(volume);
}

void Sound_initTracks(void) {
  const char *music_path;
  nebu_List *soundList;
  nebu_List *p;
  int i;

  music_path = getDirectory(PATH_MUSIC);
  soundList = readDirectoryContents(music_path, NULL);
  
  if(soundList->next == NULL) {
    fprintf(stderr, "[sound] no music files found...exiting\n");
    nebu_assert(0); exit(1);
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

void Sound_setup(void) {
  printf("[sound] initializing sound\n");

  Audio_Init();
  Sound_loadFX();
  Sound_setFxVolume(getSettingf("fxVolume"));
  Sound_reloadTrack();
  Sound_setMusicVolume(getSettingf("musicVolume"));
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
