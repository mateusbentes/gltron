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

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    struct {
        int x;
        int y;
        float angle;
        float speed;
    } SpeedDial;
    struct {
        int x;
        int y;
        int w;
        int h;
        const char* text;
    } SpeedText;
    struct {
        int x;
        int y;
        int active;
    } Buster;
    struct {
        int x;
        int y;
        int w;
        int h;
    } MapFrame;
} HUDType;

// Declare the global HUD instance
extern HUDType HUD;

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
 * Set the Lua state for the scripting module
 */
void scripting_SetLuaState(void *L_param);

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

/*
 * Register a C function with the scripting system
 */
void scripting_Register(const char* name, lua_CFunction func);

// Menu action functions
void SinglePlayerAction(void);
void MultiplayerAction(void);
void BackToMainMenuAction(void);
void SetResolution(const char* value);
void SetFullscreen(const char* value);
void BackToOptionsMenuAction(void);
void SetMusicVolume(int value);
void SetEffectsVolume(int value);
void ConfigureKeyboardControls(void);
void ConfigureMouseControls(void);
void QuitGameAction(void);

// HUD and gauge functions
void JoyThresholdUp(void);
void JoyThresholdDown(void);
void draw_hud(int score, const char* ai_message);

// Initialization functions
void menu_functions(void);
void menu(void);
void hudconfig(void);
void hud(void);
void gauge(void);
void android_touch(void);

// Other functions
void configureCallbacks(void);
void scripting_RunGC(void);
void exitGame(void);

#ifdef __cplusplus
}
#endif

#endif /* SCRIPTING_H */
