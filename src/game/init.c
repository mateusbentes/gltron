#include "filesystem/path.h"
#include "filesystem/dirsetup.h"
#include "game/engine.h"
#include <stdio.h>
#include "game/init.h"
#include "game/gltron.h"
#include "game/game.h"
#include "game/camera.h"
#include <string.h> /* For memset */
#include "game/resource.h"
#include "base/nebu_resource.h"
#include "input/input.h"
#include "base/util.h"
#include "scripting/scripting.h"
#include "scripting/embedded_processing.h"  /* Added for embedded script processing */
#include "base/nebu_system.h"
#include "audio/audio.h"
#include "audio/sound_glue.h"  /* Added sound_glue.h for Audio_* functions */
#include "audio/nebu_audio_system.h"
#include "configuration/settings.h"
#include "configuration/configuration.h"
#include "configuration/platform_settings.h"  /* Added platform settings header */
#include "video/nebu_video_system.h"
#include "filesystem/nebu_filesystem.h"
#include "scripting/nebu_scripting.h"
#include "input/nebu_input_system.h"
#include "video/video.h"  /* This should define PlayerVisual */

#include <stdlib.h>
#include <string.h>  /* Added for strncpy */

#include "base/nebu_assert.h"

/* Now that we've included all necessary headers, we can declare the function prototype */
void initPlayerCamera(PlayerVisual *pV, int type);

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include "scripting/lua_compat.h"

// Define LUA_OK for Lua 5.1 compatibility
#ifndef LUA_OK
#define LUA_OK 0
#endif

static lua_State *lua_state = NULL;

/* Define DEBUG_SCRIPTING - set to 0 for production, 1 for debugging */
#define DEBUG_SCRIPTING 0

// External function declarations
extern int isMenuActive(void);

/* Define current_callback - used by l_setCallback and l_mainLoop */
static char current_callback[256] = "gui";  /* Default callback is "gui" */

void initFilesystem(int argc, const char *argv[]);
void debug_print_paths(void);
void initGUIs(void);
void initGame(void);
void initGame2(void);
void initPlayers(void);
void initEnterGame(void);
void initExitGame(void);
void initSubsystems(int argc, const char *argv[]);

/* Debug function to print out the current paths */
void debug_print_paths(void) {
    printf("[debug] Current paths:\n");
    printf("[debug] PATH_PREFERENCES: %s\n", getDirectory(PATH_PREFERENCES));
    printf("[debug] PATH_SNAPSHOTS: %s\n", getDirectory(PATH_SNAPSHOTS));
    printf("[debug] PATH_DATA: %s\n", getDirectory(PATH_DATA));
    printf("[debug] PATH_SCRIPTS: %s\n", getDirectory(PATH_SCRIPTS));
    printf("[debug] PATH_MUSIC: %s\n", getDirectory(PATH_MUSIC));
    printf("[debug] PATH_ART: %s\n", getDirectory(PATH_ART));
    printf("[debug] PATH_LEVEL: %s\n", getDirectory(PATH_LEVEL));
}

void exitSubsystems(void)
{
    // Skip Sound_shutdown() to prevent segmentation fault
    printf("[audio] Skipping Sound_shutdown to prevent segmentation fault\n");

    // Audio_Quit();  /* Commented out to prevent segmentation fault */
    printf("[audio] Skipping Audio_Quit to prevent segmentation fault\n");

    freeVideoData();

    if(gWorld)
        video_FreeLevel(gWorld);
    gWorld = NULL;

    if(game)
        game_FreeGame(game);
    game = NULL;

    if(game2)
        game_FreeGame2(game2);
    game2 = NULL;

    // Skip scripting_Quit() to prevent segmentation fault
    printf("[scripting] Skipping scripting_Quit() to prevent segmentation fault\n");

    nebu_FS_ClearAllPaths();
    resource_FreeAll();
    resource_Shutdown();
}

// Initialize the game
void initGame(void) {
    printf("[init] Initializing game\n");

    // Initialize game data structures
    printf("[init] Initializing game data structures\n");
    game = (Game*)malloc(sizeof(Game));
    if (!game) {
        fprintf(stderr, "[init] Failed to allocate memory for game\n");
        exit(EXIT_FAILURE);
    }
    memset(game, 0, sizeof(Game));

    game2 = (Game2*)malloc(sizeof(Game2));
    if (!game2) {
        fprintf(stderr, "[init] Failed to allocate memory for game2\n");
        exit(EXIT_FAILURE);
    }
    memset(game2, 0, sizeof(Game2));

    // Initialize game settings
    printf("[init] Initializing game settings\n");
    updateSettingsCache();

    // Initialize game level
    printf("[init] Initializing game level\n");
    game2->level = (game_level*)malloc(sizeof(game_level));
    if (!game2->level) {
        fprintf(stderr, "[init] Failed to allocate memory for level\n");
        exit(EXIT_FAILURE);
    }
    memset(game2->level, 0, sizeof(game_level));

    // Set up level boundaries
    game2->level->boundingBox.vMin.v[0] = -100.0f;
    game2->level->boundingBox.vMin.v[1] = -100.0f;
    game2->level->boundingBox.vMax.v[0] = 100.0f;
    game2->level->boundingBox.vMax.v[1] = 100.0f;

    // Initialize world
    printf("[init] Initializing world\n");
    gWorld = NULL;  // Will be initialized by video_LoadLevel

    // Initialize players
    printf("[init] Initializing players\n");
    initPlayers();

    // Initialize camera
    printf("[init] Initializing camera\n");
    // Done inside initPlayers

    printf("[init] Initializing scripting\n");

#ifdef USE_SCRIPTING   
    initScripting();
#endif

    
    
    // Set game state
    printf("[init] Setting game state\n");
    game2->time.current = 0;
    game2->time.lastFrame = 0;
    game2->time.dt = 0;

    game->pauseflag = PAUSE_GAME_RUNNING;
    game2->play = 1;

    printf("[init] Game initialized\n");
}

// Enter game mode
void initEnterGame(void) {
    printf("[init] Entering game mode\n");

    // Update settings cache
    updateSettingsCache();

    // Hide mouse pointer
    // Use nebu_Input_HidePointer only if it's defined
#ifdef HAVE_NEBU_INPUT_HIDEPOINTER
    nebu_Input_HidePointer();
#else
    // Fallback if the function isn't available
    printf("[init] nebu_Input_HidePointer not available\n");
#endif

    // Reset game time
    game2->time.offset = nebu_Time_GetElapsed() - game2->time.current;

    // Enable audio
    Audio_EnableEngine();

    // Disable booster & wallbuster for all players
    for (int i = 0; i < game->players; i++) {
        game->player[i].data.boost_enabled = 0;
        game->player[i].data.wall_buster_enabled = 0;
    }

    printf("[init] Game mode entered\n");
}

// Exit game mode
void initExitGame(void) {
    printf("[init] Exiting game mode\n");

    // Disable audio
    Audio_DisableEngine();

    // Show mouse pointer
    // Use nebu_Input_ShowPointer only if it's defined
#ifdef HAVE_NEBU_INPUT_SHOWPOINTER
    nebu_Input_ShowPointer();
#else
    // Alternative implementation if nebu_Input_ShowPointer is not available
    printf("[init] nebu_Input_ShowPointer not available\n");
#endif

    printf("[init] Game mode exited\n");
}

void initSubsystems(int argc, const char *argv[]) {

    // Initialize Nebu system
    nebu_Init();

    resource_Init();

    initFilesystem(argc, argv);
    
    // Make sure initScripting() is called before initVideo()
    printf("[init] Initializing scripting system\n");

#ifdef USE_SCRIPTING
    initScripting();
#endif

    /* Initialize platform-specific settings before general configuration */
    platform_InitSettings();
    
    initConfiguration(argc, argv);
    initArtpacks();
    initGUIs();
    
    // Initialize game structure first
    initGame();
    
    // Initialize game2 structure
    initGame2();

    // Initialize video
    initVideo();

    // Initialize audio
    initAudio();
    
    // Initialize input
    initInput();
    
    // Initialize players
    initPlayers();

    fprintf(stderr, "[status] done loading level...\n");
}


void initFilesystem(int argc, const char *argv[]) {
	printf("[debug] Initializing filesystem with argv[0]: %s\n", argv[0]);
	
	/* Set up directories based on the executable path */
	dirSetup(argv[0]);
	
	/* Print out the current paths for debugging */
	debug_print_paths();
	
	// FIXME: why should argc be 1
	nebu_assert(argc == 1);
}

/* Set the current callback */
int l_setCallback(lua_State *L) {
    const char *callback = luaL_checkstring(L, 1);
    printf("[c] Setting callback: %s\n", callback);
    
    /* Store the callback for later use */
    strncpy(current_callback, callback, sizeof(current_callback) - 1);
    current_callback[sizeof(current_callback) - 1] = '\0';
    
    return 0;  /* No return values */
}

/* Run the main loop */
int l_mainLoop(lua_State *L) {
    int status = 0;
    
    printf("[c] Running main loop with callback: %s\n", current_callback);
    
    /* Run the appropriate callback */
    if (strcmp(current_callback, "gui") == 0) {
        status = runGUI();
    } else if (strcmp(current_callback, "game") == 0) {
        status = runGame();
    } else if (strcmp(current_callback, "pause") == 0) {
        status = runPause();
    } else if (strcmp(current_callback, "credits") == 0) {
        status = runCredits();
    } else if (strcmp(current_callback, "configure") == 0) {
        status = runConfigure();
    } else if (strcmp(current_callback, "timedemo") == 0) {
        status = runTimedemo();
    } else {
        printf("[c] Unknown callback: %s\n", current_callback);
        status = 0;  /* Quit */
    }
    
    /* Return the status to Lua */
    lua_pushnumber(L, status);
    return 1;  /* One return value */
}

/* Helper function to load and execute scripts */
int load_and_execute_script(lua_State *L, const char* script_name, const char* script_content) {
    printf("[scripting] Loading script: %s\n", script_name);
    
    // Validate inputs
    if (!L || !script_name || !script_content) {
        fprintf(stderr, "[FATAL] Invalid parameters for script loading\n");
        return 0;
    }
    
    // Check script content length
    size_t len = strlen(script_content);
    if (len == 0) {
        fprintf(stderr, "[FATAL] Script %s has zero length\n", script_name);
        return 0;
    }
    
    // Ensure stack has enough space
    if (!safe_lua_checkstack(L, 5)) {
        fprintf(stderr, "[FATAL] Cannot grow Lua stack for script loading\n");
        return 0;
    }
    
    // Load the script
    int load_status = safe_luaL_loadbuffer(L, script_content, len, script_name);
    if (load_status != LUA_OK) {
        fprintf(stderr, "[FATAL] Failed to load %s: %s\n", 
                script_name, lua_tostring(L, -1));
        safe_lua_pop(L, 1);  // Pop error message
        return 0;
    }
    
    // Execute the script
    int exec_status = safe_lua_pcall(L, 0, 0, 0);
    if (exec_status != LUA_OK) {
        fprintf(stderr, "[FATAL] Failed to execute %s: %s\n", 
                script_name, lua_tostring(L, -1));
        safe_lua_pop(L, 1);  // Pop error message
        return 0;
    }
    
    printf("[scripting] Successfully loaded and executed: %s\n", script_name);
    return 1;  // Success
}

void initScripting(void) {

#ifdef USE_SCRIPTING
    scripting_Init(NEBU_SCRIPTING_DEBUG);
    init_c_interface();

    /* load basic scripting services */
    runScript(PATH_SCRIPTS, "basics.lua");
    runScript(PATH_SCRIPTS, "joystick.lua");
    runScript(PATH_SCRIPTS, "path.lua");

    runScript(PATH_SCRIPTS, "video.lua");

    runScript(PATH_SCRIPTS, "console.lua");
#endif

}

void initConfiguration(int argc, const char *argv[])
{
  printf("[init] Initializing configuration\n");

#ifndef USE_SCRIPTING
    printf("[init] Scripting is disabled, skipping Lua setup\n");
    return;

#else
  /* load some more defaults from config file */
  runScript(PATH_SCRIPTS, "config.lua");
  runScript(PATH_SCRIPTS, "artpack.lua");
#endif
  
  /* go for .gltronrc (or whatever is defined in RC_NAME) */
  {
    char *path;
    path = getPossiblePath(PATH_PREFERENCES, RC_NAME);
    if (path != NULL) {
      if (nebu_FS_Test(path)) {
        printf("[status] loading settings from %s\n", path);
        /* CHANGE: Don't call scripting_RunFile, just print a message */
        printf("[scripting] Would run script file: %s (stub)\n", path);
      } else {
        printf("[error] cannot load %s from %s\n", RC_NAME, path);
      }
      free(path);
    }
    else {
      printf("[fatal] can't get valid pref path for %s\n", RC_NAME);
      nebu_assert(0); exit(1); // something is seriously wrong
    }
  }
  
  // CHANGE: Replace Lua-dependent version check with a stub
  printf("[scripting] Skipping version check (stub)\n");
  
  // CHANGE: Replace Lua-dependent config validation with a stub
  printf("[scripting] Skipping config validation (stub)\n");
  
  /* parse any comandline switches overrinding the loaded settings */
  parse_args(argc, argv);
  
  /* sanity check some settings */
  checkSettings();
  
  // CHANGE: Replace scripting_Run calls with stubs
  printf("[scripting] Would run: setupArtpackPaths() (stub)\n");
  printf("[scripting] Would run: setupLevels() (stub)\n");
    
  /* intialize the settings cache, remember to do that everytime you
     change something */
  updateSettingsCache();
}

void initAudio(void) {
    int audio_available = 1;

    fprintf(stderr, "[audio] Initializing audio system\n");

    /* Initialize the audio system */
    fprintf(stderr, "[audio] Calling Audio_Init()\n");
    Audio_Init();

    fprintf(stderr, "[audio] Calling Audio_Start()\n");
    Audio_Start();

    /* Load audio scripts with error checking */
    fprintf(stderr, "[audio] Loading audio scripts\n");

#ifdef USE_SCRIPTING
    // Parte relacionada ao uso de Lua
    /* Exemplo de como carregar os scripts Lua, se necessário */
    char *audio_script = getPossiblePath(PATH_SCRIPTS, "audio.lua");
    if (audio_script && nebu_FS_Test(audio_script)) {
        fprintf(stderr, "[audio] Loading audio.lua from: %s\n", audio_script);
        runScript(PATH_SCRIPTS, "audio.lua");
        free(audio_script);
    } else {
        fprintf(stderr, "[error] Failed to load audio.lua\n");
        if (audio_script) free(audio_script);
        audio_available = 0;
    }

    char *music_functions_script = getPossiblePath(PATH_SCRIPTS, "music_functions.lua");
    if (music_functions_script && nebu_FS_Test(music_functions_script)) {
        fprintf(stderr, "[audio] Loading music_functions.lua from: %s\n", music_functions_script);
        runScript(PATH_SCRIPTS, "music_functions.lua");
        free(music_functions_script);
    } else {
        fprintf(stderr, "[error] Failed to load music_functions.lua\n");
        if (music_functions_script) free(music_functions_script);

        /* CHANGE: Would create nextTrack and previousTrack functions in Lua (stub) */
        fprintf(stderr, "[audio] Would create nextTrack and previousTrack functions (stub)\n");
    }
#else
    fprintf(stderr, "[audio] Scripting is disabled, loading audio manually\n");

    // Load crash sound
    char *crash_sound = getPossiblePath(PATH_DATA, "game_crash.wav");
    if (crash_sound && nebu_FS_Test(crash_sound)) {
        fprintf(stderr, "[audio] Loading crash sound from: %s\n", crash_sound);
        Audio_LoadSample(crash_sound, 1);
        free(crash_sound);
    } else {
        fprintf(stderr, "[error] Failed to load crash sound\n");
        if (crash_sound) free(crash_sound);
        audio_available = 0;
    }

    char *engine_sound = getPossiblePath(PATH_DATA, "game_engine.wav");
    if (engine_sound && nebu_FS_Test(engine_sound)) {
        fprintf(stderr, "[audio] Loading engine sound from: %s\n", engine_sound);
        Audio_LoadSample(engine_sound, 0);
        free(engine_sound);
    } else {
        fprintf(stderr, "[error] Failed to load engine sound\n");
        if (engine_sound) free(engine_sound);
        audio_available = 0;
    }

    char *recognizer_sound = getPossiblePath(PATH_DATA, "game_recognizer.wav");
    if (recognizer_sound && nebu_FS_Test(recognizer_sound)) {
        fprintf(stderr, "[audio] Loading recognizer sound from: %s\n", recognizer_sound);
        Audio_LoadSample(recognizer_sound, 2);
        free(recognizer_sound);
    } else {
        fprintf(stderr, "[error] Failed to load recognizer sound\n");
        if (recognizer_sound) free(recognizer_sound);
        audio_available = 0;
    }

    /* Set up audio volumes with error checking */
    fprintf(stderr, "[audio] Setting audio volumes\n");

    if (isSetting("fxVolume")) {
        float volume = getSettingf("fxVolume");
        fprintf(stderr, "[audio] Setting FX volume to: %f\n", volume);
        Audio_SetFxVolume(volume);
    } else {
        fprintf(stderr, "[warning] fxVolume setting not found, using default\n");
        Audio_SetFxVolume(0.8f);
    }

    if (isSetting("musicVolume")) {
        float volume = getSettingf("musicVolume");
        fprintf(stderr, "[audio] Setting music volume to: %f\n", volume);
        Audio_SetMusicVolume(volume);
    } else {
        fprintf(stderr, "[warning] musicVolume setting not found, using default\n");
        Audio_SetMusicVolume(0.8f);
    }

    /* Try to load and play music with error checking */
    fprintf(stderr, "[audio] Checking if music is enabled\n");

    if (audio_available && isSetting("playMusic") && getSettingi("playMusic")) {
        fprintf(stderr, "[audio] Music is enabled, trying to play\n");

        // Play music
        char *music_path = getPossiblePath(PATH_MUSIC, "song_revenge_of_cats.it");
        if (music_path && nebu_FS_Test(music_path)) {
            fprintf(stderr, "[audio] Loading and playing music from: %s\n", music_path);
            Audio_LoadMusic(music_path);
            Audio_PlayMusic();
            free(music_path);
        } else {
            fprintf(stderr, "[error] Failed to load music\n");
            audio_available = 0;
        }
    } else {
        fprintf(stderr, "[audio] Music is disabled\n");
    }

#endif

    fprintf(stderr, "[audio] Audio initialization complete\n");
}

void initVideo(void) {
    nebu_Video_Init();
    // this requires the player data
    initVideoData();
    setupDisplay();

    loadArt();
    loadModels();
    
    // Add this line to initialize the game world
    video_LoadLevel();
}
	
void initGUIs(void)
{
#ifndef USE_SCRIPTING
    menu_functions();
    menu();

    // Call hud with appropriate parameters
    hud(0, "");  // Initialize with score 0 and empty AI message

    gauge();

    // Android touch configuration
#if defined(__ANDROID__)
    android_touch();
#endif
#else
    // menu
    runScript(PATH_SCRIPTS, "menu_functions.lua");
    runScript(PATH_SCRIPTS, "menu.lua");

    // hud stuff
    runScript(PATH_SCRIPTS, "hud-config.lua");
    runScript(PATH_SCRIPTS, "hud.lua");
    runScript(PATH_SCRIPTS, "gauge.lua");

    // Android touch configuration
#if defined(__ANDROID__)
    runScript(PATH_SCRIPTS, "android_touch.lua");
#endif
#endif
}

void initInput(void) {
	nebu_Input_Init();

	gInput.mouse1 = 0;
	gInput.mouse2 = 0;
}

void initGame2(void) {
    printf("[init] Initializing game2\n");
    
    // Allocate memory for the game2 structure
    game2 = (Game2*) malloc(sizeof(Game2));
    if (!game2) {
        fprintf(stderr, "[init] Failed to allocate memory for game2\n");
        exit(EXIT_FAILURE);
    }
    
    // Initialize the game2 structure
    memset(game2, 0, sizeof(Game2));
    
    // Initialize minimal game state
    game2->level = (game_level*) malloc(sizeof(game_level));
    if (!game2->level) {
        fprintf(stderr, "[init] Failed to allocate memory for game2->level\n");
        free(game2);
        exit(EXIT_FAILURE);
    }
    memset(game2->level, 0, sizeof(game_level));
    
    // Set up bounding box
    game2->level->boundingBox.vMin.v[0] = -100.0f;
    game2->level->boundingBox.vMin.v[1] = -100.0f;
    game2->level->boundingBox.vMin.v[2] = 0.0f;
    game2->level->boundingBox.vMax.v[0] = 100.0f;
    game2->level->boundingBox.vMax.v[1] = 100.0f;
    game2->level->boundingBox.vMax.v[2] = 10.0f;
    
    // Set default time values
    game2->time.dt = 0.0f;
    game2->time.lastFrame = 0;
    game2->time.current = 0;
    game2->time.offset = 0;
    
    // Initialize game2->play
    game2->play = 1;  // Set to 1 to indicate that the game is playing
    
    printf("[init] game2 initialized successfully\n");
}

void initPlayerCamera(PlayerVisual *pV, int type) {
    Camera *c = &pV->camera;
    Player *p = pV->pPlayer;
    
    // Initialize camera based on type
    // type 0 = follow camera
    
    // Set camera target to player position
    c->target[0] = p->data.posx;
    c->target[1] = p->data.posy;
    c->target[2] = 0.0f;
    
    // Get direction from player
    int direction = p->data.dir;
    
    // Set camera position based on player direction
    switch (direction) {
        case 0:  // Right
            c->cam[0] = c->target[0] - 20.0f;
            c->cam[1] = c->target[1];
            c->cam[2] = 10.0f;
            break;
        case 1:  // Up
            c->cam[0] = c->target[0];
            c->cam[1] = c->target[1] - 20.0f;
            c->cam[2] = 10.0f;
            break;
        case 2:  // Left
            c->cam[0] = c->target[0] + 20.0f;
            c->cam[1] = c->target[1];
            c->cam[2] = 10.0f;
            break;
        case 3:  // Down
            c->cam[0] = c->target[0];
            c->cam[1] = c->target[1] + 20.0f;
            c->cam[2] = 10.0f;
            break;
    }
    
    printf("[init] Initialized camera at (%f, %f, %f) targeting (%f, %f, %f)\n", 
           c->cam[0], c->cam[1], c->cam[2], c->target[0], c->target[1], c->target[2]);
}

void initPlayers(void) {
    printf("[init] Initializing players\n");

    // Check if game is NULL
    if (!game) {
        printf("[init] game is NULL, cannot initialize players\n");
        return;
    }

    // Set up player count
    int player_count = 4;  // Default to 4 players (1 human, 3 AI)
    printf("[init] Setting up %d players\n", player_count);

    // Set player count
    game->players = player_count;

    // Allocate memory for players
    game->player = (Player*) malloc(player_count * sizeof(Player));
    if (!game->player) {
        fprintf(stderr, "[init] Failed to allocate memory for players\n");
        return;
    }
    memset(game->player, 0, player_count * sizeof(Player));

    // Initialize each player
    for (int i = 0; i < player_count; i++) {
        // Set player type (0 = human, 1 = AI)
        // AI is a struct, not an int, so we need to initialize it properly
        game->player[i].ai.active = (i == 0) ? 0 : 1;  // Set active field to 0 for human, 1 for AI
        game->player[i].ai.tdiff = 0;
        game->player[i].ai.lasttime = 0;

        // Initialize player data
        game->player[i].data.speed = 10.0f;
        game->player[i].data.trail_height = 3.5f;
        game->player[i].data.exp_radius = 0.0f;
        game->player[i].data.turn_time = 0;
        game->player[i].data.last_dir = -1;
        game->player[i].data.dir = i;  // Each player starts facing a different direction
        game->player[i].data.boost_enabled = 1;
        game->player[i].data.wall_buster_enabled = 0;

        // Set player position based on direction
        float pos_x = 0.0f;
        float pos_y = 0.0f;

        switch (i) {
            case 0:  // Player 1 (bottom left)
                pos_x = -50.0f;
                pos_y = -50.0f;
                break;
            case 1:  // Player 2 (bottom right)
                pos_x = 50.0f;
                pos_y = -50.0f;
                break;
            case 2:  // Player 3 (top right)
                pos_x = 50.0f;
                pos_y = 50.0f;
                break;
            case 3:  // Player 4 (top left)
                pos_x = -50.0f;
                pos_y = 50.0f;
                break;
        }

        // Set player position in the data structure
        game->player[i].data.posx = pos_x;
        game->player[i].data.posy = pos_y;

        // Initialize player colors
        switch (i) {
            case 0:  // Player 1 (blue)
                game->player[i].profile.pColorDiffuse[0] = 0.0f;
                game->player[i].profile.pColorDiffuse[1] = 0.0f;
                game->player[i].profile.pColorDiffuse[2] = 1.0f;
                game->player[i].profile.pColorDiffuse[3] = 1.0f;
                break;
            case 1:  // Player 2 (red)
                game->player[i].profile.pColorDiffuse[0] = 1.0f;
                game->player[i].profile.pColorDiffuse[1] = 0.0f;
                game->player[i].profile.pColorDiffuse[2] = 0.0f;
                game->player[i].profile.pColorDiffuse[3] = 1.0f;
                break;
            case 2:  // Player 3 (green)
                game->player[i].profile.pColorDiffuse[0] = 0.0f;
                game->player[i].profile.pColorDiffuse[1] = 1.0f;
                game->player[i].profile.pColorDiffuse[2] = 0.0f;
                game->player[i].profile.pColorDiffuse[3] = 1.0f;
                break;
            case 3:  // Player 4 (yellow)
                game->player[i].profile.pColorDiffuse[0] = 1.0f;
                game->player[i].profile.pColorDiffuse[1] = 1.0f;
                game->player[i].profile.pColorDiffuse[2] = 0.0f;
                game->player[i].profile.pColorDiffuse[3] = 1.0f;
                break;
        }

        // Copy diffuse color to specular color with reduced intensity
        for (int j = 0; j < 3; j++) {
            game->player[i].profile.pColorSpecular[j] = game->player[i].profile.pColorDiffuse[j] * 0.5f;
        }
        game->player[i].profile.pColorSpecular[3] = 1.0f;

        printf("[init] Initialized player %d at position (%f, %f)\n",
               i, game->player[i].data.posx, game->player[i].data.posy);
    }

    // Initialize player visuals
    printf("[init] Initializing player visuals\n");

    // Allocate memory for player visuals
    gppPlayerVisuals = (PlayerVisual**) malloc(player_count * sizeof(PlayerVisual*));
    if (!gppPlayerVisuals) {
        fprintf(stderr, "[init] Failed to allocate memory for player visuals\n");
        return;
    }
    memset(gppPlayerVisuals, 0, player_count * sizeof(PlayerVisual*));

    // Set viewport type (1 = split screen)
    gViewportType = 1;

    // Initialize each player visual
    for (int i = 0; i < player_count; i++) {
        // Allocate memory for player visual
        gppPlayerVisuals[i] = (PlayerVisual*) malloc(sizeof(PlayerVisual));
        if (!gppPlayerVisuals[i]) {
            fprintf(stderr, "[init] Failed to allocate memory for player visual %d\n", i);
            continue;
        }
        memset(gppPlayerVisuals[i], 0, sizeof(PlayerVisual));

        // Set player pointer
        gppPlayerVisuals[i]->pPlayer = &game->player[i];

        // Set up display
        Visual *d = &gppPlayerVisuals[i]->display;

        // Set viewport dimensions based on player index
        switch (i) {
            case 0:  // Player 1 (top left)
                d->vp_x = 0;
                d->vp_y = 300;
                d->vp_w = 400;
                d->vp_h = 300;
                break;
            case 1:  // Player 2 (top right)
                d->vp_x = 400;
                d->vp_y = 300;
                d->vp_w = 400;
                d->vp_h = 300;
                break;
            case 2:  // Player 3 (bottom left)
                d->vp_x = 0;
                d->vp_y = 0;
                d->vp_w = 400;
                d->vp_h = 300;
                break;
            case 3:  // Player 4 (bottom right)
                d->vp_x = 400;
                d->vp_y = 0;
                d->vp_w = 400;
                d->vp_h = 300;
                break;
        }

        // Set viewport to be on screen
        d->onScreen = 1;

        // Initialize camera for this player
        initPlayerCamera(gppPlayerVisuals[i], 0);  // 0 = follow camera

        printf("[init] Initialized player visual %d\n", i);
    }

    printf("[init] Players initialized\n");
}
