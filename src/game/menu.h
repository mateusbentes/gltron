#ifndef GAME_MENU_H
#define GAME_MENU_H

// Menu system functions
void initMenu(void);
void drawMenu(void);
void menuIdle(void);
void keyMenu(int state, int key, int x, int y);
void mouseMenu(int button, int state, int x, int y);

// Menu state functions
int isMenuActive(void);
void startGame(void);
void showSettings(void);
void showHelp(void);
void showCredits(void);
void exitGame(void);
void returnToMenu(void);

// Touch interface for menu
void touchMenu(int state, int x, int y, int screenWidth, int screenHeight);

#endif // GAME_MENU_H
