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
#include "game/menu.h"    // For menu functions

#include "base/nebu_assert.h"
#include "input/input.h"  // For input handling

/* No need to declare gWorld here, it's already declared in video.h */

// Function prototypes
void mainDisplay(void);
void mainIdle(void);
void mainKeyboard(int state, int key, int x, int y);
void mainMouse(int button, int state, int x, int y);
void mainMouseMotion(int x, int y);

int main(int argc, char *argv[] ) {
    nebu_debug_memory_CheckLeaksOnExit();
    // nebu_assert_config(NEBU_ASSERT_PRINT_STDERR);
    
    /* Initialize subsystems */
    initSubsystems(argc, (const char**) argv);
    
#ifdef USE_EMBEDDED_SCRIPTS
    /* Process main.lua from embedded scripts */
    printf("[main] Processing embedded main.lua\n");
    process_embedded_main();
    
    /* Initialize the game world before rendering */
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
    
    /* Initialize game before game2 */
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
    
    /* Initialize game2 */
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
    
    /* Initialize players */
    printf("[main] Initializing players\n");
    initPlayers();
    
    /* Set game2->play to 1 to indicate the game is playing */
    if (game2) {
        printf("[main] Setting game2->play to 1\n");
        game2->play = 1;
    }
    
    /* Initialize menu system */
    printf("[main] Initializing menu system\n");
    initMenu();
    
    /* Set up callbacks */
    printf("[main] Setting up callbacks\n");
    nebu_System_SetCallback_Display(mainDisplay);
    nebu_System_SetCallback_Idle(mainIdle);
    nebu_System_SetCallback_Key(mainKeyboard);
    nebu_System_SetCallback_Mouse(mainMouse);
    nebu_System_SetCallback_MouseMotion(mainMouseMotion);
    
    /* Enter main loop */
    printf("[main] Entering main loop\n");
    nebu_System_MainLoop();
    
#else
    /* Run main.lua from file */
    runScript(PATH_SCRIPTS, "main.lua");
#endif
    
    /* Exit subsystems */
    exitSubsystems();
    
    return 0;
}

/* Display callback */
void mainDisplay(void) {
    if (isMenuActive()) {
        /* Menu is active, draw menu */
        drawMenu();
    } else {
        /* Game is active, draw game */
        displayGame();
    }
}

/* Idle callback */
void mainIdle(void) {
    if (isMenuActive()) {
        /* Menu is active, update menu */
        menuIdle();
    } else {
        /* Game is active, update game */
        int status = runGUI();
        if (status == 0) {
            /* Game ended, return to menu */
            printf("[main] Game ended, returning to menu\n");
            initMenu();
        }
    }
}

/* Keyboard callback */
void mainKeyboard(int state, int key, int x, int y) {
    if (isMenuActive()) {
        /* Menu is active, handle menu input */
        keyMenu(state, key, x, y);
    } else {
        /* Game is active, handle game input */
        keyGame(state, key, x, y);
    }
}

/* Mouse callback */
void mainMouse(int button, int state, int x, int y) {
    if (isMenuActive()) {
        /* Menu is active, handle menu input */
        mouseMenu(button, state, x, y);
    } else {
        /* Game is active, handle game input */
        gameMouse(button, state, x, y);
    }
}

/* Mouse motion callback */
void mainMouseMotion(int x, int y) {
    if (isMenuActive()) {
        /* Menu is active, handle menu input */
        /* TODO: Implement menu mouse motion handling */
    } else {
        /* Game is active, handle game input */
        /* TODO: Implement game mouse motion handling */
    }
}
