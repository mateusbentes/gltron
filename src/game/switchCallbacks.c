#include "base/switchCallbacks.h"
#include "base/nebu_callbacks.h"
#include "base/nebu_system.h"

#include "base/nebu_debug_memory.h"

#include <string.h>
#include <stdio.h>
#include "base/nebu_assert.h"

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

ExtendedCallbacks configureCallbacks = {
    .base = {
        .name = "configure",
        .init = NULL,
        .exit = NULL,
        .idle = NULL,
        .reshape = NULL,
        .keyboard = NULL,
        .mouse = NULL,
        .mouseMotion = NULL
    },
    .special = NULL,
    .specialUp = NULL,
    .mouseWheel = NULL,

    // Initialize touch callbacks
    .touch = NULL,
    .touchUp = NULL,
    .touchMotion = NULL,
    .touchPinch = NULL,
    .touchRotate = NULL
};

Callbacks *last_callback = NULL;
Callbacks *current_callback = NULL;

void game_Callbacks_ExitCurrent(void) {
    /* called when the window is recreated */
    if(current_callback && current_callback->exit)
        (current_callback->exit)(); /* give them the chance to quit */
}

void game_Callbacks_InitCurrent(void)
{
    /* called when the window is recreated */
    nebu_System_SetCallback_Key(current_callback);
    // calls init
}

#define N_CALLBACKS 7
Callbacks *callbackList[N_CALLBACKS] = {
    &gameCallbacks, &guiCallbacks, &pauseCallbacks, &configureCallbacks.base,
    &promptCallbacks, &creditsCallbacks, &timedemoCallbacks
};

void setCallback(const char *name) {
    int i;

    /* Check for NULL or empty name */
    if (!name || name[0] == '\0') {
        fprintf(stderr, "error: NULL or empty callback name provided\n");
        /* Default to GUI */
        for(i = 0; i < N_CALLBACKS; i++) {
            if(strcmp(callbackList[i]->name, "gui") == 0)
                break;
        }
        if(i == N_CALLBACKS) {
            fprintf(stderr, "fatal: no callback named 'gui' found\n");
            return; /* Don't crash, just return */
        }
    } else {
        /* Find the callback with the given name */
        for(i = 0; i < N_CALLBACKS; i++) {
            if(strcmp(callbackList[i]->name, name) == 0)
                break;
        }
        if(i == N_CALLBACKS) {
            fprintf(stderr, "fatal: no callback named '%s' found\n", name);
            /* Default to GUI */
            for(i = 0; i < N_CALLBACKS; i++) {
                if(strcmp(callbackList[i]->name, "gui") == 0)
                    break;
            }
            if(i == N_CALLBACKS) {
                fprintf(stderr, "fatal: no callback named 'gui' found\n");
                return; /* Don't crash, just return */
            }
        }
    }

    last_callback = current_callback;
    current_callback = callbackList[i];

    nebu_System_SetCallbacks(callbackList[i]);
}
