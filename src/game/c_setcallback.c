/* 
 * c_setcallback.c - Lua interface for setting callbacks
 */
#include "game/gltron.h"
#include "base/switchCallbacks.h"
#include "scripting/nebu_scripting.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* External declarations */
extern void video_LoadLevel(void);
extern void *gWorld;  /* Add this line to declare gWorld */

/* Lua interface for setting callbacks */
int c_SetCallback(lua_State *L) {
    const char *name;
    int top = lua_gettop(L);
    
    if(top < 1) {
        fprintf(stderr, "[fatal] no callback set name provided\n");
        setCallbackByType(CB_GUI); /* Default to GUI */
        return 0;
    }
    
    if(!lua_isstring(L, top)) {
        fprintf(stderr, "[fatal] invalid callback set (not a string)\n");
        setCallbackByType(CB_GUI); /* Default to GUI */
        return 0;
    }
    
    name = lua_tolstring(L, top, NULL);
    if(!name || name[0] == '\0') {
        fprintf(stderr, "[fatal] NULL or empty callback set name\n");
        setCallbackByType(CB_GUI); /* Default to GUI */
        return 0;
    }
    
    fprintf(stderr, "[debug] c_SetCallback: setting callback to: %s\n", name);
    
    /* Check if trying to set to "pause" but gWorld is NULL */
    if(strcmp(name, "pause") == 0 && !gWorld) {
        fprintf(stderr, "[warning] c_SetCallback: trying to set to 'pause' but gWorld is NULL\n");
        fprintf(stderr, "[warning] c_SetCallback: loading level first\n");
        video_LoadLevel();
    }
    
    /* Convert string to callback type directly */
    CallbackType type = getCallbackTypeFromString(name);
    
    /* Call setCallbackByType directly with the numeric type */
    setCallbackByType(type);
    
    return 0;
}
