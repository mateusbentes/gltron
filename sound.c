/* use libmikmod to play a soundsample */

#include <mikmod.h>
#include "gltron.h"

MODULE* sound_module;

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
    printf("Cound not initialize sound: %s\n",
	   MikMod_strerror(MikMod_errno));
    return 1;
  }
  return 0;
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
  if (sound_module) {
    Player_Start(sound_module);
    printf("sound startet\n");
    return 0;
  } else 
    return 1;
}

int stopSound() {
  Player_Stop();
  printf("sound stopped");
  return 0;
}

void deleteSound() {
  if(Player_Active())
    Player_Stop();
  if(sound_module)
    Player_Free(sound_module);
  MikMod_Exit();
}

void soundIdle() {
  if(Player_Active())
    MikMod_Update();
}
