#include "base/switchCallbacks.h"
#include "base/nebu_callbacks.h"
#include "base/nebu_system.h"
#include "game/gltron.h"
#include <stdio.h>
#include <string.h>

/* Define the available callback sets */
extern Callbacks gameCallbacks;
extern Callbacks guiCallbacks;
extern Callbacks pauseCallbacks;
extern Callbacks creditsCallbacks;
extern Callbacks configureCallbacks;
extern Callbacks timedemoCallbacks;

/* Current active callback set */
static Callbacks *currentCallbacks = NULL;

/* Get the current callback set */
Callbacks* getCurrentCallbacks(void) {
    return currentCallbacks;
}

/* Set the current callback set based on name */
void setCallback(const char *name) {
    if (!name) {
        fprintf(stderr, "[error] setCallback: NULL name provided\n");
        return;
    }

    fprintf(stderr, "[debug] setCallback: setting callback to '%s'\n", name);

    if (strcmp(name, "game") == 0) {
        currentCallbacks = &gameCallbacks;
    } else if (strcmp(name, "gui") == 0) {
        currentCallbacks = &guiCallbacks;
    } else if (strcmp(name, "pause") == 0) {
        currentCallbacks = &pauseCallbacks;
    } else if (strcmp(name, "credits") == 0) {
        currentCallbacks = &creditsCallbacks;
    } else if (strcmp(name, "configure") == 0) {
        currentCallbacks = &configureCallbacks;
    } else if (strcmp(name, "timedemo") == 0) {
        currentCallbacks = &timedemoCallbacks;
    } else {
        fprintf(stderr, "[error] setCallback: unknown callback set '%s'\n", name);
        /* Default to GUI if unknown */
        currentCallbacks = &guiCallbacks;
    }

    /* Initialize the new callback set if it has an init function */
    if (currentCallbacks && currentCallbacks->init) {
        currentCallbacks->init();
    }
    
    /* Register the callbacks with the Nebu system */
    nebu_System_SetCallbacks(currentCallbacks);
}
