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

#include "base/nebu_assert.h"

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
	Sound_shutdown();
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

	scripting_Quit();
	nebu_FS_ClearAllPaths();
	resource_FreeAll();
	resource_Shutdown();
}

void initSubsystems(int argc, const char *argv[]) {
	nebu_Init();

	resource_Init();

	initFilesystem(argc, argv);
	initScripting();

	/* Initialize platform-specific settings before general configuration */
	platform_InitSettings();
	
	initConfiguration(argc, argv);
	initArtpacks(); // stores the artpack directory names in a lua table, so we can display it in the menu later on

	initGUIs();
	initVideo();
	initAudio();  /* Uncommented to initialize audio */
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

void initScripting(void) {
	scripting_Init(NEBU_SCRIPTING_DEBUG);
	init_c_interface();

  /* load basic scripting services */
	runScript(PATH_SCRIPTS, "basics.lua");
	runScript(PATH_SCRIPTS, "joystick.lua");
	runScript(PATH_SCRIPTS, "path.lua");

	runScript(PATH_SCRIPTS, "video.lua");

	runScript(PATH_SCRIPTS, "console.lua");
}

void initConfiguration(int argc, const char *argv[])
{
	/* load some more defaults from config file */
	runScript(PATH_SCRIPTS, "config.lua");
	runScript(PATH_SCRIPTS, "artpack.lua");
	
	/* go for .gltronrc (or whatever is defined in RC_NAME) */
	{
		char *path;
		path = getPossiblePath(PATH_PREFERENCES, RC_NAME);
		if (path != NULL) {
		if (nebu_FS_Test(path)) {
			printf("[status] loading settings from %s\n", path);
			scripting_RunFile(path);
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
	
	// check if the config file is from the same version
	// if not, override using defaults
	{
		float ini_version = 0, app_version;
		if(isSetting("version"))
			ini_version = getSettingf("version");
		scripting_GetGlobal("app_version", NULL);
		scripting_GetFloatResult(&app_version);
		if(ini_version < app_version)
		{
			/* load some more defaults from config file */
			runScript(PATH_SCRIPTS, "config.lua");
			runScript(PATH_SCRIPTS, "artpack.lua");
			printf("[warning] old config file version %f found, app version is %f, overriding using defaults\n",
				ini_version, app_version);
			setSettingf("version", app_version);
		}
	}
	// check if config is valid
	{
		int isValid = 1;
		scripting_GetGlobal("save_completed", NULL);
		if(scripting_IsNil()) {
			isValid = 0;
		}
		scripting_Pop();
		scripting_GetGlobal("settings", "keys", NULL);
		if(!scripting_IsTable())
		{
			isValid = 0;
		}
		scripting_Pop();
		if(!isValid)
		{
			printf("[warning] defunct config file found, overriding using defaults\n");
			runScript(PATH_SCRIPTS, "config.lua");
			runScript(PATH_SCRIPTS, "artpack.lua");

		}
	}

	/* parse any comandline switches overrinding the loaded settings */
	parse_args(argc, argv);

	/* sanity check some settings */
	checkSettings();

	scripting_Run("setupArtpackPaths()");
	scripting_Run("setupLevels()");
		
	/* intialize the settings cache, remember to do that everytime you
	   change something */
	updateSettingsCache();
}

void initVideo(void) {
	nebu_Video_Init();
	// this requuires the player data
	initVideoData();
	setupDisplay();

	loadArt();
	loadModels();
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
    
    char *audio_script = getPossiblePath(PATH_SCRIPTS, "audio.lua");
    if (audio_script && nebu_FS_Test(audio_script)) {
        fprintf(stderr, "[audio] Loading audio.lua from: %s\n", audio_script);
        scripting_RunFile(audio_script);
        free(audio_script);
    } else {
        fprintf(stderr, "[error] Failed to load audio.lua\n");
        if (audio_script) free(audio_script);
        audio_available = 0;
    }
    
    char *music_functions_script = getPossiblePath(PATH_SCRIPTS, "music_functions.lua");
    if (music_functions_script && nebu_FS_Test(music_functions_script)) {
        fprintf(stderr, "[audio] Loading music_functions.lua from: %s\n", music_functions_script);
        scripting_RunFile(music_functions_script);
        free(music_functions_script);
    } else {
        fprintf(stderr, "[error] Failed to load music_functions.lua\n");
        if (music_functions_script) free(music_functions_script);
        
        /* Create a basic music_functions.lua script */
        fprintf(stderr, "[audio] Creating basic music_functions.lua stubs\n");
        scripting_Run("function nextTrack() print('[lua] nextTrack called (stub)') return 1 end");
        scripting_Run("function previousTrack() print('[lua] previousTrack called (stub)') return 1 end");
    }
    
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
        scripting_Run("if settings then settings.playMusic = 0 end");
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
        
        /* Check if nextTrack function exists */
        scripting_GetGlobal("nextTrack", NULL);
        if (!scripting_IsNil()) {
            fprintf(stderr, "[audio] nextTrack function found, calling it\n");
            scripting_Pop();
            scripting_Run("if nextTrack then nextTrack() end");
        } else {
            fprintf(stderr, "[error] nextTrack function not found\n");
            scripting_Pop();
        }
    } else {
        fprintf(stderr, "[audio] Music is disabled\n");
    }
    
    fprintf(stderr, "[audio] Audio initialization complete\n");
}
	
void initGUIs(void)
{
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
}

void initInput(void) {
	nebu_Input_Init();

	gInput.mouse1 = 0;
	gInput.mouse2 = 0;
}
