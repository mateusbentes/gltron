/* 
 * c_setcallback.c - Lua interface for setting callbacks
 */
#include "game/gltron.h"
#include "base/switchCallbacks.h"
#include "scripting/nebu_scripting.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Callback type definitions */
#define CB_GUI        0
#define CB_GAME       1
#define CB_PAUSE      2
#define CB_CREDITS    3
#define CB_CONFIGURE  4
#define CB_TIMEDEMO   5
#define CB_UNKNOWN   -1

/* Convert string callback name to numeric type */
static int getCallbackType(const char *name) {
    if (!name) return CB_UNKNOWN;
    
    if (strcmp(name, "gui") == 0)        return CB_GUI;
    if (strcmp(name, "game") == 0)       return CB_GAME;
    if (strcmp(name, "pause") == 0)      return CB_PAUSE;
    if (strcmp(name, "credits") == 0)    return CB_CREDITS;
    if (strcmp(name, "configure") == 0)  return CB_CONFIGURE;
    if (strcmp(name, "timedemo") == 0)   return CB_TIMEDEMO;
    
    return CB_UNKNOWN;
}

/* Lua interface for setting callbacks */
int c_SetCallback(lua_State *L) {
    const char *name;
    int callbackType;
    int top = lua_gettop(L);
    
    if(top < 1) {
        fprintf(stderr, "[fatal] no callback set name provided\n");
        return 0;
    }
    
    if(!lua_isstring(L, top)) {
        fprintf(stderr, "[fatal] invalid callback set (not a string)\n");
        return 0;
    }
    
    name = lua_tostring(L, top);
    if(!name) {
        fprintf(stderr, "[fatal] NULL callback set name\n");
        return 0;
    }
    
    fprintf(stderr, "[debug] Setting callback to: %s\n", name);
    
    /* Convert string to numeric callback type */
    callbackType = getCallbackType(name);
    if(callbackType == CB_UNKNOWN) {
        fprintf(stderr, "[warning] unknown callback set: %s\n", name);
        /* Continue anyway, as the setCallback function might handle unknown callbacks */
    }
    
    /* Call setCallback with the name */
    setCallback(name);
    
    return 0;
}
