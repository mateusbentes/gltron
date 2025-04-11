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

#include "lua.h"
#include "lualib.h"

/* External declaration for touch interface registration */
extern void touch_interface_register(void);

/* Define GAME_SUCCESS for backward compatibility */
#define GAME_SUCCESS 0

// some functions defined elsewhere: graphics_hud.c
int c_drawRectangle(lua_State *l);
int c_drawCircle(lua_State *l);
int c_translate(lua_State *l);
int c_scale(lua_State *l);
int c_pushMatrix(lua_State *l);
int c_popMatrix(lua_State *l);
int c_drawTextFitIntoRect(lua_State *l);
int c_color(lua_State *l);
int c_draw2D(lua_State* l);
int c_drawHUDSurface(lua_State* l);
int c_drawHUDMask(lua_State* l);

int c_quitGame(lua_State *L) {
	saveSettings();
	nebu_System_ExitLoop(eSRC_Credits);
	return 0;
}

int c_resetGame(lua_State *L) {
	game_ResetData();
	video_ResetData();
	Audio_ResetData();
	return 0;
}

int c_resetScores(lua_State *L) {
	resetScores();
	return 0;
}

int c_resetCamera(lua_State *L) {
	camera_ResetAll();
	return 0;
}

int c_video_restart(lua_State *L) {
	game_Callbacks_ExitCurrent();
	initGameScreen();
	
	shutdownDisplay();
	setupDisplay();
	if(game)
		changeDisplay(-1);
	game_Callbacks_InitCurrent();
	return 0;
}

int c_update_settings_cache(lua_State *L) {
	updateSettingsCache();
	return 0;
}

int c_update_audio_volume(lua_State *L) { 
	Sound_setMusicVolume(getSettingf("musicVolume"));
	Sound_setFxVolume(getSettingf("fxVolume"));
	return 0;
}

int c_updateUI(lua_State *L)
{
	if(game)
		changeDisplay(-1);
	return 0;
}

int c_startGame(lua_State *L) { 
    fprintf(stderr, "[debug] c_startGame: starting game\n");
    
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

int c_reloadTrack(lua_State *L) {
    fprintf(stderr, "[audio] c_reloadTrack called\n");
    
    // Get the current track from settings
    char *track = NULL;
    scripting_GetGlobal("settings", "current_track", NULL);
    scripting_GetStringResult(&track);
    
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
        scripting_StringResult_Free(track);
    }
    
    return 0;
}

int c_reloadArtpack(lua_State *L)  {
	resource_ReleaseType(eRT_2d);
	resource_ReleaseType(eRT_Font);
	resource_ReleaseType(eRT_Texture);

	gui_ReleaseResources(); // TODO: ugly, do this differently
	gui_LoadResources();
	return 0;
}

int c_reloadLevel(lua_State *L) {
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
		scripting_Run("levels[ current_level_index] = \"\"");
		// and revert to last level
		scripting_Run("current_level_index = last_level_index");
		scripting_Run("settings.current_level = levels[ current_level_index ]");
		game_LoadLevel(); // if you can't reload last level, fail silently
	}

	video_LoadLevel();
	initLevels();

	game_ResetData();
	video_ResetData();
	return 0;
}

int c_cancelGame(lua_State *L)
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
	scripting_Run("game_initialized = 0");
	return 0;
}
  
int c_configureKeyboard(lua_State *L) {
	nebu_System_ExitLoop(eSRC_GUI_Prompt);
	return 0;
}

int c_getKeyName(lua_State *L) {
	int top = lua_gettop(L);
	if(lua_isnumber(L, top)) {
		lua_pushstring(L, nebu_Input_GetKeyname( (int) lua_tonumber(L, top) ));
	} else {
		lua_pushstring(L, "error");
	}
	return 1;
}

int c_timedemo(lua_State *L) {
	nebu_System_ExitLoop(eSRC_Timedemo);
	return 0;
}

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
    
    /* Convert string to callback type directly */
    CallbackType type;
    
    if(strcmp(name, "game") == 0) {
        type = CB_GAME;
    } else if(strcmp(name, "gui") == 0) {
        type = CB_GUI;
    } else if(strcmp(name, "pause") == 0) {
        type = CB_PAUSE;
    } else if(strcmp(name, "credits") == 0) {
        type = CB_CREDITS;
    } else if(strcmp(name, "configure") == 0) {
        type = CB_CONFIGURE;
    } else if(strcmp(name, "timedemo") == 0) {
        type = CB_TIMEDEMO;
    } else {
        fprintf(stderr, "[warning] c_SetCallback: unknown callback name '%s', defaulting to GUI\n", name);
        type = CB_GUI;
    }
    
    /* Call setCallbackByType directly with the numeric type */
    fprintf(stderr, "[debug] c_SetCallback: calling setCallbackByType with type %d\n", type);
    setCallbackByType(type);
    
    return 0;
}

int c_loadLevel(lua_State *L) {
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

int c_mainLoop(lua_State *L) {
	int value = nebu_System_MainLoop();
	lua_pushnumber(L, value);
	return 1;
}

int c_loadDirectory(lua_State *L) {
	int dir;
	const char *dirPath;
	nebu_List *files, *p;
	int nFiles = 0;

	// load directory enum from stack
	int top = lua_gettop(L); // number of arguments
	if(top != 1) {
		// wrong number of arguments
		// lua_error(L, "wrong number of arguments for function "
		//	"c_loadDirectory: should be 1\n");
	}
	if(!lua_isnumber(L, -1)) {
		// lua_error(L, "number  expected for arg1 to function "
		//	"c_loadDirecotry");
	}
	dir = (int) lua_tonumber(L, -1);

	dirPath = getDirectory(dir); // PATH_ART or PATH_LEVEL or PATH_MUSIC
	files = readDirectoryContents(dirPath, NULL);

	lua_newtable(L);
	for(p = files; p->next; p = p->next) {
		lua_pushstring(L, p->data);
		lua_rawseti(L, -2, nFiles + 1);
		nFiles++;
		free(p->data);
	}
	nebu_List_Free(files);
	return 1;
}

#ifndef PATH_MAX
// #warning PATH_MAX "is not defined in limits.h!"
#define PATH_MAX 255
#endif

static char art_dir_default[PATH_MAX];
static char art_dir_artpack[PATH_MAX];
static char *art_dirs[2];

int c_setArtPath(lua_State *l)
{
	char *artpack;
	scripting_GetGlobal("settings", "current_artpack", NULL);
	scripting_GetStringResult(&artpack);
	fprintf(stderr, "[status] loading artpack '%s'\n", artpack);

	sprintf(art_dir_default, "%s%c%s", getDirectory(PATH_ART), SEPARATOR, "default");
	sprintf(art_dir_artpack, "%s%c%s", getDirectory(PATH_ART), SEPARATOR, artpack);

	scripting_StringResult_Free(artpack);

	art_dirs[0] = art_dir_artpack;
	art_dirs[1] = art_dir_default;

	// I don't understand why the explicit cast is required to
	// keep the compiler happy
	nebu_FS_SetupPath_WithDirs(PATH_ART, 2, (const char**) art_dirs);
	return 0;
}

int c_game_ComputeTimeDelta(lua_State *l)
{
	int dt = game_ComputeTimeDelta();
	scripting_PushInteger(dt);
	return 1;
}

void init_c_interface(void) {
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
