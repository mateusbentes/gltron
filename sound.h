#ifndef SOUND_H
#define SOUND_H
int initSound(void);
void deleteSound(void);
void soundIdle(void);
int loadSound(char *name);
int playSound(void);
int stopSound(void);
#endif
