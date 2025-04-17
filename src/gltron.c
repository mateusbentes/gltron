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
#include "game/gui.h"     // For guiMainLoop

#include "base/nebu_assert.h"
#include "input/input.h"  // For input handling
#include "video/nebu_renderer_gl.h"  // For screen dimensions
#include "video/nebu_video_system.h" // For nebu_Video_GetWidth and nebu_Video_GetHeight

/* No need to declare gWorld here, it's already declared in video.h */

// Function prototypes
void mainDisplay(void);
void mainIdle(void);
void mainKeyboard(int state, int key, int x, int y);
void mainMouse(int button, int state, int x, int y);
void mainMouseMotion(int x, int y);
void mainTouch(int state, int x, int y);

// Forward declarations for functions defined elsewhere
extern void initExitGame(void);
extern void touchMenu(int state, int x, int y, int screenWidth, int screenHeight);
extern void inputTouchGame(int state, int x, int y, int screenWidth, int screenHeight);
extern void inputDrawTouchControls(int screenWidth, int screenHeight);
extern void keyMenu(int state, int key, int x, int y);
extern void keyGame(int state, int key, int x, int y);
extern void mouseMenu(int button, int state, int x, int y);
extern void gameMouse(int button, int state, int x, int y);

// Define SystemEvent type if not already defined
#ifndef SYSTEM_EVENT_DEFINED
#define SYSTEM_EVENT_DEFINED
typedef struct {
    int type;
    union {
        struct {
            float x, y;
        } tfinger;
    };
} SystemEvent;
#endif

// Define system event types
#define SYSTEM_FINGERDOWN 1
#define SYSTEM_FINGERUP 2
#define SYSTEM_FINGERMOTION 3
#define SYSTEM_KEYPRESS 1

// Function prototype for handleSystemEvent
void handleSystemEvent(SystemEvent *event);

int main(int argc, char *argv[] ) {
    nebu_debug_memory_CheckLeaksOnExit();
    // nebu_assert_config(NEBU_ASSERT_PRINT_STDERR);

    /* Initialize subsystems */
    initSubsystems(argc, (const char**) argv);

    /* Initialize the game world before rendering */
    printf("[main] Initializing game world\n");
    if (!gWorld) {
        // printf("[main] Creating game world\n");
        // video_LoadLevel(); // Caso precise carregar um nível
        if (!gWorld) {
            fprintf(stderr, "[error] Failed to create game world\n");
        } else {
            printf("[main] Game world created successfully\n");
        }
    }

    /* Initialize game */
    printf("[main] Initializing game\n");
    initGame();
    if (!game) {
        fprintf(stderr, "[error] Failed to initialize game\n");
        exit(EXIT_FAILURE);
    }
    printf("[main] Game initialized successfully\n");

    /* Initialize players */
    printf("[main] Initializing players\n");
    initPlayers();

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
    nebu_System_SetCallback_SystemEvent(handleSystemEvent);

    /* Enter main loop */
    printf("[main] Entering main loop\n");
    nebu_System_MainLoop();

    /* Exit subsystems */
    exitSubsystems();

    return 0;
}

/* Display callback */
void mainDisplay(void) {
    static int firstCall = 1;

    printf("[mainDisplay] isMenuActive: %d\n", isMenuActive());

    if (isMenuActive()) {
        /* Menu is active, draw menu */
        printf("[mainDisplay] Drawing menu\n");
        menuIdle();
    } else {
        if (firstCall) {
            printf("[mainDisplay] First-time setup: loading level\n");
            video_LoadLevel();
            firstCall = 0;
        }

        printf("[mainDisplay] Drawing game\n");
        displayGame();

        /* Draw touch controls on mobile platforms */
        #if defined(ANDROID) || defined(__ANDROID__) || defined(IOS) || defined(__IOS__)
        inputDrawTouchControls(nebu_Video_GetWidth(), nebu_Video_GetHeight());
        #endif
    }
}

/* Idle callback */
void mainIdle(void) {
    if (isMenuActive()) {
        /* Menu is active, update menu */
        printf("[mainIdle] Updating menu\n");
        menuIdle();
    } else {
        /* Game is active, update game */
        printf("[mainIdle] Updating game\n");
        int status = guiMainLoop();
        if (status == 0) {
            /* Game ended, return to menu */
            printf("[main] Game ended, returning to menu\n");
            initExitGame();
            initMenu();
        }
    }
}

/* Keyboard callback */
void mainKeyboard(int state, int key, int x, int y) {
    if (isMenuActive()) {
        /* Menu is active, handle menu input */
        printf("[mainKeyboard] Handling menu input\n");
        keyMenu(state, key, x, y);
    } else {
        /* Game is active, handle game input */
        printf("[mainKeyboard] Handling game input\n");
        keyGame(state, key, x, y);

        /* Check for escape key to return to menu */
        if (state == SYSTEM_KEYPRESS && key == 27) { // 27 is ESC key
            printf("[main] ESC pressed, returning to menu\n");
            initExitGame();
            initMenu();
        }
    }
}

/* Mouse callback */
void mainMouse(int button, int state, int x, int y) {
    if (isMenuActive()) {
        /* Menu is active, handle menu input */
        printf("[mainMouse] Handling menu input\n");
        mouseMenu(button, state, x, y);
    } else {
        /* Game is active, handle game input */
        printf("[mainMouse] Handling game input\n");
        gameMouse(button, state, x, y);
    }
}

/* Mouse motion callback */
void mainMouseMotion(int x, int y) {
    if (isMenuActive()) {
        /* Menu is active, handle menu input */
        printf("[mainMouseMotion] Handling menu input\n");
        /* TODO: Implement menu mouse motion handling */
    } else {
        /* Game is active, handle game input */
        printf("[mainMouseMotion] Handling game input\n");
        /* TODO: Implement game mouse motion handling */
    }
}

/* Touch input callback */
void mainTouch(int state, int x, int y) {
    if (isMenuActive()) {
        /* Menu is active, handle menu touch input */
        printf("[mainTouch] Handling menu touch input\n");
        touchMenu(state, x, y, nebu_Video_GetWidth(), nebu_Video_GetHeight());
    } else {
        /* Game is active, handle game touch input */
        printf("[mainTouch] Handling game touch input\n");
        inputTouchGame(state, x, y, nebu_Video_GetWidth(), nebu_Video_GetHeight());
    }
}

/* Handle system events for touch input */
void handleSystemEvent(SystemEvent *event) {
    int screenWidth = nebu_Video_GetWidth();
    int screenHeight = nebu_Video_GetHeight();

    #if defined(ANDROID) || defined(__ANDROID__) || defined(IOS) || defined(__IOS__)
    switch (event->type) {
        case SYSTEM_FINGERDOWN:
            mainTouch(1, (int)(event->tfinger.x * screenWidth),
                     (int)(event->tfinger.y * screenHeight));
            break;

        case SYSTEM_FINGERUP:
            mainTouch(0, (int)(event->tfinger.x * screenWidth),
                     (int)(event->tfinger.y * screenHeight));
            break;

        case SYSTEM_FINGERMOTION:
            mainTouch(2, (int)(event->tfinger.x * screenWidth),
                     (int)(event->tfinger.y * screenHeight));
            break;
    }
    #endif
}
