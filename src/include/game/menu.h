#ifndef MENU_H
#define MENU_H

/* Menu colors */
extern float menu_fgColor[4];
extern float menu_hlColor1[4];
extern float menu_hlColor2[4];

/* Menu display properties */
typedef struct mDisplay {
  float fgColor[4]; /* entries */
  float hlColor1[4]; /* the highlighted one */
  float hlColor2[4];
  char szCaption[64];
} mDisplay;

/* Menu structure */
typedef struct Menu {
  int nEntries;
  int iHighlight;
  mDisplay display;
  char szName[64];
  char szCapFormat[64];
  struct Menu** pEntries;
  struct Menu* parent;
  void* param; /* reserved to bind parameters at runtime */
} Menu;

typedef struct node {
  void* data;
  void* next;
} node;

/* Menu states */
typedef enum {
  MENU_STATE_MAIN,
  MENU_STATE_SETTINGS,
  MENU_STATE_HELP,
  MENU_STATE_CREDITS,
  MENU_STATE_GAME
} MenuState;

/* Menu options for main menu */
typedef enum {
  MENU_OPTION_START_GAME,
  MENU_OPTION_SETTINGS,
  MENU_OPTION_HELP,
  MENU_OPTION_CREDITS,
  MENU_OPTION_EXIT,
  MENU_OPTION_COUNT
} MenuOption;

/* TODO: this variable is a hack */
extern int menutime;

/* Function prototypes */
void initMenu(void);
void drawMenu(void);
void handleMenuInput(int key);
void startGame(void);
void showSettings(void);
void showHelp(void);
void showCredits(void);
void exitGame(void);
int isMenuActive(void);
void keyMenu(int state, int key, int x, int y);
void mouseMenu(int button, int state, int x, int y);
void menuIdle(void);

#endif
