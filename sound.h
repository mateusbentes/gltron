#ifndef SOUND_H
#define SOUND_H

// Include MikMod header to get the MODULE type definition
#include <mikmod.h>

// Declare sound effect variables as external
extern MODULE* crash_sound;
extern MODULE* lose_sound;
extern MODULE* win_sound;
extern MODULE* highlight_sound;
extern MODULE* engine_sound;
extern MODULE* start_sound;
extern MODULE* action_sound;

int initSound(void);
void deleteSound(void);
void soundIdle(void);
int loadSound(char *name);
int playSound(void);
int stopSound(void);
int loadSoundEffect(char *name, MODULE** sound_effect);
int playSoundEffect(MODULE* sound_effect);

#endif
