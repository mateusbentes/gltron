#ifndef GUI_H
#define GUI_H

#include "video/video.h"

// Function prototypes
void drawGuiBackground(void);
void displayGui(void);
void displayConfigure(void);
void idleGui(void);
void keyboardConfigure(int state, int key, int x, int y);
void keyboardGui(int state, int key, int x, int y);
void initGui(void);
void gui_LoadResources(void);
void gui_ReleaseResources(void);
void exitGui(void);
void guiMouse(int buttons, int state, int x, int y);
void guiMouseMotion(int mx, int my);
int guiMainLoop(void);  // Renamed from runGUI to guiMainLoop
void drawGuiMenu(Visual *d);  // Renamed from drawMenu to avoid conflict

#endif
