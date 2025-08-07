#include "game/init.h"
#include "game/menu.h"
#include "game/gui.h"
#include "game/game.h"
#include "game/engine.h"
#include "base/nebu_system.h"
#include "configuration/settings.h"
#include <GL/glew.h>
#include <stdio.h>
#include <SDL2/SDL.h>

// Game state management
typedef enum {
    GAME_STATE_MENU,
    GAME_STATE_PLAYING,
    GAME_STATE_PAUSED,
    GAME_STATE_SETTINGS,
    GAME_STATE_CREDITS,
    GAME_STATE_QUIT
} GameState;

static GameState gCurrentGameState = GAME_STATE_MENU;
static GameState gPreviousGameState = GAME_STATE_MENU;

// Screen dimensions - get from settings or use defaults
static int getScreenWidth(void) {
    // Use the correct settings access for GLTron
    int width = getSettingi("width");
    return (width > 0) ? width : 1024;
}

static int getScreenHeight(void) {
    // Use the correct settings access for GLTron
    int height = getSettingi("height");
    return (height > 0) ? height : 768;
}

// Forward declarations
static int handleMenuState(void);
static int handleGameState(void);
static int handlePauseState(void);
static int handleSettingsState(void);
static int handleCreditsState(void);
static void switchToState(GameState newState);

// State transition function
static void switchToState(GameState newState) {
    printf("[callbacks] Switching from state %d to state %d\n", gCurrentGameState, newState);
    gPreviousGameState = gCurrentGameState;
    gCurrentGameState = newState;
    
    switch (newState) {
        case GAME_STATE_MENU:
            // Menu should be activated - we'll handle this in the menu state
            break;
        case GAME_STATE_PLAYING:
            // Deactivate menu - handled in game state
            break;
        case GAME_STATE_PAUSED:
            // Keep menu active for pause screen
            break;
        case GAME_STATE_SETTINGS:
            // Settings handled as menu state
            break;
        case GAME_STATE_CREDITS:
            // Credits handled separately
            break;
        case GAME_STATE_QUIT:
            break;
    }
}

// Menu state handler
static int handleMenuState(void) {
    // Process SDL events
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                return 0; // Quit game
            case SDL_KEYDOWN:
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    return 0; // Quit on escape from main menu
                }
                break;
        }
    }
    
    // Handle menu input and rendering
    if (isMenuActive && isMenuActive()) {
        menuMain();
    } else {
        // If menu is not active, try to activate it
        printf("[callbacks] Menu not active, trying to initialize\n");
        initMenu();
        if (menuMain) {
            menuMain();
        }
    }
    
    return 1; // Continue running
}

// Game state handler
static int handleGameState(void) {
    // Check for pause key first
    const Uint8 *keyState = SDL_GetKeyboardState(NULL);
    if (keyState[SDL_SCANCODE_ESCAPE]) {
        switchToState(GAME_STATE_PAUSED);
        return 1;
    }
    
    // Run the main game loop
    int status = nebu_System_MainLoop();
    if (status == 0) {
        return 0; // Quit requested
    }
    
    // Check if game is properly initialized using the correct types
    if (!game || !game2) {
        printf("[callbacks] Game not initialized, returning to menu\n");
        switchToState(GAME_STATE_MENU);
    }
    
    return 1;
}

// Pause state handler
static int handlePauseState(void) {
    int screenWidth = getScreenWidth();
    int screenHeight = getScreenHeight();
    
    // Process SDL events
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                return 0;
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        // Resume game
                        switchToState(GAME_STATE_PLAYING);
                        return 1;
                    case SDLK_q:
                        // Quit to menu
                        switchToState(GAME_STATE_MENU);
                        return 1;
                }
                break;
        }
    }
    
    // Draw pause overlay
    glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Set up 2D rendering for pause text
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, screenWidth, screenHeight, 0, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Draw pause indicator
    glColor3f(1.0f, 1.0f, 1.0f);
    int centerX = screenWidth / 2;
    int centerY = screenHeight / 2;
    
    glBegin(GL_QUADS);
    glVertex2f(centerX - 100, centerY - 30);
    glVertex2f(centerX + 100, centerY - 30);
    glVertex2f(centerX + 100, centerY + 30);
    glVertex2f(centerX - 100, centerY + 30);
    glEnd();
    
    // Draw instructions
    glColor3f(1.0f, 1.0f, 0.0f);
    glBegin(GL_QUADS);
    glVertex2f(centerX - 150, centerY + 50);
    glVertex2f(centerX + 150, centerY + 50);
    glVertex2f(centerX + 150, centerY + 80);
    glVertex2f(centerX - 150, centerY + 80);
    glEnd();
    
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    
    nebu_System_SwapBuffers();
    
    return 1;
}

// Settings state handler
static int handleSettingsState(void) {
    // For now, just handle as menu state
    // In the future, this could show a specific settings screen
    return handleMenuState();
}

// Credits state handler
static int handleCreditsState(void) {
    int screenWidth = getScreenWidth();
    int screenHeight = getScreenHeight();
    
    // Process SDL events
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                return 0;
            case SDL_KEYDOWN:
                // Any key returns to menu
                switchToState(GAME_STATE_MENU);
                return 1;
        }
    }
    
    // Draw credits screen
    glClearColor(0.1f, 0.1f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, screenWidth, screenHeight, 0, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_TEXTURE_2D);
    
    // Draw credits background
    glColor3f(0.2f, 0.2f, 0.5f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(screenWidth, 0);
    glVertex2f(screenWidth, screenHeight);
    glVertex2f(0, screenHeight);
    glEnd();
    
    // Draw credits title
    int centerX = screenWidth / 2;
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
    glVertex2f(centerX - 100, 100);
    glVertex2f(centerX + 100, 100);
    glVertex2f(centerX + 100, 140);
    glVertex2f(centerX - 100, 140);
    glEnd();
    
    // Draw credits content (simple rectangles representing text)
    glColor3f(0.8f, 0.8f, 0.8f);
    for (int i = 0; i < 5; i++) {
        int y = 200 + i * 40;
        glBegin(GL_QUADS);
        glVertex2f(centerX - 200, y);
        glVertex2f(centerX + 200, y);
        glVertex2f(centerX + 200, y + 25);
        glVertex2f(centerX - 200, y + 25);
        glEnd();
    }
    
    // Draw "Press any key" message
    glColor3f(1.0f, 1.0f, 0.0f);
    glBegin(GL_QUADS);
    glVertex2f(centerX - 120, screenHeight - 80);
    glVertex2f(centerX + 120, screenHeight - 80);
    glVertex2f(centerX + 120, screenHeight - 50);
    glVertex2f(centerX - 120, screenHeight - 50);
    glEnd();
    
    nebu_System_SwapBuffers();
    
    return 1;
}

// Main GUI loop - this is the main state machine
int guiMainLoop(void) {
    switch (gCurrentGameState) {
        case GAME_STATE_MENU:
            return handleMenuState();
        case GAME_STATE_PLAYING:
            return handleGameState();
        case GAME_STATE_PAUSED:
            return handlePauseState();
        case GAME_STATE_SETTINGS:
            return handleSettingsState();
        case GAME_STATE_CREDITS:
            return handleCreditsState();
        case GAME_STATE_QUIT:
            return 0;
        default:
            printf("[callbacks] Unknown game state: %d\n", gCurrentGameState);
            switchToState(GAME_STATE_MENU);
            return 1;
    }
}

// Public callback functions
int runGame(void) {
    printf("[callbacks] Starting game\n");
    switchToState(GAME_STATE_PLAYING);
    return 1;
}

int runGUI(void) {
    printf("[callbacks] Running GUI\n");
    switchToState(GAME_STATE_MENU);
    return guiMainLoop();
}

int runPause(void) {
    printf("[callbacks] Pausing game\n");
    switchToState(GAME_STATE_PAUSED);
    return 1;
}

int runConfigure(void) {
    printf("[callbacks] Opening settings\n");
    switchToState(GAME_STATE_SETTINGS);
    return 1;
}

int runCredits(void) {
    printf("[callbacks] Showing credits\n");
    switchToState(GAME_STATE_CREDITS);
    return 1;
}

int runTimedemo(void) {
    printf("[callbacks] Running timedemo\n");
    // Run a quick demo and then quit
    return 0;
}

// Utility functions for menu integration
void showSettings(void) {
    printf("[callbacks] Showing settings\n");
    switchToState(GAME_STATE_SETTINGS);
}

void showCredits(void) {
    printf("[callbacks] Showing credits\n");
    switchToState(GAME_STATE_CREDITS);
}

void returnToMenu(void) {
    printf("[callbacks] Returning to menu\n");
    switchToState(GAME_STATE_MENU);
}

void exitGame(void) {
    printf("[callbacks] Exiting game\n");
    switchToState(GAME_STATE_QUIT);
}

// GUI update and display functions
void updateGUI(void) {
    // Update GUI state based on current game state
    switch (gCurrentGameState) {
        case GAME_STATE_MENU:
        case GAME_STATE_SETTINGS:
            if (handleMenuInput) {
                handleMenuInput();
            }
            break;
        case GAME_STATE_PLAYING:
            // Game updates handled in handleGameState
            break;
        case GAME_STATE_PAUSED:
            // Pause updates handled in handlePauseState
            break;
        case GAME_STATE_CREDITS:
            // Credits updates handled in handleCreditsState
            break;
        default:
            break;
    }
}

void displayGUI(void) {
    // Display based on current game state
    switch (gCurrentGameState) {
        case GAME_STATE_MENU:
        case GAME_STATE_SETTINGS:
            if (drawMenu) {
                drawMenu();
            }
            break;
        case GAME_STATE_PLAYING:
            // Game display handled in handleGameState
            break;
        case GAME_STATE_PAUSED:
            // Pause display handled in handlePauseState
            break;
        case GAME_STATE_CREDITS:
            // Credits display handled in handleCreditsState
            break;
        default:
            break;
    }
}

// Initialize the callback system
void initCallbacks(void) {
    printf("[callbacks] Initializing callback system\n");
    gCurrentGameState = GAME_STATE_MENU;
    gPreviousGameState = GAME_STATE_MENU;
}

// Get current game state (for external queries)
int getCurrentGameState(void) {
    return (int)gCurrentGameState;
}

// Force state change (for external control)
void forceGameState(int state) {
    if (state >= 0 && state <= GAME_STATE_QUIT) {
        switchToState((GameState)state);
    }
}
