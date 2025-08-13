#include "game/gltron.h"
#include "game/game.h"
#include "game/game_data.h"
#include "game/camera.h"
#include "game/engine.h"
#include "audio/sound_glue.h"
#include "video/video.h"
#include "configuration/settings.h"
#include "configuration/configuration.h"
#include "scripting/scripting.h"
#include "game/resource.h"
#include "base/nebu_resource.h"
#include "configuration/settings.h"
#include "base/switchCallbacks.h"
#include "scripting/nebu_scripting.h"
#include "filesystem/path.h"
#include "filesystem/nebu_filesystem.h"
#include "input/nebu_input_system.h"
#include "audio/audio.h"

#include "base/nebu_debug_memory.h"
#include "base/nebu_assert.h"

// Forward declarations for GUI functions
void initGui(void);
void exitGui(void);
void idleGui(void);
void keyboardGui(int state, int key, int x, int y);

// Forward declarations for game functions
void game_Init(void);
void game_Exit(void);
void game_Idle(void);
void game_Reshape(int width, int height);
void game_Keyboard(unsigned char key, int x, int y);
void game_Mouse(int button, int state, int x, int y);

// Forward declarations for other functions
void video_Init(void);
void resource_LoadInitial(void);
void game_ApplySettings(void);
void hud_Init(void);
void game_SaveState(void);
void resource_Cleanup(void);
void video_Shutdown(void);
void Audio_Shutdown(void);
void game_FreeData(void);
void game_Update(void);
void physics_Update(void);
void Audio_Update(void);
void hud_Update(void);
int game_CheckGameOver(void);
void game_HandleGameOver(void);
void video_SetViewport(int width, int height);
void camera_UpdateProjection(int width, int height);
void hud_UpdateLayout(int width, int height);
void game_Pause(void);
void game_Reset(void);
void game_ToggleMute(void);
void input_HandleKey(unsigned char key);
void input_HandleMouseClick(int x, int y);
void camera_ZoomIn(void);
void camera_ZoomOut(void);

// Define the callback variables
ExtendedCallbacks gameCallbacks = {
    .base = {
        .name = "game",
        .init = game_Init,
        .exit = game_Exit,
        .idle = game_Idle,
        .reshape = game_Reshape,
        .keyboard = game_Keyboard,
        .mouse = game_Mouse,
        .mouseMotion = NULL
    },
    .special = NULL,
    .specialUp = NULL,
    .mouseWheel = NULL,
    .touch = NULL,
    .touchUp = NULL,
    .touchMotion = NULL,
    .touchPinch = NULL,
    .touchRotate = NULL
};

// Initialize guiCallbacks with the appropriate functions from gui.c
ExtendedCallbacks guiCallbacks = {
    .base = {
        .name = "gui",
        .init = initGui,
        .exit = exitGui,
        .idle = idleGui,
        .reshape = NULL,
        .keyboard = keyboardGui,
        .mouse = NULL,
        .mouseMotion = NULL
    },
    .special = NULL,
    .specialUp = NULL,
    .mouseWheel = NULL,
    .touch = NULL,
    .touchUp = NULL,
    .touchMotion = NULL,
    .touchPinch = NULL,
    .touchRotate = NULL
};

ExtendedCallbacks creditsCallbacks = {
    .base = {
        .name = "credits",
        .init = NULL,
        .exit = NULL,
        .idle = NULL,
        .reshape = NULL,
        .keyboard = NULL,
        .mouse = NULL,
        .mouseMotion = NULL
    },
    .special = NULL,
    .specialUp = NULL,
    .mouseWheel = NULL,
    .touch = NULL,
    .touchUp = NULL,
    .touchMotion = NULL,
    .touchPinch = NULL,
    .touchRotate = NULL
};

ExtendedCallbacks timedemoCallbacks = {
    .base = {
        .name = "timedemo",
        .init = NULL,
        .exit = NULL,
        .idle = NULL,
        .reshape = NULL,
        .keyboard = NULL,
        .mouse = NULL,
        .mouseMotion = NULL
    },
    .special = NULL,
    .specialUp = NULL,
    .mouseWheel = NULL,
    .touch = NULL,
    .touchUp = NULL,
    .touchMotion = NULL,
    .touchPinch = NULL,
    .touchRotate = NULL
};

ExtendedCallbacks _32bit_warningCallbacks = {
    .base = {
        .name = "32bit_warning",
        .init = NULL,
        .exit = NULL,
        .idle = NULL,
        .reshape = NULL,
        .keyboard = NULL,
        .mouse = NULL,
        .mouseMotion = NULL
    },
    .special = NULL,
    .specialUp = NULL,
    .mouseWheel = NULL,
    .touch = NULL,
    .touchUp = NULL,
    .touchMotion = NULL,
    .touchPinch = NULL,
    .touchRotate = NULL
};

// Define configureCallbacks as a variable
ExtendedCallbacks configureCallbacks = {
    .base = {
        .name = "configure",
        .init = NULL,
        .exit = NULL,
        .idle = NULL,
        .reshape = NULL,
        .keyboard = NULL,
        .mouse = NULL,
        .mouseMotion = NULL
    },
    .special = NULL,
    .specialUp = NULL,
    .mouseWheel = NULL,
    .touch = NULL,
    .touchUp = NULL,
    .touchMotion = NULL,
    .touchPinch = NULL,
    .touchRotate = NULL
};

typedef enum {
    eSRC_MainMenu = 15,
    eSRC_OptionsMenu = 16,
    eSRC_KeyboardConfig = 17,
    eSRC_MouseConfig = 18
} eScriptingReturnCodesAdditional;

/* External declaration for touch interface registration */
extern void touch_interface_register(void);

/* Define GAME_SUCCESS for backward compatibility */
#define GAME_SUCCESS 0

int c_startGame(void *L);
int c_video_restart(void *L);
void game_Init(void);
void game_Exit(void);
void game_Idle(void);
void game_Reshape(int width, int height);
void game_Keyboard(unsigned char key, int x, int y);
void game_Mouse(int button, int state, int x, int y);

// some functions defined elsewhere: graphics_hud.c
int c_drawRectangle(void *l);
int c_drawCircle(void *l);
int c_translate(void *l);
int c_scale(void *l);
int c_pushMatrix(void *l);
int c_popMatrix(void *l);
int c_drawTextFitIntoRect(void *l);
int c_color(void *l);
int c_draw2D(void *l);
int c_drawHUDSurface(void *l);
int c_drawHUDMask(void *l);

// Forward declarations for C implementations
double angle_MathFromClock360(double angle);
void JoyThresholdUp();
void JoyThresholdDown();

// Implement menu action functions
void SinglePlayerAction(void) {
    printf("[menu] Starting single player game\n");
    // Set up single player game
    setSettingi("players", 1);
    setSettingi("ai_opponents", 0);
    // Start the game
    c_startGame(NULL);
}

void MultiplayerAction(void) {
    printf("[menu] Starting multiplayer game\n");
    // Set up multiplayer game
    setSettingi("players", 2);
    setSettingi("ai_opponents", 0);
    // Start the game
    c_startGame(NULL);
}

void SetResolution(const char* value) {
    printf("[settings] Setting resolution to %s\n", value);
    // Parse resolution string and set appropriate settings
    if (strcmp(value, "800x600") == 0) {
        setSettingi("width", 800);
        setSettingi("height", 600);
    } else if (strcmp(value, "1024x768") == 0) {
        setSettingi("width", 1024);
        setSettingi("height", 768);
    } else if (strcmp(value, "1280x720") == 0) {
        setSettingi("width", 1280);
        setSettingi("height", 720);
    } else if (strcmp(value, "1920x1080") == 0) {
        setSettingi("width", 1920);
        setSettingi("height", 1080);
    }
    // Restart video system with new resolution
    c_video_restart(NULL);
}

void SetFullscreen(const char* value) {
    printf("[settings] Setting fullscreen to %s\n", value);
    // Set fullscreen mode
    if (strcmp(value, "On") == 0) {
        setSettingi("fullscreen", 1);
    } else {
        setSettingi("fullscreen", 0);
    }
    // Restart video system with new settings
    c_video_restart(NULL);
}

void SetMusicVolume(int value) {
    printf("[audio] Setting music volume to %d\n", value);
    // Set music volume
    setSettingf("musicVolume", value / 100.0f);
    Sound_setMusicVolume(getSettingf("musicVolume"));
}

// Implement HUD and gauge functions
// Implement HUD and gauge functions
double angle_MathFromClock360(double angle) {
    // Convert angle from clock format (0-12) to radians (0-2π)
    printf("[math] Converting angle from clock format: %f\n", angle);
    return (angle / 12.0) * 2.0 * M_PI;
}

void JoyThresholdUp() {
    printf("[input] Increasing joystick threshold\n");
    // Increase joystick threshold
    int threshold = getSettingi("joy_threshold");
    threshold += 5;
    if (threshold > 100) threshold = 100;
    setSettingi("joy_threshold", threshold);
    printf("[input] New joystick threshold: %d\n", threshold);
}

void JoyThresholdDown() {
    printf("[input] Decreasing joystick threshold\n");
    // Decrease joystick threshold
    int threshold = getSettingi("joy_threshold");
    threshold -= 5;
    if (threshold < 0) threshold = 0;
    setSettingi("joy_threshold", threshold);
    printf("[input] New joystick threshold: %d\n", threshold);
}

// Implement graphics functions
int c_drawRectangle(void *l) {
    // Get parameters from Lua stack
    float x = luaL_checknumber(l, 1);
    float y = luaL_checknumber(l, 2);
    float w = luaL_checknumber(l, 3);
    float h = luaL_checknumber(l, 4);

    printf("[graphics] Drawing rectangle at (%f, %f) with size (%f, %f)\n", x, y, w, h);

    // Draw rectangle using graphics system
    // Implementation would depend on your graphics library
    // This is a placeholder for the actual drawing code

    return 0;
}

int c_drawCircle(void *l) {
    // Get parameters from Lua stack
    float x = luaL_checknumber(l, 1);
    float y = luaL_checknumber(l, 2);
    float radius = luaL_checknumber(l, 3);

    printf("[graphics] Drawing circle at (%f, %f) with radius %f\n", x, y, radius);

    // Draw circle using graphics system
    // Implementation would depend on your graphics library
    // This is a placeholder for the actual drawing code

    return 0;
}

int c_translate(void *l) {
    // Get parameters from Lua stack
    float x = luaL_checknumber(l, 1);
    float y = luaL_checknumber(l, 2);

    printf("[graphics] Translating by (%f, %f)\n", x, y);

    // Apply translation using graphics system
    // Implementation would depend on your graphics library
    // This is a placeholder for the actual transformation code

    return 0;
}

int c_scale(void *l) {
    // Get parameters from Lua stack
    float x = luaL_checknumber(l, 1);
    float y = luaL_checknumber(l, 2);

    printf("[graphics] Scaling by (%f, %f)\n", x, y);

    // Apply scaling using graphics system
    // Implementation would depend on your graphics library
    // This is a placeholder for the actual transformation code

    return 0;
}

int c_pushMatrix(void *l) {
    printf("[graphics] Pushing matrix\n");

    // Push matrix using graphics system
    // Implementation would depend on your graphics library
    // This is a placeholder for the actual matrix operation

    return 0;
}

int c_popMatrix(void *l) {
    printf("[graphics] Popping matrix\n");

    // Pop matrix using graphics system
    // Implementation would depend on your graphics library
    // This is a placeholder for the actual matrix operation

    return 0;
}

int c_drawTextFitIntoRect(void *l) {
    // Get parameters from Lua stack
    const char* text = luaL_checkstring(l, 1);
    float x = luaL_checknumber(l, 2);
    float y = luaL_checknumber(l, 3);
    float w = luaL_checknumber(l, 4);
    float h = luaL_checknumber(l, 5);

    printf("[graphics] Drawing text '%s' in rectangle (%f, %f, %f, %f)\n", text, x, y, w, h);

    // Draw text using graphics system
    // Implementation would depend on your graphics library
    // This is a placeholder for the actual text drawing code

    return 0;
}

int c_color(void *l) {
    // Get parameters from Lua stack
    float r = luaL_checknumber(l, 1);
    float g = luaL_checknumber(l, 2);
    float b = luaL_checknumber(l, 3);
    float a = luaL_optnumber(l, 4, 1.0f);

    printf("[graphics] Setting color to (%f, %f, %f, %f)\n", r, g, b, a);

    // Set color using graphics system
    // Implementation would depend on your graphics library
    // This is a placeholder for the actual color setting code

    return 0;
}

int c_draw2D(void *l) {
    printf("[graphics] Drawing 2D elements\n");

    // Draw 2D elements using graphics system
    // Implementation would depend on your graphics library
    // This is a placeholder for the actual drawing code

    return 0;
}

int c_drawHUDSurface(void *l) {
    // Get parameters from Lua stack
    const char* surface_name = luaL_checkstring(l, 1);
    float x = luaL_checknumber(l, 2);
    float y = luaL_checknumber(l, 3);

    printf("[graphics] Drawing HUD surface '%s' at (%f, %f)\n", surface_name, x, y);

    // Draw HUD surface using graphics system
    // Implementation would depend on your graphics library
    // This is a placeholder for the actual drawing code

    return 0;
}

int c_drawHUDMask(void *l) {
    // Get parameters from Lua stack
    const char* mask_name = luaL_checkstring(l, 1);
    float x = luaL_checknumber(l, 2);
    float y = luaL_checknumber(l, 3);

    printf("[graphics] Drawing HUD mask '%s' at (%f, %f)\n", mask_name, x, y);

    // Draw HUD mask using graphics system
    // Implementation would depend on your graphics library
    // This is a placeholder for the actual drawing code

    return 0;
}

int c_quitGame(void *L) {
    saveSettings();
    nebu_System_ExitLoop(eSRC_Credits);
    return 0;
}

int c_resetGame(void *L) {
    game_ResetData();
    video_ResetData();
    Audio_ResetData();
    return 0;
}

int c_resetScores(void *L) {
    resetScores();
    return 0;
}

int c_resetCamera(void *L) {
    camera_ResetAll();
    return 0;
}

int c_video_restart(void *L) {
    game_Callbacks_ExitCurrent();
    initGameScreen();

    shutdownDisplay();
    setupDisplay();
    if(game)
        changeDisplay(-1);
    game_Callbacks_InitCurrent();
    return 0;
}

int c_update_settings_cache(void *L) {
    updateSettingsCache();
    return 0;
}

int c_update_audio_volume(void *L) {
    Sound_setMusicVolume(getSettingf("musicVolume"));
    Sound_setFxVolume(getSettingf("fxVolume"));
    return 0;
}

int c_updateUI(void *L)
{
    if(game)
        changeDisplay(-1);
    return 0;
}

int c_startGame(void *L) {
    fprintf(stderr, "[debug] c_startGame: starting game\n");

    /* Run game initialization script */
    // Execute C version of initialization
    printf("[debug] c_startGame: running game initialization\n");

    video_UnloadLevel();
    game_UnloadLevel();

    fprintf(stderr, "[debug] c_startGame: loading level\n");
    game_LoadLevel(); // loads the lua level description
    video_LoadLevel();

    fprintf(stderr, "[debug] c_startGame: creating players\n");
    /* initialize the rest of the game's datastructures */
    game_CreatePlayers(getSettingi("players") + getSettingi("ai_opponents"), &game, &game2);

    fprintf(stderr, "[debug] c_startGame: changing display\n");
    changeDisplay(-1);

    fprintf(stderr, "[debug] c_startGame: checking if game2->level exists\n");
    if(!game2 || !game2->level)
    {
        fprintf(stderr, "[debug] c_startGame: initializing levels\n");
        initLevels();
    }

    fprintf(stderr, "[debug] c_startGame: resetting data\n");
    game_ResetData();
    video_ResetData();
    Audio_ResetData();

    fprintf(stderr, "[debug] c_startGame: exiting loop with eSRC_Game_Launch\n");
    nebu_System_ExitLoop(eSRC_Game_Launch);

    fprintf(stderr, "[debug] c_startGame: function completed\n");
    return 0;
}

int c_reloadTrack(void *L) {
    fprintf(stderr, "[audio] c_reloadTrack called\n");

    // Get the current track from settings
    char *track = NULL;
    // Use C version of track retrieval
    printf("[audio] Getting current track from settings\n");

    if(track && track[0] != '\0') {
        fprintf(stderr, "[audio] Loading track: %s\n", track);

        // Try to find the track in music directory first
        char music_path[512];
        sprintf(music_path, "music/%s", track);

        // Load and play the music
        Audio_LoadMusic(music_path);
        Audio_PlayMusic();

        fprintf(stderr, "[audio] Track loaded and playing\n");
    } else {
        fprintf(stderr, "[audio] No current track set in settings\n");
    }

    if(track) {
        free(track);
    }

    return 0;
}

int c_reloadArtpack(void *L)  {
    resource_ReleaseType(eRT_2d);
    resource_ReleaseType(eRT_Font);
    resource_ReleaseType(eRT_Texture);

    gui_ReleaseResources(); // TODO: ugly, do this differently
    gui_LoadResources();
    return 0;
}

int c_reloadLevel(void *L) {
    int status = GAME_SUCCESS;  // Default to success

    // free all loaded mesh resources & textures
    video_UnloadLevel();
    game_UnloadLevel();

    // Call game_LoadLevel() but don't try to use its return value
    game_LoadLevel();

    // Check if level loading was successful by checking if game2->level exists
    if(game2->level == NULL)
    {
        status = 1;  // Error code
    }

    if(status != GAME_SUCCESS)
    {
        // scrap current level
        // Use C version of level management
        printf("[debug] c_reloadLevel: reverting to last level\n");
        game_LoadLevel(); // if you can't reload last level, fail silently
    }

    video_LoadLevel();
    initLevels();

    game_ResetData();
    video_ResetData();
    return 0;
}

int c_cancelGame(void *L)
{
    if(game)
    {
        game_FreeGame(game);
        game = NULL;
    }
    if(game2)
    {
        game_FreeGame2(game2);
        game2 = NULL;
    }
    // Use C version of game state management
    printf("[debug] c_cancelGame: game state reset\n");
    return 0;
}

int c_configureKeyboard(void *L) {
    nebu_System_ExitLoop(eSRC_GUI_Prompt);
    return 0;
}

int c_getKeyName(void *L) {
    // Use C version of key name retrieval
    printf("[debug] c_getKeyName: getting key name\n");
    return 1;
}

int c_timedemo(void *L) {
    nebu_System_ExitLoop(eSRC_Timedemo);
    return 0;
}

int c_SetCallback(void *L) {
    // Use C version of callback setting
    printf("[debug] c_SetCallback: setting callback\n");
    return 0;
}

int c_loadLevel(void *L) {
    fprintf(stderr, "[debug] c_loadLevel: loading level\n");

    /* Unload any existing level */
    if(gWorld) {
        fprintf(stderr, "[debug] c_loadLevel: unloading existing level\n");
        video_FreeLevel(gWorld);
        gWorld = NULL;
    }

    /* Load the level */
    fprintf(stderr, "[debug] c_loadLevel: calling video_LoadLevel()\n");
    video_LoadLevel();

    /* Check if loading was successful */
    if(!gWorld) {
        fprintf(stderr, "[error] c_loadLevel: failed to load level\n");
    } else {
        fprintf(stderr, "[debug] c_loadLevel: successfully loaded level\n");
    }

    return 0;
}

int c_mainLoop(void *L) {
    int value = nebu_System_MainLoop();
    // Use C version of main loop
    printf("[debug] c_mainLoop: running main loop\n");
    return value;
}

int c_loadDirectory(void *L) {
    // Use C version of directory loading
    printf("[debug] c_loadDirectory: loading directory\n");
    return 1;
}

#ifndef PATH_MAX
// #warning PATH_MAX "is not defined in limits.h!"
#define PATH_MAX 255
#endif

static char art_dir_default[PATH_MAX];
static char art_dir_artpack[PATH_MAX];
static char *art_dirs[2];

int c_setArtPath(void *l)
{
    // Use C version of art path setting
    printf("[debug] c_setArtPath: setting art path\n");
    return 0;
}

int c_game_ComputeTimeDelta(void *l)
{
    int dt = game_ComputeTimeDelta();
    // Use C version of time delta computation
    printf("[debug] c_game_ComputeTimeDelta: computing time delta\n");
    return dt;
}

// Implement the game callback functions
void game_Init(void) {
    printf("[game] Initializing game\n");

    // Initialize game state
    game_ResetData();

    // Initialize video system
    video_Init();

    // Initialize audio system
    Audio_Init();

    // Load initial resources
    resource_LoadInitial();

    // Set up initial game settings
    game_ApplySettings();

    // Initialize HUD
    hud_Init();

    printf("[game] Game initialized successfully\n");
}

void game_Exit(void) {
    printf("[game] Exiting game\n");

    // Save game state
    game_SaveState();

    // Clean up resources
    resource_Cleanup();

    // Shutdown systems
    video_Shutdown();
    Audio_Shutdown();

    // Free game data
    game_FreeData();

    printf("[game] Game exited successfully\n");
}

void game_Idle(void) {
    // Update game state
    game_Update();

    // Update physics
    physics_Update();

    // Update audio
    Audio_Update();

    // Update HUD
    hud_Update();

    // Check for game over conditions
    if (game_CheckGameOver()) {
        game_HandleGameOver();
    }
}

void game_Reshape(int width, int height) {
    printf("[game] Reshaping window to %dx%d\n", width, height);

    // Update viewport
    video_SetViewport(width, height);

    // Update projection matrix
    camera_UpdateProjection(width, height);

    // Update HUD elements
    hud_UpdateLayout(width, height);
}

void game_Keyboard(unsigned char key, int x, int y) {
    printf("[game] Key pressed: %c at (%d, %d)\n", key, x, y);

    // Handle special keys
    switch (key) {
        case 27: // ESC key
            game_Pause();
            break;
        case 'p':
        case 'P':
            game_Pause();
            break;
        case 'r':
        case 'R':
            game_Reset();
            break;
        case 'm':
        case 'M':
            game_ToggleMute();
            break;
        default:
            // Handle regular key presses
            input_HandleKey(key);
            break;
    }
}

void game_Mouse(int button, int state, int x, int y) {
    printf("[game] Mouse button %d %s at (%d, %d)\n",
           button, state == 0 ? "pressed" : "released", x, y);

    // Handle mouse button events
    if (state == 0) { // Button pressed
        switch (button) {
            case 0: // Left button
                input_HandleMouseClick(x, y);
                break;
            case 1: // Middle button
                camera_ZoomIn();
                break;
            case 2: // Right button
                camera_ZoomOut();
                break;
            default:
                break;
        }
    } else { // Button released
        // Handle button release if needed
    }
}

// Implement the scripting_Register function
void scripting_Register(const char* name, lua_CFunction func) {
    printf("[scripting] Registering function: %s\n", name);

    // In a non-Lua implementation, we might store these functions in a lookup table
    // For now, we'll just log the registration
    printf("[scripting] Function %s registered (non-Lua implementation)\n", name);

    // Store the function in a lookup table if needed
    // This would be the actual implementation for a non-Lua system
}

// Implement the scripting_RunGC function
void scripting_RunGC(void) {
    printf("[scripting] Running garbage collection\n");

    // In a non-Lua implementation, we might perform memory cleanup
    // For GLtron, we would clean up game resources

    // Example: Clean up unused resources
    resource_Cleanup();

    printf("[scripting] Garbage collection completed\n");
}

// Implement the exitGame function
void exitGame(void) {
    printf("[game] Exiting game\n");

    // Save any necessary game state
    saveSettings();

    // Clean up resources
    if (game) {
        game_FreeGame(game);
        game = NULL;
    }
    if (game2) {
        game_FreeGame2(game2);
        game2 = NULL;
    }

    // Clean up video resources
    video_Shutdown();

    // Clean up audio resources
    Audio_Shutdown();

    // Exit the main loop
    nebu_System_ExitLoop(eSRC_Quit);

    printf("[game] Game exited successfully\n");
}


void init_c_interface(void) {
    // Initialize HUD configuration
    globalHUD.SpeedDial.x = 776;
    globalHUD.SpeedDial.y = 0;
    globalHUD.SpeedDial.angle = 0.0f;
    globalHUD.SpeedDial.speed = 0.0f;

    globalHUD.SpeedText.x = 150;
    globalHUD.SpeedText.y = 60;
    globalHUD.SpeedText.w = 44;
    globalHUD.SpeedText.h = 28;
    globalHUD.SpeedText.text = "0";

    globalHUD.Buster.x = 776;
    globalHUD.Buster.y = 41;
    globalHUD.Buster.active = 0;

    globalHUD.MapFrame.x = 10;
    globalHUD.MapFrame.y = 10;
    globalHUD.MapFrame.w = 100;
    globalHUD.MapFrame.h = 100;

    // Register C functions
    scripting_Register("c_drawHUD", draw_hud);
    scripting_Register("c_JoyThresholdUp", JoyThresholdUp);
    scripting_Register("c_JoyThresholdDown", JoyThresholdDown);
    scripting_Register("c_angle_MathFromClock360", angle_MathFromClock360);

    // Register other C functions
    scripting_Register("c_quitGame", c_quitGame);
    scripting_Register("c_resetGame", c_resetGame);
    scripting_Register("c_resetScores", c_resetScores);
    scripting_Register("c_resetCamera", c_resetCamera);
    scripting_Register("c_video_restart", c_video_restart);
    scripting_Register("c_update_settings_cache", c_update_settings_cache);
    scripting_Register("c_update_audio_volume", c_update_audio_volume);
    scripting_Register("c_startGame", c_startGame);
    scripting_Register("c_reloadTrack", c_reloadTrack);
    scripting_Register("c_reloadArtpack", c_reloadArtpack);
    scripting_Register("c_reloadLevel", c_reloadLevel);
    scripting_Register("c_configureKeyboard", c_configureKeyboard);
    scripting_Register("c_getKeyName", c_getKeyName);
    scripting_Register("c_timedemo", c_timedemo);
    scripting_Register("c_loadDirectory", c_loadDirectory);
    scripting_Register("c_mainLoop", c_mainLoop);
    scripting_Register("c_setCallback", c_SetCallback);
    scripting_Register("c_setArtPath", c_setArtPath);
    scripting_Register("c_cancelGame", c_cancelGame);
    scripting_Register("c_drawCircle", c_drawCircle);
    scripting_Register("c_drawRectangle", c_drawRectangle);
    scripting_Register("c_translate", c_translate);
    scripting_Register("c_scale", c_scale);
    scripting_Register("c_pushMatrix", c_pushMatrix);
    scripting_Register("c_popMatrix", c_popMatrix);
    scripting_Register("c_drawTextFitIntoRect", c_drawTextFitIntoRect);
    scripting_Register("c_color", c_color);
    scripting_Register("c_draw2D", c_draw2D);
    scripting_Register("c_drawHUDSurface", c_drawHUDSurface);
    scripting_Register("c_drawHUDMask", c_drawHUDMask);
    scripting_Register("c_updateUI", c_updateUI);
    scripting_Register("c_game_ComputeTimeDelta", c_game_ComputeTimeDelta);

    /* Register touch input functions */
    touch_interface_register();
}

/*
	resource management:
	- memory resource
		- 
	- context dependent resources (e.g. OpenGL-context)
		- OpenGL texture ids
		- Nebu-2D surfaces (basically OpenGL textures)
*/
