#include "game/game.h"
#include "game/gltron.h"
#include "input/input.h"
#include "video/video.h"
#include "video/nebu_video_system.h"
#include "audio/audio.h"
#include "audio/sound_glue.h"
#include "configuration/settings.h"
#include "base/nebu_system.h"
#include "base/switchCallbacks.h"
#include "game/init.h"
#include "game/gui.h"
#include "game/engine.h"
#include "video/video.h"
#include "input/input.h"
#include "base/sdl_compat.h"

#include <SDL2/SDL.h>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

// Menu states
typedef enum {
    MENU_STATE_MAIN,
    MENU_STATE_SETTINGS,
    MENU_STATE_HELP,
    MENU_STATE_CREDITS,
    MENU_STATE_GAME
} MenuState;

// Menu options for main menu
typedef enum {
    MENU_OPTION_START_GAME,
    MENU_OPTION_SETTINGS,
    MENU_OPTION_HELP,
    MENU_OPTION_CREDITS,
    MENU_OPTION_EXIT,
    MENU_OPTION_COUNT
} MenuOption;

// Global menu variables
static MenuState gMenuState = MENU_STATE_MAIN;
static int gSelectedOption = MENU_OPTION_START_GAME;
static int gMenuActive = 1;  // Start with menu active

// Menu option text
static const char* gMenuOptionText[MENU_OPTION_COUNT] = {
    "Start Game",
    "Settings",
    "Help",
    "Credits",
    "Exit"
};

// Function prototypes
void drawMenu(void);
void handleMenuInput(int key);
void touchMenu(int state, int x, int y, int screenWidth, int screenHeight);
void startGame(void);
void showSettings(void);
void showHelp(void);
void showCredits(void);
void exitGame(void);
void menuIdle(void);
void returnToMenu(void);

// Initialize the menu system
void initMenu(void) {
    printf("[menu] Initializing menu system\n");
    fflush(stdout);
    gMenuState = MENU_STATE_MAIN;
    printf("[menu] Set menu state to MAIN\n");
    fflush(stdout);
    gSelectedOption = MENU_OPTION_START_GAME;
    printf("[menu] Set selected option to START_GAME\n");
    fflush(stdout);
    gMenuActive = 1;
    printf("[menu] Menu system initialized successfully\n");
    fflush(stdout);
}

// Draw the menu
void drawMenu(void) {
    int i;
    int screenWidth, screenHeight;
    
    printf("[menu] drawMenu called\n");
    fflush(stdout);
    
    // Get screen dimensions
    nebu_Video_GetDimension(&screenWidth, &screenHeight);
    printf("[menu] Screen dimensions: %dx%d\n", screenWidth, screenHeight);
    fflush(stdout);
    
    // Ensure we have valid dimensions
    if (screenWidth <= 0 || screenHeight <= 0) {
        screenWidth = 1024;
        screenHeight = 768;
        printf("[menu] Using fallback dimensions: %dx%d\n", screenWidth, screenHeight);
    }
    
    // Set viewport to full screen
    glViewport(0, 0, screenWidth, screenHeight);
    
    // Clear the screen
    glClearColor(0.0f, 0.0f, 0.1f, 1.0f);  // Dark blue background
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Set up orthographic projection for 2D drawing
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, screenWidth, screenHeight, 0, -1, 1);  // Fixed coordinate system (Y flipped for screen coords)
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    // Disable depth testing and lighting for 2D drawing
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Draw a simple background
    glBegin(GL_QUADS);
    glColor4f(0.0f, 0.0f, 0.2f, 1.0f);  // Dark blue
    glVertex2f(0, 0);
    glVertex2f(screenWidth, 0);
    glColor4f(0.0f, 0.0f, 0.4f, 1.0f);  // Lighter blue
    glVertex2f(screenWidth, screenHeight);
    glVertex2f(0, screenHeight);
    glEnd();
    
    // Draw the GLTron logo background
    glColor4f(0.2f, 0.2f, 0.2f, 0.8f);  // Dark gray background
    glBegin(GL_QUADS);
    glVertex2f(screenWidth/2 - 200, 50);
    glVertex2f(screenWidth/2 + 200, 50);
    glVertex2f(screenWidth/2 + 200, 150);
    glVertex2f(screenWidth/2 - 200, 150);
    glEnd();
    
    // Draw "GLTron" title (simulated with a rectangle)
    glColor4f(0.0f, 0.8f, 1.0f, 1.0f);  // Cyan
    glBegin(GL_QUADS);
    glVertex2f(screenWidth/2 - 150, 70);
    glVertex2f(screenWidth/2 + 150, 70);
    glVertex2f(screenWidth/2 + 150, 130);
    glVertex2f(screenWidth/2 - 150, 130);
    glEnd();
    
    // Draw menu options
    for (i = 0; i < MENU_OPTION_COUNT; i++) {
        int y = 200 + i * 60;  // Start lower and space out more
        
        // Highlight selected option
        if (i == gSelectedOption) {
            // Draw selection box
            glColor4f(0.0f, 0.5f, 1.0f, 0.7f);  // Semi-transparent blue
            glBegin(GL_QUADS);
            glVertex2f(screenWidth/2 - 180, y - 5);
            glVertex2f(screenWidth/2 + 180, y - 5);
            glVertex2f(screenWidth/2 + 180, y + 45);
            glVertex2f(screenWidth/2 - 180, y + 45);
            glEnd();
            
            // Draw selection indicator (triangle)
            glColor4f(1.0f, 1.0f, 0.0f, 1.0f);  // Yellow
            glBegin(GL_TRIANGLES);
            glVertex2f(screenWidth/2 - 200, y + 20);
            glVertex2f(screenWidth/2 - 180, y + 30);
            glVertex2f(screenWidth/2 - 180, y + 10);
            glEnd();
            
            // Draw option text (simulated with a rectangle)
            glColor4f(1.0f, 1.0f, 1.0f, 1.0f);  // White
        } else {
            // Draw option text (simulated with a rectangle)
            glColor4f(0.7f, 0.7f, 0.7f, 1.0f);  // Light gray
        }
        
        // Draw option text background (simulated with a rectangle)
        glBegin(GL_QUADS);
        glVertex2f(screenWidth/2 - 120, y + 5);
        glVertex2f(screenWidth/2 + 120, y + 5);
        glVertex2f(screenWidth/2 + 120, y + 35);
        glVertex2f(screenWidth/2 - 120, y + 35);
        glEnd();
    }
    
    // Draw instructions at bottom
    glColor4f(0.7f, 0.7f, 0.7f, 1.0f);  // Light gray
    
    // Draw instruction text background (simulated with rectangles)
    glBegin(GL_QUADS);
    glVertex2f(screenWidth/2 - 200, screenHeight - 80);
    glVertex2f(screenWidth/2 + 200, screenHeight - 80);
    glVertex2f(screenWidth/2 + 200, screenHeight - 60);
    glVertex2f(screenWidth/2 - 200, screenHeight - 60);
    glEnd();
    
    glBegin(GL_QUADS);
    glVertex2f(screenWidth/2 - 150, screenHeight - 50);
    glVertex2f(screenWidth/2 + 150, screenHeight - 50);
    glVertex2f(screenWidth/2 + 150, screenHeight - 30);
    glVertex2f(screenWidth/2 - 150, screenHeight - 30);
    glEnd();
    
    // Restore OpenGL state
    glEnable(GL_DEPTH_TEST);
    
    // Swap buffers
    nebu_System_SwapBuffers();
}

// Handle menu input
void handleMenuInput(int key) {
    switch (key) {
        case SDLK_UP:
        case 'w':
        case 'W':
            // Move selection up
            gSelectedOption = (gSelectedOption + MENU_OPTION_COUNT - 1) % MENU_OPTION_COUNT;
            printf("[menu] Selected option: %s\n", gMenuOptionText[gSelectedOption]);
            break;
            
        case SDLK_DOWN:
        case 's':
        case 'S':
            // Move selection down
            gSelectedOption = (gSelectedOption + 1) % MENU_OPTION_COUNT;
            printf("[menu] Selected option: %s\n", gMenuOptionText[gSelectedOption]);
            break;
            
        case SDLK_RETURN:
        case SDLK_SPACE:
            // Select current option
            switch (gSelectedOption) {
                case MENU_OPTION_START_GAME:
                    startGame();
                    break;
                    
                case MENU_OPTION_SETTINGS:
                    showSettings();
                    break;
                    
                case MENU_OPTION_HELP:
                    showHelp();
                    break;
                    
                case MENU_OPTION_CREDITS:
                    showCredits();
                    break;
                    
                case MENU_OPTION_EXIT:
                    exitGame();
                    break;
            }
            break;
            
        case SDLK_ESCAPE:  // ESC key
            // If in a submenu, return to main menu
            if (gMenuState != MENU_STATE_MAIN) {
                gMenuState = MENU_STATE_MAIN;
                gSelectedOption = MENU_OPTION_START_GAME;
                printf("[menu] Returned to main menu\n");
            } else {
                // If in main menu, exit game
                exitGame();
            }
            break;
    }
}

// Start the game
void startGame(void) {
    printf("[menu] Starting game\n");
    gMenuActive = 0;
    gMenuState = MENU_STATE_GAME;
    
    // For now, just show a message that the game would start
    // TODO: Implement actual game initialization and transition
    printf("[menu] Game start requested - this would transition to the actual game\n");
    
    // Return to menu for now
    gMenuActive = 1;
    gMenuState = MENU_STATE_MAIN;
}

// Show settings menu
void showSettings(void) {
    printf("[menu] Showing settings menu\n");
    gMenuState = MENU_STATE_SETTINGS;
    
    // For now, just show a message
    printf("[menu] Settings menu requested - this would show game settings\n");
    
    // Return to main menu for now
    gMenuState = MENU_STATE_MAIN;
}


// Show help screen
void showHelp(void) {
    printf("[menu] Showing help screen\n");
    gMenuState = MENU_STATE_HELP;
    // TODO: Implement help screen
}

// Show credits screen
void showCredits(void) {
    printf("[menu] Showing credits screen\n");
    gMenuState = MENU_STATE_CREDITS;
    // TODO: Implement credits screen
}

// Exit the game
void exitGame(void) {
    printf("[menu] Exiting game\n");
    gMenuActive = 0;
    nebu_System_Exit();
}

// Check if menu is active
int isMenuActive(void) {
    return gMenuActive;
}

// Return to menu from game
void returnToMenu(void) {
    printf("[menu] Returning to menu\n");
    gMenuActive = 1;
    gMenuState = MENU_STATE_MAIN;
    gSelectedOption = MENU_OPTION_START_GAME;
    
    // Set menu callbacks
    nebu_System_SetCallback_Display(drawMenu);
    nebu_System_SetCallback_Idle(menuIdle);
    nebu_System_SetCallback_Key(keyMenu);
    nebu_System_SetCallback_Mouse(mouseMenu);
}

// Menu key handler
void keyMenu(int state, int key, int x, int y) {
    // Only process key press events (state == 1 means key down)
    if (state != 1) {
        return;
    }
    
    printf("[menu] Key pressed: %d\n", key);
    handleMenuInput(key);
}

// Menu mouse handler
void mouseMenu(int button, int state, int x, int y) {
    int screenWidth, screenHeight;
    nebu_Video_GetDimension(&screenWidth, &screenHeight);
    
    // Convert mouse click to touch event
    if (button == SDL_BUTTON_LEFT) { // Left mouse button
        touchMenu(state, x, y, screenWidth, screenHeight);
    }
}

// Menu touch handler
void touchMenu(int state, int x, int y, int screenWidth, int screenHeight) {
    printf("[menu] Touch event: state=%d, position=(%d, %d)\n", state, x, y);
    
    // Only process touch down events (state == 1)
    if (state != 1) {
        return;
    }
    
    // Calculate menu item positions (matching the drawing coordinates)
    int menuStartY = 200;
    int menuItemHeight = 60;
    int menuItemCount = MENU_OPTION_COUNT;
    
    // Check if touch is within menu area
    for (int i = 0; i < menuItemCount; i++) {
        int itemY = menuStartY + i * menuItemHeight;
        
        // Check if touch is within this menu item
        if (x >= screenWidth/2 - 180 && x <= screenWidth/2 + 180 &&
            y >= itemY - 5 && y <= itemY + 45) {
            
            // Select this menu item
            gSelectedOption = i;
            printf("[menu] Selected option: %s\n", gMenuOptionText[i]);
            
            // Simulate Enter key press to activate the option
            handleMenuInput(SYSTEM_KEY_RETURN);
            
            break;
        }
    }
}

// Menu idle function
void menuIdle(void) {
    // The display callback (drawMenu) will be called automatically by the main loop
    // We just need to handle any idle processing here
    
    // Delay to limit frame rate
    nebu_Time_FrameDelay(16); // ~60 FPS
}
