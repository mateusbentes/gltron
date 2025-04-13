/*
  gltron
  Copyright (C) 1999 by Andreas Umbach <marvin@dataway.ch>
*/
#include "base/nebu_system.h" // includes main
#include "game/init.h"
#include "filesystem/path.h"
#include "base/util.h"
#include "base/nebu_debug_memory.h"
#include "scripting/embedded_processing.h" // Added for process_embedded_main()

/* Add these includes for gWorld, video_LoadLevel, and displayGame */
#include "video/video.h"  // For video_LoadLevel and displayGame
#include "game/game.h"    // For gWorld
#include "game/game_level.h" // For game level functions

#include "base/nebu_assert.h"

/* No need to declare gWorld here, it's already declared in video.h */

int main(int argc, char *argv[] ) {
    nebu_debug_memory_CheckLeaksOnExit();
    // nebu_assert_config(NEBU_ASSERT_PRINT_STDERR);
    
    /* Initialize subsystems */
    initSubsystems(argc, (const char**) argv);
    
#ifdef USE_EMBEDDED_SCRIPTS
    /* Process main.lua from embedded scripts */
    printf("[main] Processing embedded main.lua\n");
    process_embedded_main();
    
    /* CHANGE: Initialize the game world before rendering */
    printf("[main] Initializing game world\n");
    if (!gWorld) {
        printf("[main] Creating game world\n");
        video_LoadLevel();
        if (!gWorld) {
            fprintf(stderr, "[error] Failed to create game world\n");
        } else {
            printf("[main] Game world created successfully\n");
        }
    }
    
    /* CHANGE: Directly call displayGame() to render the game */
    printf("[main] Directly calling displayGame() to render the game\n");
    displayGame();
    
    /* Add a delay to give the window time to appear */
    printf("[main] Waiting for window to appear\n");
    SDL_Delay(1000);  /* Wait for 1 second */
    
    /* CHANGE: Directly call runGUI() to start the game */
    printf("[main] Directly calling runGUI() to start the game\n");
    int status = 1;
    while (status) {
        status = runGUI();
        /* Add a small delay to avoid 100% CPU usage */
        SDL_Delay(10);
        
        /* Force a redraw every frame */
        displayGame();
    }
#else
    /* Run main.lua from file */
    runScript(PATH_SCRIPTS, "main.lua");
#endif
    
    /* Exit subsystems */
    exitSubsystems();
    
    return 0;
}
