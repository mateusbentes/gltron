#ifndef SOUND_H
#define SOUND_H

#ifndef SOUND_BACKEND_OPENMPT
#include <mikmod.h>
#else
// Android backend: provide placeholder types so declarations compile
typedef void MODULE;
typedef void SAMPLE;
#endif

// Music module (MikMod only; on Android backend, these are placeholders)
extern MODULE* sound_module;

// Sound effects as samples (MikMod); on Android backend, these are used as IDs casted to pointers
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
