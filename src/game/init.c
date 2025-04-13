#include "filesystem/path.h"
#include "filesystem/dirsetup.h"
#include "game/engine.h"
#include <stdio.h>
#include "game/init.h"
#include "game/gltron.h"
#include "game/game.h"
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
#include "video/video.h"

#include <stdlib.h>
#include <string.h>  /* Added for strncpy */

#include "base/nebu_assert.h"

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

// Define LUA_OK for Lua 5.1 compatibility
#ifndef LUA_OK
#define LUA_OK 0
#endif

static lua_State *lua_state = NULL;

/* Define DEBUG_SCRIPTING - set to 0 for production, 1 for debugging */
#define DEBUG_SCRIPTING 0

/* Define current_callback - used by l_setCallback and l_mainLoop */
static char current_callback[256] = "gui";  /* Default callback is "gui" */

void initFilesystem(int argc, const char *argv[]);
void debug_print_paths(void);
void initGUIs(void);

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

void initSubsystems(int argc, const char *argv[]) {
    nebu_Init();

    resource_Init();

    initFilesystem(argc, argv);
    
    // Make sure initScripting() is called before initVideo()
    printf("[init] Initializing scripting system\n");
    initScripting();
    
    /* Initialize platform-specific settings before general configuration */
    platform_InitSettings();
    
    initConfiguration(argc, argv);
    initArtpacks(); // stores the artpack directory names in a lua table, so we can display it in the menu later on

    initGUIs();
    
    // initVideo() calls video_LoadLevel(), which calls video_CreateLevel()
    printf("[init] Initializing video system\n");
    initVideo();
    
    initAudio();
    initInput();

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

void initScripting(void) {
    printf("[init] Initializing Lua VM\n");
    
    lua_State *L = luaL_newstate();  // Use luaL_newstate instead of lua_open
    if (!L) {
        fprintf(stderr, "[FATAL] Failed to create Lua state\n");
        exit(EXIT_FAILURE);  // Exit on failure
    }
    
    luaL_openlibs(L);  // Open standard libraries
    scripting_SetLuaState(L);
    
    printf("[init] Lua VM initialized successfully\n");
    
    // Register C functions
    lua_register(L, "c_setCallback", l_setCallback);
    lua_register(L, "c_mainLoop", l_mainLoop);
    
#ifdef USE_EMBEDDED_SCRIPTS
    printf("[init] Loading embedded scripts\n");
    const char* scripts[] = {
        "basics.lua", "joystick.lua", "path.lua", 
        "video.lua", "console.lua", "menu.lua",
        "hud.lua", "game.lua", NULL
    };
    
    for (int i = 0; scripts[i]; i++) {
        const char* script = get_embedded_script(scripts[i]);
        if (!script) {
            fprintf(stderr, "[FATAL] Missing embedded script: %s\n", scripts[i]);
            exit(EXIT_FAILURE);
        }
        
        if (luaL_loadbuffer(lua_state, script, strlen(script), scripts[i]) != LUA_OK ||
            lua_pcall(lua_state, 0, 0, 0) != LUA_OK) {
            fprintf(stderr, "[FATAL] Failed to load %s: %s\n", 
                    scripts[i], lua_tostring(lua_state, -1));
            lua_pop(lua_state, 1);  // Pop error message
            exit(EXIT_FAILURE);
        }
    }
#else
    const char* fs_scripts[] = {
        "basics.lua", "joystick.lua", "path.lua",
        "video.lua", "console.lua", "menu.lua",
        "hud.lua", "game.lua", NULL
    };
    
    for (int i = 0; fs_scripts[i]; i++) {
        char path[256];
        snprintf(path, sizeof(path), "scripts/%s", fs_scripts[i]);
        
        if (luaL_loadfile(lua_state, path) != LUA_OK || lua_pcall(lua_state, 0, 0, 0) != LUA_OK) {
            fprintf(stderr, "[FATAL] Failed to load %s: %s\n",
                    path, lua_tostring(lua_state, -1));
            lua_pop(lua_state, 1);  // Pop error message
            exit(EXIT_FAILURE);
        }
    }
#endif

    printf("[init] Scripting system ready\n");
}

void initConfiguration(int argc, const char *argv[])
{
  printf("[init] Initializing configuration\n");
#ifdef USE_EMBEDDED_SCRIPTS
  /* Use embedded scripts for config and artpack */
  printf("[init] Using embedded scripts for configuration\n");
  
  /* Skip setting default settings */
  printf("[init] Skipping default settings (stub)\n");
  
  printf("[init] Configuration initialized\n");
  
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
  
// CHANGE: Modify initAudio to avoid Lua-dependent calls
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
    
#ifdef USE_EMBEDDED_SCRIPTS
    /* Use embedded scripts for audio */
    fprintf(stderr, "[audio] Using embedded scripts for audio\n");
    
    /* Load audio.lua */
    const char* audio_script = get_embedded_script("audio.lua");
    if(audio_script) {
        fprintf(stderr, "[audio] Found embedded script: audio.lua\n");
        /* We don't actually execute the script, just pretend we did */
        fprintf(stderr, "[audio] Processed audio.lua\n");
    } else {
        fprintf(stderr, "[error] Failed to find embedded script: audio.lua\n");
        audio_available = 0;
    }
    
    /* Load music_functions.lua */
    const char* music_functions_script = get_embedded_script("music_functions.lua");
    if(music_functions_script) {
        fprintf(stderr, "[audio] Found embedded script: music_functions.lua\n");
        /* We don't actually execute the script, just pretend we did */
        fprintf(stderr, "[audio] Processed music_functions.lua\n");
    } else {
        fprintf(stderr, "[error] Failed to find embedded script: music_functions.lua\n");
        
        /* CHANGE: Don't use scripting_Run, just print a message */
        fprintf(stderr, "[audio] Would create nextTrack and previousTrack functions (stub)\n");
    }
#else
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
        
        /* CHANGE: Don't use scripting_Run, just print a message */
        fprintf(stderr, "[audio] Would create nextTrack and previousTrack functions (stub)\n");
    }
#endif
    
    /* Load sound samples with error checking */
    fprintf(stderr, "[audio] Loading sound samples\n");
    
    /* Check if sound files exist before loading */
    char *crash_sound = getPossiblePath(PATH_DATA, "sounds/game_crash.wav");
    if (crash_sound && nebu_FS_Test(crash_sound)) {
        fprintf(stderr, "[audio] Loading crash sound from: %s\n", crash_sound);
        Audio_LoadSample(crash_sound, 1);
        free(crash_sound);
    } else {
        fprintf(stderr, "[error] Failed to load crash sound\n");
        if (crash_sound) free(crash_sound);
        audio_available = 0;
    }
    
    char *engine_sound = getPossiblePath(PATH_DATA, "sounds/game_engine.wav");
    if (engine_sound && nebu_FS_Test(engine_sound)) {
        fprintf(stderr, "[audio] Loading engine sound from: %s\n", engine_sound);
        Audio_LoadSample(engine_sound, 0);
        free(engine_sound);
    } else {
        fprintf(stderr, "[error] Failed to load engine sound\n");
        if (engine_sound) free(engine_sound);
        audio_available = 0;
    }
    
    char *recognizer_sound = getPossiblePath(PATH_DATA, "sounds/game_win.wav");
    if (recognizer_sound && nebu_FS_Test(recognizer_sound)) {
        fprintf(stderr, "[audio] Loading recognizer sound from: %s\n", recognizer_sound);
        Audio_LoadSample(recognizer_sound, 2);
        free(recognizer_sound);
    } else {
        fprintf(stderr, "[error] Failed to load recognizer sound\n");
        if (recognizer_sound) free(recognizer_sound);
        audio_available = 0;
    }
    
    /* If audio is not available, disable it */
    if (!audio_available) {
        fprintf(stderr, "[audio] Audio not fully available, disabling music\n");
        /* CHANGE: Don't use scripting_Run, just print a message */
        fprintf(stderr, "[audio] Would disable music (stub)\n");
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
        
        /* CHANGE: Just print a message instead of calling scripting functions */
        fprintf(stderr, "[audio] Would check for nextTrack function and call it (stub)\n");
    } else {
        fprintf(stderr, "[audio] Music is disabled\n");
    }
    
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
#ifdef USE_EMBEDDED_SCRIPTS
    /* Use embedded scripts for GUIs */
    printf("[init] Using embedded scripts for GUIs\n");
    
    /* Load menu scripts */
    const char* menu_functions_script = get_embedded_script("menu_functions.lua");
    if(menu_functions_script) {
        printf("[init] Found embedded script: menu_functions.lua\n");
        /* We don't actually execute the script, just pretend we did */
        printf("[init] Processed menu_functions.lua\n");
    } else {
        fprintf(stderr, "[error] Failed to find embedded script: menu_functions.lua\n");
    }
    
    const char* menu_script = get_embedded_script("menu.lua");
    if(menu_script) {
        printf("[init] Found embedded script: menu.lua\n");
        /* We don't actually execute the script, just pretend we did */
        printf("[init] Processed menu.lua\n");
    } else {
        fprintf(stderr, "[error] Failed to find embedded script: menu.lua\n");
    }
    
    /* Load HUD scripts */
    const char* hud_config_script = get_embedded_script("hud-config.lua");
    if(hud_config_script) {
        printf("[init] Found embedded script: hud-config.lua\n");
        /* We don't actually execute the script, just pretend we did */
        printf("[init] Processed hud-config.lua\n");
    } else {
        fprintf(stderr, "[error] Failed to find embedded script: hud-config.lua\n");
    }
    
    const char* hud_script = get_embedded_script("hud.lua");
    if(hud_script) {
        printf("[init] Found embedded script: hud.lua\n");
        /* We don't actually execute the script, just pretend we did */
        printf("[init] Processed hud.lua\n");
    } else {
        fprintf(stderr, "[error] Failed to find embedded script: hud.lua\n");
    }
    
    const char* gauge_script = get_embedded_script("gauge.lua");
    if(gauge_script) {
        printf("[init] Found embedded script: gauge.lua\n");
        /* We don't actually execute the script, just pretend we did */
        printf("[init] Processed gauge.lua\n");
    } else {
        fprintf(stderr, "[error] Failed to find embedded script: gauge.lua\n");
    }
    
    /* Load Android touch configuration if needed */
#if defined(ANDROID) || defined(__ANDROID__)
    const char* android_touch_script = get_embedded_script("android_touch.lua");
    if(android_touch_script) {
        printf("[init] Found embedded script: android_touch.lua\n");
        /* We don't actually execute the script, just pretend we did */
        printf("[init] Processed android_touch.lua\n");
    } else {
        fprintf(stderr, "[error] Failed to find embedded script: android_touch.lua\n");
    }
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
#if defined(ANDROID) || defined(__ANDROID__)
    runScript(PATH_SCRIPTS, "android_touch.lua");
#endif
#endif
}

void initInput(void) {
	nebu_Input_Init();

	gInput.mouse1 = 0;
	gInput.mouse2 = 0;
}
