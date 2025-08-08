#include "audio/audio.h"
#include "video/video.h"
#include "video/graphics_utility.h"
#include "input/input.h"
#include "configuration/configuration.h"
#include "configuration/settings.h"
#include "filesystem/path.h"
#include "scripting/scripting.h"
#include "game/32bit_warning.h"

#include "video/nebu_2d.h"
#include "video/nebu_renderer_gl.h"
#include "video/nebu_video_system.h"
#include "input/nebu_input_system.h"
#include "base/nebu_math.h"
#include "scripting/nebu_scripting.h"
#include "filesystem/nebu_filesystem.h"

#include "base/nebu_assert.h"
#include <string.h>

#include "base/nebu_debug_memory.h"

extern int gScreenWidth;
extern int gScreenHeight;

// Resolution options
typedef struct {
    int width, height;
    const char *label;
} ResolutionOption;

static const ResolutionOption gResOptions[] = {
    { 800, 600,   "800 x 600"   },
    { 1024, 768,  "1024 x 768"  },
    { 1280, 720,  "1280 x 720"  },
    { 1280, 1024, "1280 x 1024" },
    { 1366, 768,  "1366 x 768"  },
    { 1440, 900,  "1440 x 900"  },
    { 1600, 900,  "1600 x 900"  },
    { 1680, 1050, "1680 x 1050" },
    { 1920, 1080, "1920 x 1080" }
};
#define NUM_RES_OPTIONS (sizeof(gResOptions)/sizeof(gResOptions[0]))

// Callback structure for the GUI state
typedef struct {
    void (*display)(void);
    void (*idle)(void);
    void (*keyboard)(int state, int key, int x, int y);
    void (*init)(void);
    void (*exit)(void);
    void (*mouse)(int buttons, int state, int x, int y);
    void (*mouseMotion)(int x, int y);
    void (*reshape)(int width, int height);
    const char *name;
} Callbacks;

// Global resources
nebu_2d *pBackground = NULL;
nebu_Font *pFont = NULL;

// Menu display and resolution logic
void drawGuiMenu(Visual *d);
void applyResolution(void);

// Global menu state
static int gActiveMenuIndex = 0;
static int gMenuItemCount = 4;

static int gResIndex = 0; // Default to 800x600

extern SDL_Window *gWindow;
extern int gScreenWidth;
extern int gScreenHeight;

void applyResolution(void) {
    int width = gResOptions[gResIndex].width;
    int height = gResOptions[gResIndex].height;

    if (gWindow) {
        SDL_SetWindowSize(gWindow, width, height);
        SDL_SetWindowPosition(gWindow, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
        gScreenWidth = width;
        gScreenHeight = height;
        glViewport(0, 0, gScreenWidth, gScreenHeight);
        printf("[GUI] Resolution changed to %dx%d\n", width, height);
    }
}

void drawGuiBackground(void) {
    nebu_Video_CheckErrors("gui background start");

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, pBackground->w, 0, pBackground->h, 0, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glDisable(GL_DEPTH_TEST);
    nebu_2d_Draw(pBackground);
}

void drawGuiMenu(Visual *d) {
    if (!d) {
        fprintf(stderr, "[drawGuiMenu] Visual pointer is NULL!\n");
        return;
    }
    if (!pFont) {
        fprintf(stderr, "[drawGuiMenu] Font pointer is NULL!\n");
        return;
    }

    const char *menuItems[] = {
        "Start Game",
        "Sound Options",
        "Resolution",
        "Exit"
    };

    int x = (int)(d->vp_w * 0.1f);
    int y = (int)(d->vp_h * 0.7f);
    int size = 24;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for (int i = 0; i < gMenuItemCount; i++) {
        if (i == gActiveMenuIndex)
            glColor3f(1, 1, 0); // Yellow highlight
        else
            glColor3f(1, 1, 1); // White text

        char line[128];
        if (i == 2) {
            snprintf(line, sizeof(line), "%s: %s", menuItems[i], gResOptions[gResIndex].label);
        } else {
            snprintf(line, sizeof(line), "%s", menuItems[i]);
        }

        drawText(pFont, x, y - size * i, size, line);
    }

    glDisable(GL_BLEND);
}

void displayGui(void) {
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    drawGuiBackground();
    drawGuiMenu(gScreen);

    nebu_System_SwapBuffers();
}

void idleGui(void) {
    Sound_idle();
    Video_Idle();
    Input_Idle();
    nebu_Time_FrameDelay(50);
    nebu_System_PostRedisplay();
}

void keyboardGui(int state, int key, int x, int y) {
    if (state != NEBU_INPUT_KEYSTATE_DOWN) return;

    switch (key) {
        case 27: // ESC
            nebu_System_ExitLoop(eSRC_GUI_Escape);
            break;

        case SYSTEM_KEY_UP:
            gActiveMenuIndex = (gActiveMenuIndex - 1 + gMenuItemCount) % gMenuItemCount;
            break;

        case SYSTEM_KEY_DOWN:
            gActiveMenuIndex = (gActiveMenuIndex + 1) % gMenuItemCount;
            break;

        case SYSTEM_KEY_LEFT:
            if (gActiveMenuIndex == 2) {
                gResIndex = (gResIndex - 1 + NUM_RES_OPTIONS) % NUM_RES_OPTIONS;
                applyResolution();
            }
            break;

        case SYSTEM_KEY_RIGHT:
            if (gActiveMenuIndex == 2) {
                gResIndex = (gResIndex + 1) % NUM_RES_OPTIONS;
                applyResolution();
            }
            break;

        case SYSTEM_KEY_RETURN:
        case ' ':
            if (gActiveMenuIndex == 0) {
                printf("[GUI] Start Game selected\n");
                // TODO: Trigger game start
            } else if (gActiveMenuIndex == 3) {
                printf("[GUI] Exit selected\n");
                nebu_System_ExitLoop(eSRC_GUI_Escape);
            }
            break;
    }
}

void initGui(void) {
    gui_LoadResources();
    updateSettingsCache();
}

void gui_LoadResources(void) {
    char *path;

    path = getPath(PATH_DATA, "babbage.ftx");
    if (path) {
        pFont = nebu_Font_Load(path, PATH_ART);
        free(path);
    } else {
        fprintf(stderr, "[fatal]: can't find babbage.ftx!\n");
        nebu_assert(0);
        exit(1);
    }

    path = nebu_FS_GetPath_WithFilename(PATH_ART, "gui.png");
    if (path) {
        pBackground = nebu_2d_LoadPNG(path, 0);
        free(path);
    } else {
        fprintf(stderr, "[fatal]: can't find gui.png!\n");
        nebu_assert(0);
        exit(1);
    }
}

void gui_ReleaseResources(void) {
    if (pFont) {
        nebu_Font_Free(pFont);
        pFont = NULL;
    }
    if (pBackground) {
        nebu_2d_Free(pBackground);
        pBackground = NULL;
    }
}

void exitGui(void) {
    gui_ReleaseResources();
    updateSettingsCache();
}

Callbacks guiCallbacks = {
    displayGui, idleGui, keyboardGui, initGui, exitGui,
    NULL /* mouse button */, NULL /* mouse motion */, NULL /* reshape */, "gui"
};

void runGUI(void) {
    // Your GUI main loop or logic here
}

void runGame(void) {
    // Main game loop or logic goes here
}

void runPause(void) {
    // Your pause menu logic here
}

void runConfigure(void) {
    // Your configuration menu logic here
}

void runCredits(void) {
    // Your credits screen logic here
}

void runTimedemo(void) {
    // Your timedemo logic here
}