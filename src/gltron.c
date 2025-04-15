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
    
    /* CHANGE: Initialize game before game2 */
    printf("[main] Initializing game\n");
    if (!game) {
        printf("[main] Creating game\n");
        initGame();
        if (!game) {
            fprintf(stderr, "[error] Failed to initialize game\n");
            exit(EXIT_FAILURE);
        }
        printf("[main] Game initialized successfully\n");
    }
    
    /* CHANGE: Initialize game2 before calling displayGame */
    printf("[main] Initializing game2\n");
    if (!game2) {
        printf("[main] Creating game2\n");
        initGame2();
        if (!game2) {
            fprintf(stderr, "[error] Failed to initialize game2\n");
            exit(EXIT_FAILURE);
        }
        printf("[main] game2 initialized successfully\n");
    }
    
    /* CHANGE: Initialize players */
    printf("[main] Initializing players\n");
    initPlayers();
    
    /* CHANGE: Set game2->play to 1 to indicate the game is playing */
    if (game2) {
        printf("[main] Setting game2->play to 1\n");
        game2->play = 1;
    }
    
    /* CHANGE: Directly call displayGame() to render the game */
    printf("[main] Directly calling displayGame() to render the game\n");
    displayGame();
    
    /* Add a delay to give the window time to appear */
    printf("[main] Forcing window refresh\n");
    displayGame();
    nebu_System_SwapBuffers();
    SDL_Delay(100);  // Short delay
    displayGame();
    nebu_System_SwapBuffers();
    
    /* CHANGE: Directly call runGUI() to start the game */
    printf("[main] Directly calling runGUI() to start the game\n");
    int status = 1;
    while (status) {
        status = runGUI();
        /* Add a small delay to avoid 100% CPU usage */
        SDL_Delay(10);
    }
#else
    /* Run main.lua from file */
    runScript(PATH_SCRIPTS, "main.lua");
#endif
    
    /* Exit subsystems */
    exitSubsystems();
    
    return 0;
}
