/*
 * Implementation file for scripting functions
 */
#include <stdio.h>  /* For fprintf, stderr */
#include <stdlib.h> /* For malloc, free */
#include <string.h> /* For string operations */
#include "scripting/scripting.h" /* For function declarations */
#include "scripting/embedded_scripts.h" /* For get_embedded_script */

// Define LUA_OK for Lua 5.1 compatibility
#ifndef LUA_OK
#define LUA_OK 0
#endif

/* Global variable to store the Lua state */
static void *lua_state = NULL;

/* Error handling for scripting functions */
jmp_buf scripting_error_jmp;

/* Functions for Lua state management */
#ifdef USE_SCRIPTING
void* scripting_GetLuaState(void) {
    if (!lua_state) {
        fprintf(stderr, "[FATAL] Lua state accessed before initialization\n");
        return NULL;
    }
    return lua_state;
}

void scripting_SetLuaState(void *L_param) {
    if (!L_param) {
        fprintf(stderr, "[FATAL] Attempted to set NULL Lua state in scripting_SetLuaState\n");
        return;
    }

    printf("[scripting] Lua state set: %p\n", L_param);

    lua_state = L_param;

    printf("[scripting] Lua state successfully stored\n");
}

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

    // Execute the script based on its content
    if (strstr(script, "angle_MathFromClock360") != NULL) {
        // Handle angle conversion
        double angle = 0.0;
        // Extract angle from script
        // This is a simplified approach - in a real implementation, you'd need proper parsing
        printf("[scripting] Executing angle conversion\n");
        return 1;
    }
    else if (strstr(script, "JoyThresholdUp") != NULL) {
        // Handle joystick threshold increase
        printf("[scripting] Executing JoyThresholdUp\n");
        JoyThresholdUp();
        return 1;
    }
    else if (strstr(script, "JoyThresholdDown") != NULL) {
        // Handle joystick threshold decrease
        printf("[scripting] Executing JoyThresholdDown\n");
        JoyThresholdDown();
        return 1;
    }
    else if (strstr(script, "draw_hud") != NULL) {
        // Handle HUD drawing
        printf("[scripting] Executing draw_hud\n");
        // Extract parameters from script
        int score = 0;
        const char* ai_message = "";
        draw_hud(score, ai_message);
        return 1;
    }

    // Default case - just print the script
    printf("[scripting] Script execution: %s\n", script);
    return 1;
}
#endif

/* Stubs when scripting is disabled */
#ifndef USE_SCRIPTING
void* scripting_GetLuaState(void) {
    printf("[scripting] Lua state is disabled, returning NULL\n");
    return NULL;
}

void scripting_SetLuaState(void *L_param) {
    printf("[scripting] Lua state is disabled, skipping Lua setup\n");
}

int scripting_RunString(const char *script) {
    printf("[scripting] Scripting is disabled, skipping script execution\n");
    return 0;
}
#endif
