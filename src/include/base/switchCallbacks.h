#ifndef SWITCH_CALLBACKS_H
#define SWITCH_CALLBACKS_H

#include "base/nebu_callbacks.h"

/**
 * @file switchCallbacks.h
 * @brief Header file for callback switching functionality.
 */

// Function prototypes
void game_Callbacks_ExitCurrent(void);
void game_Callbacks_InitCurrent(void);
void setCallback(const char *name);

// Declare the external callback variables
extern ExtendedCallbacks gameCallbacks;
extern ExtendedCallbacks guiCallbacks;
extern ExtendedCallbacks pauseCallbacks;
extern ExtendedCallbacks promptCallbacks;
extern ExtendedCallbacks creditsCallbacks;
extern ExtendedCallbacks timedemoCallbacks;
extern ExtendedCallbacks _32bit_warningCallbacks;

// Declare the configureCallbacks variable
extern ExtendedCallbacks configureCallbacks;

// Declare the last and current callback pointers
extern Callbacks *last_callback;
extern Callbacks *current_callback;

#endif // SWITCH_CALLBACKS_H
