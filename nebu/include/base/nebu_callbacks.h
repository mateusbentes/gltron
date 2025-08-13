#ifndef NEBU_CALLBACKS_H
#define NEBU_CALLBACKS_H

/**
 * @file nebu_callbacks.h
 * @brief Header file for callback structures and functions.
 */

// Define callback types
typedef enum {
    CB_UNKNOWN = 0,
    CB_GAME,
    CB_GUI,
    CB_PAUSE,
    CB_CREDITS,
    CB_CONFIGURE,
    CB_TIMEDEMO
} CallbackType;

// Basic callback structure
typedef struct Callbacks {
    void (*display)(void);
    void (*idle)(void);
    void (*keyboard)(int, int, int, int);
    void (*init)(void);
    void (*exit)(void);
    void (*mouse)(int, int, int, int);
    void (*mouseMotion)(int, int);
    void (*reshape)(int, int);  /* Added reshape callback */
    const char *name;  /* Changed from char* to const char* for better practice */
} Callbacks;

// Extended callback structure
typedef struct ExtendedCallbacks {
    Callbacks base;
    void (*special)(int, int, int);
    void (*specialUp)(int, int, int);
    void (*mouseWheel)(int, int);
    void (*touch)(int, int, int, int);
    void (*touchUp)(int, int, int, int);
    void (*touchMotion)(int, int, int, int);
    void (*touchPinch)(int, float, float, float);
    void (*touchRotate)(int, float, float, float);
} ExtendedCallbacks;

#endif // NEBU_CALLBACKS_H
