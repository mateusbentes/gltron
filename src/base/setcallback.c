/* 
 * setcallback.c - Callback management for GLtron
 */
#include "base/switchCallbacks.h"
#include "base/nebu_callbacks.h"
#include "base/nebu_system.h"
#include "game/gltron.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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
    /* Defensive programming - check for NULL */
    if (!name) {
        fprintf(stderr, "[error] setCallback: NULL name provided\n");
        return;
    }

    fprintf(stderr, "[debug] setCallback: setting callback to '%s'\n", name);

    /* Use a safer string comparison approach */
    Callbacks *newCallbacks = NULL;
    
    if (name[0] == 'g') {
        if (name[1] == 'a' && name[2] == 'm' && name[3] == 'e' && name[4] == '\0') {
            newCallbacks = &gameCallbacks;
        } else if (name[1] == 'u' && name[2] == 'i' && name[3] == '\0') {
            newCallbacks = &guiCallbacks;
        }
    } else if (name[0] == 'p' && name[1] == 'a' && name[2] == 'u' && 
               name[3] == 's' && name[4] == 'e' && name[5] == '\0') {
        newCallbacks = &pauseCallbacks;
    } else if (name[0] == 'c') {
        if (name[1] == 'r' && name[2] == 'e' && name[3] == 'd' && 
            name[4] == 'i' && name[5] == 't' && name[6] == 's' && name[7] == '\0') {
            newCallbacks = &creditsCallbacks;
        } else if (name[1] == 'o' && name[2] == 'n' && name[3] == 'f' && 
                  name[4] == 'i' && name[5] == 'g' && name[6] == 'u' && 
                  name[7] == 'r' && name[8] == 'e' && name[9] == '\0') {
            newCallbacks = &configureCallbacks;
        }
    } else if (name[0] == 't' && name[1] == 'i' && name[2] == 'm' && 
               name[3] == 'e' && name[4] == 'd' && name[5] == 'e' && 
               name[6] == 'm' && name[7] == 'o' && name[8] == '\0') {
        newCallbacks = &timedemoCallbacks;
    }
    
    if (newCallbacks) {
        currentCallbacks = newCallbacks;
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
    if (currentCallbacks) {
        nebu_System_SetCallbacks(currentCallbacks);
    }
}
