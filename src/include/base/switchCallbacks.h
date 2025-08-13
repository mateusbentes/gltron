/* 
 * switchCallbacks.h - Callback management for GLtron
 *
 * This file uses the Callbacks structure from nebu_callbacks.h
 */
#ifndef SWITCHCALLBACKS_H
#define SWITCHCALLBACKS_H

#include "base/nebu_callbacks.h"

/* Extended Callbacks structure with additional callback types */
typedef struct {
    Callbacks base;  // Inherit from the base Callbacks struct
    void (*special)(int key, int x, int y);
    void (*specialUp)(int key, int x, int y);
    void (*mouseWheel)(int wheel, int direction, int x, int y);

    // Touch callbacks
    void (*touch)(int id, int x, int y);
    void (*touchUp)(int id, int x, int y);
    void (*touchMotion)(int id, int x, int y);
    void (*touchPinch)(int id1, int id2, float scale);
    void (*touchRotate)(int id1, int id2, float angle);
} ExtendedCallbacks;

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
extern ExtendedCallbacks gameCallbacks;
extern ExtendedCallbacks guiCallbacks;
extern ExtendedCallbacks pauseCallbacks;
extern ExtendedCallbacks promptCallbacks;
extern ExtendedCallbacks creditsCallbacks;
extern ExtendedCallbacks timedemoCallbacks;
extern ExtendedCallbacks _32bit_warningCallbacks;
extern ExtendedCallbacks configureCallbacks;  // Added this declaration

/* Callback management functions */
void game_Callbacks_ExitCurrent(void);
void game_Callbacks_InitCurrent(void);
void setCallback(const char *name);
void setCallbackByType(CallbackType type);
void setCallbackSafe(const char *name);
ExtendedCallbacks* getCurrentCallbacks(void);

/* Utility function to convert string to callback type */
CallbackType getCallbackTypeFromString(const char *name);

#endif /* SWITCHCALLBACKS_H */
