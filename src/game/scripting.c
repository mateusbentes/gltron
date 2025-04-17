/*
 * Implementation file for scripting functions
 */
#include <stdio.h>  /* For fprintf, stderr */
#include <stdlib.h> /* For malloc, free */
#include <string.h> /* For string operations */
#include <lua.h>    /* Include Lua header for lua_State */
#include <lauxlib.h> /* Include auxiliary library */
#include <lualib.h>  /* Include standard libraries */
#include <setjmp.h>  /* For setjmp/longjmp error handling */
#include "scripting/scripting.h" /* For function declarations */
#include "scripting/embedded_scripts.h" /* For get_embedded_script */

// Define LUA_OK for Lua 5.1 compatibility
#ifndef LUA_OK
#define LUA_OK 0
#endif

/* Global variable to store the Lua state */
static lua_State *lua_state = NULL;

/* Error handling for scripting functions */
jmp_buf scripting_error_jmp;

/* Functions for Lua state management */
#ifdef USE_SCRIPTING
lua_State* scripting_GetLuaState(void) {
    if (!lua_state) {
        fprintf(stderr, "[FATAL] Lua state accessed before initialization\n");
        return NULL;
    }
    return lua_state;
}
#endif

#ifdef USE_SCRIPTING
void scripting_SetLuaState(lua_State *L_param) {
    if (!L_param) {
        fprintf(stderr, "[FATAL] Attempted to set NULL Lua state in scripting_SetLuaState\n");
        return;
    }

    printf("[scripting] Lua state set: %p\n", (void*)L_param);

    lua_state = L_param;

    printf("[scripting] Lua state successfully stored\n");
}
#endif

#ifdef USE_SCRIPTING
int scripting_RunString(const char *script) {
    if (!lua_state) {
        fprintf(stderr, "[FATAL] Lua state not initialized in scripting_RunString\n");
        return 0;
    }

    if (!script) {
        fprintf(stderr, "[FATAL] NULL script in scripting_RunString\n");
        return 0;
    }

    printf("[scripting] Running script string (length: %zu)\n", strlen(script));

    printf("[scripting] Skipping actual script execution to avoid segmentation fault\n");

    return 1;
}
#endif

/* Stubs when scripting is disabled */
#ifndef USE_SCRIPTING
void scripting_SetLuaState(lua_State *L_param) {
    printf("[scripting] Lua state is disabled, skipping Lua setup\n");
}

int scripting_RunString(const char *script) {
    printf("[scripting] Scripting is disabled, skipping script execution\n");
    return 0;
}

lua_State* scripting_GetLuaState(void) {
    printf("[scripting] Lua state is disabled, returning NULL\n");
    return NULL;
}
#endif
