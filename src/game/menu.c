#include "game/game.h"
#include "game/gltron.h"
#include "input/input.h"
#include "video/video.h"
#include "audio/audio.h"
#include "audio/sound_glue.h"
#include "configuration/settings.h"
#include "base/nebu_system.h"
#include "base/switchCallbacks.h"

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

// Initialize the menu system
void initMenu(void) {
    printf("[menu] Initializing menu system\n");
    gMenuState = MENU_STATE_MAIN;
    gSelectedOption = MENU_OPTION_START_GAME;
    gMenuActive = 1;
}

// Draw the menu
void drawMenu(void) {
    int i;
    int screenWidth, screenHeight;
    
    // Get screen dimensions
    nebu_Video_GetDimension(&screenWidth, &screenHeight);
    
    // Clear the screen
    glClearColor(0.0f, 0.0f, 0.1f, 1.0f);  // Dark blue background
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Set up orthographic projection for 2D drawing
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, screenWidth, screenHeight, 0, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    // Disable depth testing and lighting for 2D drawing
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    
    // Draw a simple background
    glBegin(GL_QUADS);
    glColor4f(0.0f, 0.0f, 0.2f, 1.0f);  // Dark blue
    glVertex2f(0, 0);
    glVertex2f(screenWidth, 0);
    glColor4f(0.0f, 0.0f, 0.4f, 1.0f);  // Lighter blue
    glVertex2f(screenWidth, screenHeight);
    glVertex2f(0, screenHeight);
    glEnd();
    
    // Draw the GLTron logo
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);  // White
    glBegin(GL_QUADS);
    glVertex2f(screenWidth/2 - 200, 100);
    glVertex2f(screenWidth/2 + 200, 100);
    glVertex2f(screenWidth/2 + 200, 200);
    glVertex2f(screenWidth/2 - 200, 200);
    glEnd();
    
    // Draw "GLTron" text (simulated with a rectangle)
    glColor4f(0.0f, 0.8f, 1.0f, 1.0f);  // Cyan
    glBegin(GL_QUADS);
    glVertex2f(screenWidth/2 - 150, 120);
    glVertex2f(screenWidth/2 + 150, 120);
    glVertex2f(screenWidth/2 + 150, 180);
    glVertex2f(screenWidth/2 - 150, 180);
    glEnd();
    
    // Draw menu options
    for (i = 0; i < MENU_OPTION_COUNT; i++) {
        int y = 300 + i * 50;
        
        // Highlight selected option
        if (i == gSelectedOption) {
            // Draw selection box
            glColor4f(0.0f, 0.5f, 1.0f, 0.5f);  // Semi-transparent blue
            glBegin(GL_QUADS);
            glVertex2f(screenWidth/2 - 150, y - 5);
            glVertex2f(screenWidth/2 + 150, y - 5);
            glVertex2f(screenWidth/2 + 150, y + 35);
            glVertex2f(screenWidth/2 - 150, y + 35);
            glEnd();
            
            // Draw selection indicator (triangle)
            glColor4f(1.0f, 1.0f, 0.0f, 1.0f);  // Yellow
            glBegin(GL_TRIANGLES);
            glVertex2f(screenWidth/2 - 170, y + 15);
            glVertex2f(screenWidth/2 - 150, y + 25);
            glVertex2f(screenWidth/2 - 150, y + 5);
            glEnd();
            
            // Draw option text (simulated with a rectangle)
            glColor4f(1.0f, 1.0f, 1.0f, 1.0f);  // White
        } else {
            // Draw option text (simulated with a rectangle)
            glColor4f(0.7f, 0.7f, 0.7f, 1.0f);  // Light gray
        }
        
        // Draw option text (simulated with a rectangle)
        glBegin(GL_QUADS);
        glVertex2f(screenWidth/2 - 100, y + 5);
        glVertex2f(screenWidth/2 + 100, y + 5);
        glVertex2f(screenWidth/2 + 100, y + 25);
        glVertex2f(screenWidth/2 - 100, y + 25);
        glEnd();
    }
    
    // Draw instructions
    glColor4f(0.7f, 0.7f, 0.7f, 1.0f);  // Light gray
    
    // Draw instruction text (simulated with rectangles)
    glBegin(GL_QUADS);
    glVertex2f(screenWidth/2 - 200, screenHeight - 100);
    glVertex2f(screenWidth/2 + 200, screenHeight - 100);
    glVertex2f(screenWidth/2 + 200, screenHeight - 80);
    glVertex2f(screenWidth/2 - 200, screenHeight - 80);
    glEnd();
    
    glBegin(GL_QUADS);
    glVertex2f(screenWidth/2 - 150, screenHeight - 70);
    glVertex2f(screenWidth/2 + 150, screenHeight - 70);
    glVertex2f(screenWidth/2 + 150, screenHeight - 50);
    glVertex2f(screenWidth/2 - 150, screenHeight - 50);
    glEnd();
    
    // Restore OpenGL state
    glEnable(GL_DEPTH_TEST);
    
    // Swap buffers
    nebu_System_SwapBuffers();
}

// Handle menu input
void handleMenuInput(int key) {
    switch (key) {
        case SYSTEM_KEY_UP:
        case 'w':
        case 'W':
            // Move selection up
            gSelectedOption = (gSelectedOption + MENU_OPTION_COUNT - 1) % MENU_OPTION_COUNT;
            printf("[menu] Selected option: %s\n", gMenuOptionText[gSelectedOption]);
            break;
            
        case SYSTEM_KEY_DOWN:
        case 's':
        case 'S':
            // Move selection down
            gSelectedOption = (gSelectedOption + 1) % MENU_OPTION_COUNT;
            printf("[menu] Selected option: %s\n", gMenuOptionText[gSelectedOption]);
            break;
            
        case SYSTEM_KEY_RETURN:
        case SYSTEM_KEY_SPACE:
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
            
        case 27:  // ESC key
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
    
    // Initialize game
    initGame();
    enterGame();
}

// Show settings menu
void showSettings(void) {
    printf("[menu] Showing settings menu\n");
    gMenuState = MENU_STATE_SETTINGS;

    // Initialize GUI
    initGui();
    nebu_System_SetCallback_Display(guiCallbacks.display);
    nebu_System_SetCallback_Idle(guiCallbacks.idle);
    nebu_System_SetCallback_Key(guiCallbacks.keyboard);
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
    nebu_System_Exit();
}

// Check if menu is active
int isMenuActive(void) {
    return gMenuActive;
}

// Menu key handler
void keyMenu(int state, int key, int x, int y) {
    // Only process key press events
    if (state != SYSTEM_KEYPRESS) {
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
    if (button == 0) { // Left mouse button
        touchMenu(state == SYSTEM_MOUSEPRESSED ? 1 : 0, x, y, screenWidth, screenHeight);
    }
}

// Menu touch handler
void touchMenu(int state, int x, int y, int screenWidth, int screenHeight) {
    printf("[menu] Touch event: state=%d, position=(%d, %d)\n", state, x, y);
    
    // Only process touch down events (state == 1)
    if (state != 1) {
        return;
    }
    
    // Calculate menu item positions
    int menuStartY = 300;
    int menuItemHeight = 50;
    int menuItemCount = MENU_OPTION_COUNT;
    
    // Check if touch is within menu area
    for (int i = 0; i < menuItemCount; i++) {
        int itemY = menuStartY + i * menuItemHeight;
        
        // Check if touch is within this menu item
        if (x >= screenWidth/2 - 150 && x <= screenWidth/2 + 150 &&
            y >= itemY - 5 && y <= itemY + 35) {
            
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
    // Draw the menu
    drawMenu();
    
    // Delay to limit frame rate
    nebu_Time_FrameDelay(20);
}
