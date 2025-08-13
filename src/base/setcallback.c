/* 
 * setcallback.c - Callback management for GLtron with debug logging
 */
#include "base/switchCallbacks.h"
#include "base/nebu_callbacks.h"
#include "base/nebu_system.h"
#include "game/gltron.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Define the available callback sets */
extern ExtendedCallbacks gameCallbacks;
extern ExtendedCallbacks guiCallbacks;
extern ExtendedCallbacks pauseCallbacks;
extern ExtendedCallbacks creditsCallbacks;
extern ExtendedCallbacks configureCallbacks;
extern ExtendedCallbacks timedemoCallbacks;

/* Current active callback set */
static ExtendedCallbacks *currentCallbacks = NULL;

/* Get the current callback set */
ExtendedCallbacks* getCurrentCallbacks(void) {
    fprintf(stderr, "[debug] getCurrentCallbacks called\n");
    return currentCallbacks;
}

/* Convert string to callback type */
CallbackType getCallbackTypeFromString(const char *name) {
    fprintf(stderr, "[debug] getCallbackTypeFromString called with name=%p\n", name);

    if (!name) {
        fprintf(stderr, "[debug] getCallbackTypeFromString: NULL name provided\n");
        return CB_UNKNOWN;
    }

    fprintf(stderr, "[debug] getCallbackTypeFromString: checking name '%s'\n", name);

    /* Check the first character and length for quick filtering */
    size_t len = 0;
    const char *p = name;
    while (*p) {
        len++;
        p++;
    }

    fprintf(stderr, "[debug] getCallbackTypeFromString: name length is %zu\n", len);

    /* Now do character-by-character comparison */
    if (len == 4 && name[0] == 'g' && name[1] == 'a' && name[2] == 'm' && name[3] == 'e') {
        fprintf(stderr, "[debug] getCallbackTypeFromString: found CB_GAME\n");
        return CB_GAME;
    }

    if (len == 3 && name[0] == 'g' && name[1] == 'u' && name[2] == 'i') {
        fprintf(stderr, "[debug] getCallbackTypeFromString: found CB_GUI\n");
        return CB_GUI;
    }

    if (len == 5 && name[0] == 'p' && name[1] == 'a' && name[2] == 'u' && name[3] == 's' && name[4] == 'e') {
        fprintf(stderr, "[debug] getCallbackTypeFromString: found CB_PAUSE\n");
        return CB_PAUSE;
    }

    if (len == 7 && name[0] == 'c' && name[1] == 'r' && name[2] == 'e' && name[3] == 'd' && name[4] == 'i' && name[5] == 't' && name[6] == 's') {
        fprintf(stderr, "[debug] getCallbackTypeFromString: found CB_CREDITS\n");
        return CB_CREDITS;
    }

    if (len == 9 && name[0] == 'c' && name[1] == 'o' && name[2] == 'n' && name[3] == 'f' && name[4] == 'i' && name[5] == 'g' && name[6] == 'u' && name[7] == 'r' && name[8] == 'e') {
        fprintf(stderr, "[debug] getCallbackTypeFromString: found CB_CONFIGURE\n");
        return CB_CONFIGURE;
    }

    if (len == 8 && name[0] == 't' && name[1] == 'i' && name[2] == 'm' && name[3] == 'e' && name[4] == 'd' && name[5] == 'e' && name[6] == 'm' && name[7] == 'o') {
        fprintf(stderr, "[debug] getCallbackTypeFromString: found CB_TIMEDEMO\n");
        return CB_TIMEDEMO;
    }

    fprintf(stderr, "[debug] getCallbackTypeFromString: unknown callback name '%s'\n", name);
    return CB_UNKNOWN;
}

/* Set the current callback set based on type */
void setCallbackByType(CallbackType type) {
    fprintf(stderr, "[debug] setCallbackByType: setting callback type %d\n", type);

    /* Select the appropriate callback set */
    switch(type) {
        case CB_GAME:
            currentCallbacks = &gameCallbacks;
            break;
        case CB_GUI:
            currentCallbacks = &guiCallbacks;
            break;
        case CB_PAUSE:
            currentCallbacks = &pauseCallbacks;
            break;
        case CB_CREDITS:
            currentCallbacks = &creditsCallbacks;
            break;
        case CB_CONFIGURE:
            currentCallbacks = &configureCallbacks;
            break;
        case CB_TIMEDEMO:
            currentCallbacks = &timedemoCallbacks;
            break;
        case CB_UNKNOWN:
        default:
            fprintf(stderr, "[error] setCallbackByType: unknown callback type %d\n", type);
            /* Default to GUI if unknown */
            currentCallbacks = &guiCallbacks;
            break;
    }

    /* Initialize the new callback set if it has an init function */
    if (currentCallbacks && currentCallbacks->base.init) {
        fprintf(stderr, "[debug] setCallbackByType: calling init function\n");
        currentCallbacks->base.init();
        fprintf(stderr, "[debug] setCallbackByType: init function returned\n");
    }

    /* Register the callbacks with the Nebu system */
    if (currentCallbacks) {
        fprintf(stderr, "[debug] setCallbackByType: calling nebu_System_SetCallbacks\n");
        nebu_System_SetCallbacks(&currentCallbacks->base);
        fprintf(stderr, "[debug] setCallbackByType: nebu_System_SetCallbacks returned\n");
    }

    fprintf(stderr, "[debug] setCallbackByType: function completed\n");
}

/* Set the current callback set based on name */
void setCallbackSafe(const char *name) {
    fprintf(stderr, "[debug] setCallbackSafe called with name=%p\n", name);

    /* Defensive programming - check for NULL or empty string */
    if (!name || name[0] == '\0') {
        fprintf(stderr, "[error] setCallbackSafe: NULL or empty name provided\n");
        setCallbackByType(CB_GUI); /* Default to GUI */
        return;
    }

    fprintf(stderr, "[debug] setCallbackSafe: setting callback to '%s'\n", name);

    /* Convert string to callback type and set it */
    CallbackType type = getCallbackTypeFromString(name);
    setCallbackByType(type);
}
