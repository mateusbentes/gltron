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

#if defined(__ANDROID__)
  #include <GLES2/gl2.h>
#else
  #include <GL/gl.h>
#endif

#include <SDL2/SDL.h>

// Forward declarations for GUI functions
void initGui(void);
void exitGui(void);
void idleGui(void);
void keyboardGui(int state, int key, int x, int y);

// External window handle
extern SDL_Window *gWindow;
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
void drawGuiBackground(void);

// GUI display and input logic
void displayGui(void);
void idleGui(void);
void keyboardGui(int state, int key, int x, int y);
void touchGuiMenu(SDL_TouchFingerEvent *event);

// GUI resource management
void initGui(void);
void gui_LoadResources(void);
void gui_ReleaseResources(void);
void exitGui(void);

// Main loop functions for different modes
void runGame(void);
void runGUI(void);
void runPause(void);
void runConfigure(void);
void runCredits(void);
void runTimedemo(void);

// Global menu state
static int gActiveMenuIndex = 0;
static int gMenuItemCount = 4;

static int gResIndex = 0; // Default to 800x600

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

        void drawText(nebu_Font* ftx, float x, float y, float size, const char *text, GLuint shaderProgram, GLint uMVP);
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
    // Update audio system
    Sound_idle();

    // Update video system
    Video_Idle();

    // Update input system
    Input_Idle();

    // Frame delay to control update rate
    nebu_Time_FrameDelay(50);

    // Request redisplay
    nebu_System_PostRedisplay();
}

// --- Android Touch Support ---
void touchGuiMenu(SDL_TouchFingerEvent *event) {
    // Convert normalized coordinates to screen pixels
    int screenW = gScreenWidth;
    int screenH = gScreenHeight;
    int x = (int)(event->x * screenW);
    int y = (int)(event->y * screenH);

    if (event->type == SDL_FINGERDOWN) {
        int itemHeight = 24;
        int menuStartY = (int)(screenH * 0.7f);

        for (int i = 0; i < gMenuItemCount; i++) {
            int itemY = menuStartY - itemHeight * i;
            if (y > itemY - itemHeight && y < itemY) {
                gActiveMenuIndex = i;
                if (i == 0) {
                    printf("[GUI] Start Game selected (touch)\n");
                    // TODO: Trigger game start
                } else if (i == 3) {
                    printf("[GUI] Exit selected (touch)\n");
                    nebu_System_ExitLoop(eSRC_GUI_Escape);
                }
                break;
            }
        }
    }
}

// Implement GUI functions
void initGui(void) {
    printf("[GUI] Initializing GUI\n");

    // Load GUI resources
    gui_LoadResources();

    // Initialize GUI state
    gActiveMenuIndex = 0;
    gMenuItemCount = 4;
    gResIndex = 0; // Default to 800x600

    // Set up initial resolution
    applyResolution();

    printf("[GUI] GUI initialized successfully\n");
}

void gui_LoadResources(void) {

    char *path;
    path = getPath(PATH_ART, "default/babbage.0.png");
    
    if (path) {
        pFont = nebu_Font_Load(path, 16, 16, 32, 96);
        free(path);
    } else {
        fprintf(stderr, "[fatal]: can't find babbage.ftx!\n");
        nebu_assert(0);
        exit(1);
    }

    path = nebu_FS_GetPath_WithFilename(PATH_ART, "default/gui.png");
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
    printf("[GUI] Exiting GUI\n");

    // Release GUI resources
    gui_ReleaseResources();

    // Update settings cache
    updateSettingsCache();

    printf("[GUI] GUI exited successfully\n");
}

void runGame(void) {
    SDL_Event event;
    int running = 1;

    while (running) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    running = 0;
                    break;
                case SDL_KEYDOWN:
                case SDL_KEYUP:
                case SDL_MOUSEBUTTONDOWN:
                case SDL_MOUSEBUTTONUP:
                case SDL_MOUSEMOTION:
                    handleGameInput(&event);
                    break;
                default:
                    break;
            }
        }

        updateGameLogic();
        drawGame();
        SDL_GL_SwapWindow(gWindow);
    }
}

void runGUI(void) {
    SDL_Event event;
    int running = 1;

    while (running) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    running = 0;
                    break;
                case SDL_KEYDOWN:
                case SDL_KEYUP:
                case SDL_MOUSEBUTTONDOWN:
                case SDL_MOUSEBUTTONUP:
                case SDL_MOUSEMOTION:
                    handleGUIInput(&event);
                    break;
                default:
                    break;
            }
        }

        updateGUI();
        drawGUI();
        SDL_GL_SwapWindow(gWindow);
    }
}

void runPause(void) {
    SDL_Event event;
    int running = 1;

    while (running) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    running = 0;
                    break;
                case SDL_KEYDOWN:
                case SDL_KEYUP:
                case SDL_MOUSEBUTTONDOWN:
                case SDL_MOUSEBUTTONUP:
                case SDL_MOUSEMOTION:
                    handlePauseInput(&event);
                    break;
                default:
                    break;
            }
        }

        updatePauseMenu();
        drawPauseMenu();
        SDL_GL_SwapWindow(gWindow);
    }
}

void runConfigure(void) {
    SDL_Event event;
    int running = 1;

    while (running) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    running = 0;
                    break;
                case SDL_KEYDOWN:
                case SDL_KEYUP:
                case SDL_MOUSEBUTTONDOWN:
                case SDL_MOUSEBUTTONUP:
                case SDL_MOUSEMOTION:
                    handleConfigureInput(&event);
                    break;
                default:
                    break;
            }
        }

        updateConfigureMenu();
        drawConfigureMenu();
        SDL_GL_SwapWindow(gWindow);
    }
}

void runCredits(void) {
    SDL_Event event;
    int running = 1;

    while (running) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    running = 0;
                    break;
                case SDL_KEYDOWN:
                case SDL_KEYUP:
                case SDL_MOUSEBUTTONDOWN:
                case SDL_MOUSEBUTTONUP:
                case SDL_MOUSEMOTION:
                    handleCreditsInput(&event);
                    break;
                default:
                    break;
            }
        }

        updateCreditsScreen();
        drawCreditsScreen();
        SDL_GL_SwapWindow(gWindow);
    }
}

void runTimedemo(void) {
    SDL_Event event;
    int running = 1;

    while (running) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    running = 0;
                    break;
                case SDL_KEYDOWN:
                case SDL_KEYUP:
                case SDL_MOUSEBUTTONDOWN:
                case SDL_MOUSEBUTTONUP:
                case SDL_MOUSEMOTION:
                    handleTimedemoInput(&event);
                    break;
                default:
                    break;
            }
        }

        updateTimedemo();
        drawTimedemo();
        SDL_GL_SwapWindow(gWindow);
    }
}
