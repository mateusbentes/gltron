#ifndef INIT_H
#define INIT_H

extern void initSubsystems(int argc, const char *argv[]);
extern void exitSubsystems(void);
extern void initScripting(void);
extern void initConfiguration(int argc, const char *argv[]);
extern void initVideo(void);
extern void initAudio(void);
extern void initInput(void);
extern void initGame(void);
extern void initGame2(void);
extern void initEnterGame(void);
extern void initExitGame(void);
int runGame(void);
int runGUI(void);
int runPause(void);
int runConfigure(void);
int runCredits(void);
int runTimedemo(void);

/* platform stuff */
#endif
