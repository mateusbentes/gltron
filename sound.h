#ifndef SOUND_H
#define SOUND_H
#include <SDL/SDL_mixer.h>
int initSound(void);
void soundIdle(void);
int loadSound(char *name);
int playSound(void);
int stopSound(void);
#endif
