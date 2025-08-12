/*
 * scripting.h
 * Header file for scripting functions
 */
#ifndef SCRIPTING_H
#define SCRIPTING_H

#include <setjmp.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

// Include nebu_scripting.h for the enum definitions
#include "scripting/nebu_scripting.h"

// make sure this list matches the one in main.lua
typedef enum EScriptingReturnCode {
    eSRC_Quit = 0,
    eSRC_Game_End = 1,
    eSRC_Game_Pause = 2,
    eSRC_Game_Unpause = 3,
    eSRC_Game_Credits = 4,
    eSRC_Game_Escape = 5,
    eSRC_Timedemo = 7,
    eSRC_Timedemo_Abort = 8,
    eSRC_Credits = 9,
    eSRC_Game_Launch = 10,
    eSRC_GUI_Escape = 11,
    eSRC_GUI_Prompt = 12,
    eSRC_GUI_Prompt_Escape = 13,
    eSRC_Pause_Escape = 14
} eScriptingReturnCodes;

/* Error handling for scripting functions */
extern jmp_buf scripting_error_jmp;

/*
 * Initialize the scripting module with specified flags
 */
void scripting_Init(int flags);

/*
 * Initialize the scripting interface
 */
void init_c_interface(void);

/*
 * Get the Lua state from the scripting module
 */
#ifdef USE_SCRIPTING
lua_State* scripting_GetLuaState(void);
#else
void* scripting_GetLuaState(void);
#endif

/*
 * Run a Lua script from a file
 * Returns 0 on success, non-zero on error
 */
int scripting_RunFile(const char *filename);

/*
 * Run a Lua script from a string with error handling
 * Returns 0 on success, non-zero on error
 */
int scripting_RunString(const char *script);

/*
 * Clean up the scripting module
 */
void scripting_Quit(void);

/*
 * Register SDL2 compatibility functions
 */
void scripting_RegisterSDL2Compat(lua_State *L);

#endif /* SCRIPTING_H */
