#include "sound.h"
#include "gltron.h"

/* linux only, at the moment */

static Mix_Music *music;

int initSound() {
  /* open the audio device */
  if(Mix_OpenAudio(22050, AUDIO_U16, 1, 1024) < 0) {
    printf("can't open audio device");
    exit(2);
  }
  return 0;
}

int loadSound(char *name) {
  music = Mix_LoadMUS(name);
  return 0;
}

int playSound() {
  if( ! Mix_PlayingMusic() )
    Mix_PlayMusic(music, -1);
  /* todo: remove the following once the bug in SDL_mixer is fixed */
  /* we don't want too many references to game objects here */
  return 0;
}

int stopSound() {
  if( Mix_PlayingMusic() )
    Mix_HaltMusic();
  return 0;
}

void soundIdle() {
  /* sdl_mixer uses pthreads, so no work here */
  return;
}
