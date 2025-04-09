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
    if (!name || name[0] == '\0') return CB_UNKNOWN;
    
    /* Check the first character and length for quick filtering */
    size_t len = 0;
    const char *p = name;
    while (*p) {
        len++;
        p++;
    }
    
    /* Now do character-by-character comparison */
    if (len == 3 && name[0] == 'g' && name[1] == 'u' && name[2] == 'i')
        return CB_GUI;
    if (len == 4 && name[0] == 'g' && name[1] == 'a' && name[2] == 'm' && name[3] == 'e')
        return CB_GAME;
    if (len == 5 && name[0] == 'p' && name[1] == 'a' && name[2] == 'u' && name[3] == 's' && name[4] == 'e')
        return CB_PAUSE;
    if (len == 7 && name[0] == 'c' && name[1] == 'r' && name[2] == 'e' && name[3] == 'd' && name[4] == 'i' && name[5] == 't' && name[6] == 's')
        return CB_CREDITS;
    if (len == 9 && name[0] == 'c' && name[1] == 'o' && name[2] == 'n' && name[3] == 'f' && name[4] == 'i' && name[5] == 'g' && name[6] == 'u' && name[7] == 'r' && name[8] == 'e')
        return CB_CONFIGURE;
    if (len == 8 && name[0] == 't' && name[1] == 'i' && name[2] == 'm' && name[3] == 'e' && name[4] == 'd' && name[5] == 'e' && name[6] == 'm' && name[7] == 'o')
        return CB_TIMEDEMO;
    
    return CB_UNKNOWN;
}

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
    
    name = lua_tostring(L, top);
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
