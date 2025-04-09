/* 
 * switchCallbacks.h - Callback management for GLtron
 * 
 * This file uses the Callbacks structure from nebu_callbacks.h
 */
#ifndef SWITCHCALLBACKS_H
#define SWITCHCALLBACKS_H

#include "base/nebu_callbacks.h"

/* Callback type enumeration */
typedef enum {
    CB_GAME = 0,
    CB_GUI = 1,
    CB_PAUSE = 2,
    CB_CONFIGURE = 3,
    CB_PROMPT = 4,
    CB_CREDITS = 5,
    CB_TIMEDEMO = 6,
    CB_32BIT_WARNING = 7,
    CB_UNKNOWN = 8
} CallbackType;

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
void setCallbackByType(CallbackType type);
Callbacks* getCurrentCallbacks(void);

/* Utility function to convert string to callback type */
CallbackType getCallbackTypeFromString(const char *name);

#endif /* SWITCHCALLBACKS_H */
