#ifndef MENU_H
#define MENU_H

#include <SDL2/SDL.h>
#include <GL/glew.h> 
#include <GL/gl.h>

// Definições de menu
typedef enum {
    MENU_STATE_MAIN,
    MENU_STATE_SETTINGS,
    MENU_STATE_HELP,
    MENU_STATE_CREDITS,
    MENU_STATE_GAME
} MenuState;

typedef enum {
    MENU_OPTION_START_GAME,
    MENU_OPTION_SETTINGS,
    MENU_OPTION_HELP,
    MENU_OPTION_CREDITS,
    MENU_OPTION_EXIT,
    MENU_OPTION_COUNT
} MenuOption;

// Funções do menu
void initMenu(void);
void drawMenu(void);
void menuIdle(void);
void keyMenu(int state, int key, int x, int y);
void mouseMenu(int button, int state, int x, int y);
int isMenuActive(void);
void showSettings(void);
void showHelp(void);
void showCredits(void);
void exitGame(void);
void returnToMenu(void);
void navigateMenu(int direction);
void selectMenuOption(void);
void handleMenuInput(void);
void menuMain(void);

#endif // MENU_H