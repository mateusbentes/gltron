#ifndef GUI_H
#define GUI_H

#include "video/video.h"

// HUD structure declaration
typedef struct {
    struct {
        int x, y;
        float angle;
        float speed;
    } SpeedDial;

    struct {
        int x, y;
        int w, h;
        char* text;
    } SpeedText;

    struct {
        int x, y;
        int active;
    } Buster;

    struct {
        int x, y;
        int w, h;
    } MapFrame;
} HUD_t;

// Declare the HUD variable as extern
extern HUD_t HUD;

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
int guiMainLoop(void);
void drawGuiMenu(Visual *d);

#endif /* GUI_H */
