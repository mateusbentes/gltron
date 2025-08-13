#ifndef PAUSE_H
#define PAUSE_H

#include "base/nebu_callbacks.h"

/**
 * @file pause.h
 * @brief Header file for pause functionality in the game.
 */

// Function prototypes for pause callbacks
void idlePause(void);
void keyboardPause(int state, int key, int x, int y);
void initPause(void);
void exitPause(void);
void displayPause(void);
void mousePause(int buttons, int state, int x, int y);
void mouseMotionPause(int mx, int my);
void pauseReshape(int x, int y);

// Function prototypes for prompt callbacks
void keyboardPrompt(int state, int key, int x, int y);
void initPrompt(void);
void exitPrompt(void);
void promptReshape(int x, int y);

// Declare the pause and prompt callbacks structures
extern ExtendedCallbacks pauseCallbacks;
extern ExtendedCallbacks promptCallbacks;

#endif // PAUSE_H
