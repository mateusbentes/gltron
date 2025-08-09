#ifndef MENU_H
#define MENU_H

#include <SDL2/SDL.h>
#ifdef __ANDROID__
  #include <GLES2/gl2.h>
#else
  #include <GL/gl.h>
#endif

void gameIdle(void);
void keyGame(SDL_KeyboardEvent *event);
void displayGame(void);
void initGame(void);

// Menu states
typedef enum {
    MENU_STATE_MAIN,
    MENU_STATE_SETTINGS,
    MENU_STATE_HELP,
    MENU_STATE_CREDITS,
    MENU_STATE_GAME
} MenuState;

// Menu options in the main menu
typedef enum {
    MENU_OPTION_START_GAME,
    MENU_OPTION_SETTINGS,
    MENU_OPTION_HELP,
    MENU_OPTION_CREDITS,
    MENU_OPTION_EXIT,
    MENU_OPTION_COUNT
} MenuOption;

// Initialization and main loop
void initMenu(void);
void menuMain(void);

// Drawing and updating menu
void drawMenu(void);
void menuIdle(void);

// Input handlers
void keyMenu(SDL_KeyboardEvent *event);
void mouseMenu(SDL_MouseButtonEvent *event);

// GUI input handlers (for callbacks)
void keyGuiMenu(SDL_KeyboardEvent *event);
void mouseGuiMenu(SDL_MouseButtonEvent *event);

// Display callback for menu
void displayMenuCallback(void);

// Utility
int isMenuActive(void);
void navigateMenu(int direction);
void selectMenuOption(void);

// Menu actions
void showSettings(void);
void showHelp(void);
void showCredits(void);
void exitGame(void);
void returnToMenu(void);

#endif // MENU_H
