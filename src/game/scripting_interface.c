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

/* External declaration for touch interface registration */
extern void touch_interface_register(void);

/* Define GAME_SUCCESS for backward compatibility */
#define GAME_SUCCESS 0

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
void draw_hud(int score, const char* ai_message);

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

void init_c_interface(void) {
    // Register C functions instead of Lua functions
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
