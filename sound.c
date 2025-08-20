/* use libmikmod to play a soundsample */

#include <mikmod.h>
#include "gltron.h"

MODULE* sound_module;
MODULE* crash_sound;
MODULE* lose_sound;
MODULE* win_sound;
MODULE* highlight_sound;
MODULE* engine_sound;
MODULE* start_sound;
MODULE* action_sound;

int initSound() {
  md_mode |= DMODE_SOFT_MUSIC;
  md_mixfreq = 44100;

#ifdef WIN32
  MikMod_RegisterDriver(&drv_win);
#else
  MikMod_RegisterAllDrivers();
#endif
  printf("%s\n", MikMod_InfoDriver());
  md_device = game->settings->sound_driver;

  MikMod_RegisterAllLoaders();

  if(MikMod_Init("")) {
    printf("Could not initialize sound: %s\n",
           MikMod_strerror(MikMod_errno));
    return 1;
  }

  // Initialize music setting
  game->settings->playMusic = 1;  // Enable music by default

  // Load sound effects
  if (game->settings->playSound) {
    loadSoundEffect("game_crash.wav", &crash_sound);
    loadSoundEffect("game_lose.wav", &lose_sound);
    loadSoundEffect("game_win.wav", &win_sound);
    loadSoundEffect("menu_highlight.wav", &highlight_sound);
    loadSoundEffect("game_engine.wav", &engine_sound);
    loadSoundEffect("game_start.wav", &start_sound);
    loadSoundEffect("menu_action.wav", &action_sound);
  }

  return 0;
}

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

int playSoundEffect(MODULE* sound_effect) {
  if (sound_effect && game->settings->playSound) {
    Player_Start(sound_effect);
    return 0;
  }
  return 1;
}

int loadSound(char* name) {
  sound_module = Player_Load(name, 64, 0);
  if(!sound_module) {
    printf("Could not load module: %s\n",
	   MikMod_strerror(MikMod_errno));
    return 1;
  }
  return 0;
}

int playSound() {
  if (sound_module && game->settings->playSound && game->settings->playMusic) {
    Player_Start(sound_module);
    printf("sound started\n");
    return 0;
  }
  return 1;
}

int stopSound() {
  if (game->settings->playMusic) {
    Player_Stop();
    printf("sound stopped\n");
    return 0;
  }
  return 1;
}

void deleteSound() {
  if(Player_Active())
    Player_Stop();
  if(sound_module)
    Player_Free(sound_module);

  // Free sound effects
  if(crash_sound) Player_Free(crash_sound);
  if(lose_sound) Player_Free(lose_sound);
  if(win_sound) Player_Free(win_sound);
  if(highlight_sound) Player_Free(highlight_sound);
  if(engine_sound) Player_Free(engine_sound);
  if(start_sound) Player_Free(start_sound);
  if(action_sound) Player_Free(action_sound);

  MikMod_Exit();
}

void soundIdle() {
  if(Player_Active())
    MikMod_Update();
}
