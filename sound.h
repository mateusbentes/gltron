#ifndef SOUND_H
#define SOUND_H

#include <mikmod.h>

// Music module
extern MODULE* sound_module;

// Sound effects as samples
extern SAMPLE* crash_sfx;
extern SAMPLE* lose_sfx;
extern SAMPLE* win_sfx;
extern SAMPLE* highlight_sfx;
extern SAMPLE* engine_sfx;
extern SAMPLE* start_sfx;
extern SAMPLE* action_sfx;

int initSound(void);
void deleteSound(void);
void soundIdle(void);
int loadSound(char *name);
int playSound(void);
int stopSound(void);

// Sample-based SFX helpers
int loadSampleEffect(char *name, SAMPLE** sfx);
int playSampleEffect(SAMPLE* sfx);

#endif
