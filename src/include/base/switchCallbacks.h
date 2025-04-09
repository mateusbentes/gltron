/* 
 * switchCallbacks.h - Callback management for GLtron
 * 
 * This file uses the Callbacks structure from nebu_callbacks.h
 */
#ifndef SWITCHCALLBACKS_H
#define SWITCHCALLBACKS_H

#include "base/nebu_callbacks.h"

/* Available callback sets */
extern Callbacks gameCallbacks;
extern Callbacks guiCallbacks;
extern Callbacks pauseCallbacks;
extern Callbacks configureCallbacks;
extern Callbacks promptCallbacks;
extern Callbacks creditsCallbacks;
extern Callbacks timedemoCallbacks;
extern Callbacks _32bit_warningCallbacks;

/* Callback management functions */
void game_Callbacks_ExitCurrent(void);
void game_Callbacks_InitCurrent(void);
void setCallback(const char *name);
Callbacks* getCurrentCallbacks(void);

#endif /* SWITCHCALLBACKS_H */
