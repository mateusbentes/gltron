#include "game/init.h"
#include "game/menu.h"
#include "game/gui.h"
#include "game/game.h"
#include "base/nebu_system.h"
#include <stdio.h>

// Forward declarations for main loop functions
int guiMainLoop(void);
int gameMainLoop(void);

int runGame(void) {
    printf("[game] Switching to GAME mode\n");
    // Initialize and enter game mode
    if (game && game2) {
        enterGame();
        return gameMainLoop();
    } else {
        printf("[game] Game not initialized, returning to menu\n");
        return runGUI();
    }
}

int runGUI(void) {
    printf("[game] Switching to GUI/menu mode\n");
    // Initialize menu if not active
    if (!isMenuActive()) {
        initMenu();
    }
    return guiMainLoop();
}

int runPause(void) {
    printf("[game] Running pause callback\n");
    // For now, just return to GUI
    return runGUI();
}

int runConfigure(void) {
    printf("[game] Running configure callback\n");
    // Show settings menu
    showSettings();
    return 1;  /* Continue running */
}

int runCredits(void) {
    printf("[game] Running credits callback\n");
    // Show credits
    showCredits();
    return 1;  /* Continue running */
}

int runTimedemo(void) {
    printf("[game] Running timedemo callback\n");
    return 0;  /* Quit after timedemo */
}

// Main loop implementations
int gameMainLoop(void) {
    int status = 0;
    
    printf("[game] Running game main loop\n");
    
    // Process input events
    status = nebu_System_MainLoop();
    if (status == 0) {
        return 0; // Quit
    }
    
    // Update game state
    updateGame();
    
    // Display the game
    displayGame();
    
    return 1; // Continue
}