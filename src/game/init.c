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

#include "base/nebu_assert.h"

/* Define DEBUG_SCRIPTING - set to 0 for production, 1 for debugging */
#define DEBUG_SCRIPTING 0

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
	printf("[init] Initializing scripting system (without Lua)\n");
	
  #ifdef USE_EMBEDDED_SCRIPTS
	/* Use embedded scripts directly */
	printf("[init] Using embedded scripts directly\n");
	
	/* Load basic scripting services */
	const char* script;
	
	/* Load basics.lua */
	printf("[init] Loading embedded basics configuration\n");
	script = get_embedded_script("basics.lua");
	if(script != NULL) {
	  printf("[init] Got embedded script: basics.lua\n");
	  printf("[init] Processing embedded configuration\n");
	  
	  /* Instead of executing the script with Lua, parse it directly */
	  process_embedded_config(script, "basics");
	  
	  printf("[init] Finished processing basics configuration\n");
	} else {
	  fprintf(stderr, "[error] Failed to find embedded script: basics.lua\n");
	  /* Skip the rest of the initialization if we can't find the basics script */
	  return;
	}
	
	/* Process other embedded configurations similarly */
	process_embedded_joystick();
	process_embedded_path();
	process_embedded_video();
	process_embedded_console();
	process_embedded_menu();
	process_embedded_hud();
	process_embedded_gauge();
	process_embedded_config_file();
	process_embedded_save();
	process_embedded_artpack();
	process_embedded_game();
	process_embedded_main();
  #else
	/* load basic scripting services */
	runScript(PATH_SCRIPTS, "basics.lua");
	runScript(PATH_SCRIPTS, "joystick.lua");
	runScript(PATH_SCRIPTS, "path.lua");
	runScript(PATH_SCRIPTS, "video.lua");
	runScript(PATH_SCRIPTS, "console.lua");
	
	/* load the main menu & hud stuff */
	runScript(PATH_SCRIPTS, "menu_functions.lua");
	runScript(PATH_SCRIPTS, "menu.lua");
	runScript(PATH_SCRIPTS, "hud-config.lua");
	runScript(PATH_SCRIPTS, "hud.lua");
	runScript(PATH_SCRIPTS, "gauge.lua");
	runScript(PATH_SCRIPTS, "config.lua");
	runScript(PATH_SCRIPTS, "save.lua");
	runScript(PATH_SCRIPTS, "artpack.lua");
	runScript(PATH_SCRIPTS, "game.lua");
	runScript(PATH_SCRIPTS, "main.lua");
  #endif
  }

  void initConfiguration(int argc, const char *argv[])
  {
  #ifdef USE_EMBEDDED_SCRIPTS
	/* Use embedded scripts for config and artpack */
	printf("[init] Using embedded scripts for configuration\n");
	
	/* Load config and artpack scripts */
	const char* config_script = get_embedded_script("config.lua");
	if(config_script) {
		printf("[init] Running embedded script: config.lua\n");
		scripting_RunString(config_script);
	} else {
		fprintf(stderr, "[error] Failed to find embedded script: config.lua\n");
	}
	
	const char* artpack_script = get_embedded_script("artpack.lua");
	if(artpack_script) {
		printf("[init] Running embedded script: artpack.lua\n");
		scripting_RunString(artpack_script);
	} else {
		fprintf(stderr, "[error] Failed to find embedded script: artpack.lua\n");
	}
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
		  fprintf(stderr, "[audio] Running embedded script: audio.lua\n");
		  scripting_RunString(audio_script);
	  } else {
		  fprintf(stderr, "[error] Failed to find embedded script: audio.lua\n");
		  audio_available = 0;
	  }
	  
	  /* Load music_functions.lua */
	  const char* music_functions_script = get_embedded_script("music_functions.lua");
	  if(music_functions_script) {
		  fprintf(stderr, "[audio] Running embedded script: music_functions.lua\n");
		  scripting_RunString(music_functions_script);
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
	// this requuires the player data
	initVideoData();
	setupDisplay();

	loadArt();
	loadModels();
}
	
void initGUIs(void)
{
#ifdef USE_EMBEDDED_SCRIPTS
	/* Use embedded scripts for GUIs */
	printf("[init] Using embedded scripts for GUIs\n");
	
	/* Load menu scripts */
	const char* menu_functions_script = get_embedded_script("menu_functions.lua");
	if(menu_functions_script) {
		printf("[init] Running embedded script: menu_functions.lua\n");
		scripting_RunString(menu_functions_script);
	} else {
		fprintf(stderr, "[error] Failed to find embedded script: menu_functions.lua\n");
	}
	
	const char* menu_script = get_embedded_script("menu.lua");
	if(menu_script) {
		printf("[init] Running embedded script: menu.lua\n");
		scripting_RunString(menu_script);
	} else {
		fprintf(stderr, "[error] Failed to find embedded script: menu.lua\n");
	}
	
	/* Load HUD scripts */
	const char* hud_config_script = get_embedded_script("hud-config.lua");
	if(hud_config_script) {
		printf("[init] Running embedded script: hud-config.lua\n");
		scripting_RunString(hud_config_script);
	} else {
		fprintf(stderr, "[error] Failed to find embedded script: hud-config.lua\n");
	}
	
	const char* hud_script = get_embedded_script("hud.lua");
	if(hud_script) {
		printf("[init] Running embedded script: hud.lua\n");
		scripting_RunString(hud_script);
	} else {
		fprintf(stderr, "[error] Failed to find embedded script: hud.lua\n");
	}
	
	const char* gauge_script = get_embedded_script("gauge.lua");
	if(gauge_script) {
		printf("[init] Running embedded script: gauge.lua\n");
		scripting_RunString(gauge_script);
	} else {
		fprintf(stderr, "[error] Failed to find embedded script: gauge.lua\n");
	}
	
	/* Load Android touch configuration if needed */
#if defined(ANDROID) || defined(__ANDROID__)
	const char* android_touch_script = get_embedded_script("android_touch.lua");
	if(android_touch_script) {
		printf("[init] Running embedded script: android_touch.lua\n");
		scripting_RunString(android_touch_script);
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
