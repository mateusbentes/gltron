#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Platform detection
#if defined(__ANDROID__)
    #define PLATFORM_MOBILE 1
    #define PLATFORM_NAME "Android"
#else
    #define PLATFORM_MOBILE 0
    #define PLATFORM_NAME "PC"
#endif

// Externally provided window/context (from video system)
extern SDL_Window *gWindow;
extern SDL_GLContext gGLContext;

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

// Defensive static label arrays
static const char *res_labels[NUM_RES_OPTIONS] = {
    "800 x 600", "1024 x 768", "1280 x 720", "1280 x 1024",
    "1366 x 768", "1440 x 900", "1600 x 900", "1680 x 1050", "1920 x 1080"
};

#if PLATFORM_MOBILE
static const char *fullscreen_labels[] = { "On" };
#else
static const char *fullscreen_labels[] = { "Off", "On" };
#endif

// Menu item types
typedef enum {
    MENU_ACTION,
    MENU_SUBMENU,
    MENU_LIST
} MenuItemType;

typedef struct MenuItem MenuItem;

struct MenuItem {
    char caption[64];
    MenuItemType type;
    void (*action)(void);
    MenuItem **children;
    int num_children;
    int selected_index; // for lists
    const char **labels; // for lists
    int num_labels;
    MenuItem *parent;
};

// Menu state
static MenuItem *gRootMenu = NULL;
static MenuItem *gCurrentMenu = NULL;
static int gSelectedItem = 0;
static int gMenuActive = 1;

// Video state
static int gScreenWidth = 1024;
static int gScreenHeight = 768;
static int gIsFullscreen = PLATFORM_MOBILE ? 1 : 0;
static int gResIndex = 1; // Default to 1024x768

// Forward declarations
static void createMenus(void);
static void handleInput(void);
static void startGame(void);
static void quitGame(void);
static void (*drawMenuImpl)(void) = NULL;
void menuDrawMenu(void);

// Menu Actions

void drawMenu(void) {
    if (drawMenuImpl)
        drawMenuImpl();
}

static void startGame(void) {
    printf("[menu] Start Game selected\n");
    gMenuActive = 0;
    // TODO: Start game logic
}

static void quitGame(void) {
    printf("[menu] Quit selected\n");
    exit(0);
}

// Resolution/Fullscreen Actions
static void applyResolution(void) {
#if !PLATFORM_MOBILE
    gScreenWidth = gResOptions[gResIndex].width;
    gScreenHeight = gResOptions[gResIndex].height;
    if (gWindow) {
        SDL_SetWindowSize(gWindow, gScreenWidth, gScreenHeight);
        SDL_SetWindowPosition(gWindow, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
        printf("[menu] Resolution set to %dx%d\n", gScreenWidth, gScreenHeight);
    }
#else
    printf("[menu] Resolution change not available on mobile\n");
#endif
}

static void toggleFullscreenAction(void) {
#if PLATFORM_MOBILE
    printf("[menu] Fullscreen toggle not available on mobile (always fullscreen)\n");
#else
    gIsFullscreen = !gIsFullscreen;
    if (gWindow) {
        if (gIsFullscreen) {
            SDL_SetWindowFullscreen(gWindow, SDL_WINDOW_FULLSCREEN_DESKTOP);
            printf("[menu] Switched to fullscreen\n");
        } else {
            SDL_SetWindowFullscreen(gWindow, 0);
            SDL_SetWindowSize(gWindow, gScreenWidth, gScreenHeight);
            SDL_SetWindowPosition(gWindow, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
            printf("[menu] Switched to windowed\n");
        }
    }
#endif
}

// Menu Item Declarations
static MenuItem mainMenuItem, menuVideoOptions;
static MenuItem itemStartGame, itemQuit;
static MenuItem itemResolution, itemFullscreen, itemApply, itemBackVideo;

// Menu Creation
static void createMenus(void) {
    // Main Menu
    mainMenuItem.type = MENU_SUBMENU;
    strcpy(mainMenuItem.caption, "Main Menu");
    mainMenuItem.num_children = 3;
    mainMenuItem.children = (MenuItem **)calloc(mainMenuItem.num_children, sizeof(MenuItem*));
    mainMenuItem.children[0] = &itemStartGame;
    mainMenuItem.children[1] = &menuVideoOptions;
    mainMenuItem.children[2] = &itemQuit;
    mainMenuItem.parent = NULL;

    // Start Game
    itemStartGame.type = MENU_ACTION;
    strcpy(itemStartGame.caption, "Start Game");
    itemStartGame.action = startGame;
    itemStartGame.parent = &mainMenuItem;

    // Quit
    itemQuit.type = MENU_ACTION;
    strcpy(itemQuit.caption, "Quit");
    itemQuit.action = quitGame;
    itemQuit.parent = &mainMenuItem;

    // Video Options
    menuVideoOptions.type = MENU_SUBMENU;
    strcpy(menuVideoOptions.caption, "Video Options");
#if PLATFORM_MOBILE
    menuVideoOptions.num_children = 2;
#else
    menuVideoOptions.num_children = 4;
#endif
    menuVideoOptions.children = (MenuItem **)calloc(menuVideoOptions.num_children, sizeof(MenuItem*));
    int idx = 0;
#if !PLATFORM_MOBILE
    menuVideoOptions.children[idx++] = &itemResolution;
#endif
    menuVideoOptions.children[idx++] = &itemFullscreen;
#if !PLATFORM_MOBILE
    menuVideoOptions.children[idx++] = &itemApply;
#endif
    menuVideoOptions.children[idx++] = &itemBackVideo;
    menuVideoOptions.parent = &mainMenuItem;

#if !PLATFORM_MOBILE
    // Resolution
    itemResolution.type = MENU_LIST;
    strcpy(itemResolution.caption, "Resolution");
    itemResolution.labels = res_labels;
    itemResolution.num_labels = NUM_RES_OPTIONS;
    itemResolution.selected_index = gResIndex;
    itemResolution.parent = &menuVideoOptions;
#endif

    // Fullscreen
    itemFullscreen.type = MENU_LIST;
    strcpy(itemFullscreen.caption, "Fullscreen");
    itemFullscreen.labels = fullscreen_labels;
#if PLATFORM_MOBILE
    itemFullscreen.num_labels = 1;
    itemFullscreen.selected_index = 0;
#else
    itemFullscreen.num_labels = 2;
    itemFullscreen.selected_index = gIsFullscreen;
#endif
    itemFullscreen.parent = &menuVideoOptions;

#if !PLATFORM_MOBILE
    // Apply
    itemApply.type = MENU_ACTION;
    strcpy(itemApply.caption, "Apply");
    itemApply.action = applyResolution;
    itemApply.parent = &menuVideoOptions;
#endif

    // Back
    itemBackVideo.type = MENU_ACTION;
    strcpy(itemBackVideo.caption, "Back");
    itemBackVideo.action = NULL;
    itemBackVideo.parent = &menuVideoOptions;

    // Set root menu
    gRootMenu = &mainMenuItem;
    gCurrentMenu = gRootMenu;
    gSelectedItem = 0;
}

// Drawing
void menuDrawMenu(void) {
    if (!gCurrentMenu) { printf("menuDrawMenu: gCurrentMenu is NULL!\n"); return; }
    if (!gCurrentMenu->children) { printf("menuDrawMenu: gCurrentMenu->children is NULL!\n"); return; }
    int w = gScreenWidth, h = gScreenHeight;
    if (gWindow) SDL_GetWindowSize(gWindow, &w, &h);
    SDL_GL_MakeCurrent(gWindow, gGLContext);
    glViewport(0, 0, w, h);
    glClearColor(0.1f, 0.1f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, w, h, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Draw background
    glColor3f(0.2f, 0.2f, 0.5f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0); glVertex2f(w, 0);
    glVertex2f(w, h); glVertex2f(0, h);
    glEnd();

    // Draw title
    int centerX = w/2, titleY = 100;
    glColor3f(1,1,1);
    int titleWidth = strlen(gCurrentMenu->caption)*12;
    glBegin(GL_QUADS);
    glVertex2f(centerX-titleWidth/2, titleY-20);
    glVertex2f(centerX+titleWidth/2, titleY-20);
    glVertex2f(centerX+titleWidth/2, titleY+20);
    glVertex2f(centerX-titleWidth/2, titleY+20);
    glEnd();

    // Draw menu items
    int itemY = 200;
    for (int i = 0; i < gCurrentMenu->num_children; ++i) {
        MenuItem *item = gCurrentMenu->children[i];
        if (!item) { printf("menuDrawMenu: children[%d] is NULL!\n", i); continue; }
        int selected = (i == gSelectedItem);
        int width = 300, height = 30;
        glColor3f(selected ? 0.8f : 0.3f, selected ? 0.8f : 0.3f, selected ? 0.2f : 0.3f);
        glBegin(GL_QUADS);
        glVertex2f(centerX-width/2, itemY-height/2); glVertex2f(centerX+width/2, itemY-height/2);
        glVertex2f(centerX+width/2, itemY+height/2); glVertex2f(centerX-width/2, itemY+height/2);
        glEnd();
        glColor3f(1,1,1);
        glBegin(GL_LINE_LOOP);
        glVertex2f(centerX-width/2, itemY-height/2); glVertex2f(centerX+width/2, itemY-height/2);
        glVertex2f(centerX+width/2, itemY+height/2); glVertex2f(centerX-width/2, itemY+height/2);
        glEnd();

        // Draw label
        glColor3f(0,0,0);
        int textWidth = strlen(item->caption)*8, textHeight = 12;
        glBegin(GL_QUADS);
        glVertex2f(centerX-textWidth/2, itemY-textHeight/2);
        glVertex2f(centerX+textWidth/2, itemY-textHeight/2);
        glVertex2f(centerX+textWidth/2, itemY+textHeight/2);
        glVertex2f(centerX-textWidth/2, itemY+textHeight/2);
        glEnd();

        // Optionally, draw current value for lists
        if (item->type == MENU_LIST && item->labels && item->num_labels > 0) {
            if (item->selected_index < 0 || item->selected_index >= item->num_labels) {
                printf("menuDrawMenu: selected_index out of bounds for %s\n", item->caption);
                continue;
            }
            const char *val = item->labels[item->selected_index];
            glColor3f(0.2f,0.2f,0.2f);
            int valWidth = strlen(val)*8;
            glBegin(GL_QUADS);
            glVertex2f(centerX+width/2+10, itemY-textHeight/2);
            glVertex2f(centerX+width/2+10+valWidth, itemY-textHeight/2);
            glVertex2f(centerX+width/2+10+valWidth, itemY+textHeight/2);
            glVertex2f(centerX+width/2+10, itemY+textHeight/2);
            glEnd();
        }
        itemY += 50;
    }
    SDL_GL_SwapWindow(gWindow);
}

// Input
static void navigateMenu(int dir) {
    int count = gCurrentMenu->num_children;
    if (count == 0) return;
    gSelectedItem = (gSelectedItem + dir + count) % count;
}

static void selectMenuItem(void) {
    MenuItem *item = gCurrentMenu->children[gSelectedItem];
    if (!item) return;
    if (item->type == MENU_ACTION) {
        if (item->action) item->action();
        else if (strcmp(item->caption, "Back") == 0 && gCurrentMenu->parent) {
            gCurrentMenu = gCurrentMenu->parent;
            gSelectedItem = 0;
        }
    } else if (item->type == MENU_SUBMENU) {
        gCurrentMenu = item;
        gSelectedItem = 0;
    } else if (item->type == MENU_LIST) {
#if !PLATFORM_MOBILE
        if (item == &itemResolution) {
            item->selected_index = (item->selected_index + 1) % item->num_labels;
            gResIndex = item->selected_index;
        } else
#endif
        if (item == &itemFullscreen) {
#if PLATFORM_MOBILE
            // Do nothing, always fullscreen
#else
            item->selected_index = (item->selected_index + 1) % item->num_labels;
            gIsFullscreen = item->selected_index;
            toggleFullscreenAction();
#endif
        }
    }
}

static void goBack(void) {
    if (gCurrentMenu && gCurrentMenu->parent) {
        gCurrentMenu = gCurrentMenu->parent;
        gSelectedItem = 0;
    }
}

static void handleInput(void) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT: quitGame(); break;
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_UP:    navigateMenu(-1); break;
                    case SDLK_DOWN:  navigateMenu(1); break;
                    case SDLK_RETURN:
                    case SDLK_SPACE: selectMenuItem(); break;
                    case SDLK_ESCAPE: goBack(); break;
                }
                break;
            case SDL_WINDOWEVENT:
                // Update width/height on resize
                if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                    gScreenWidth = event.window.data1;
                    gScreenHeight = event.window.data2;
                    glViewport(0, 0, gScreenWidth, gScreenHeight);
                }
                break;
        }
    }
}

// Main Menu Loop
void menuMainLoop(void) {
    while (gMenuActive) {
        handleInput();
        menuDrawMenu();
        SDL_Delay(16); // ~60fps
    }
}

// Entry Point for Menu System
void menuSystemInit(void) {
    createMenus();
    drawMenuImpl = menuDrawMenu;
    gMenuActive = 1;
    printf("[menu] Menu system initialized for %s\n", PLATFORM_NAME);
}

void menuSystemCleanup(void) {
    // Cleanup menu data if needed
}

// Legacy API Compatibility Layer
int isMenuActive(void) { return gMenuActive; }
void initMenu(void) { menuSystemInit(); }
void activateMenu(void) { gMenuActive = 1; }
void menuMain(void) { menuMainLoop(); }
void menuIdle(void) { menuDrawMenu(); }
void keyMenu(int key, int down) {
    SDL_Event event;
    memset(&event, 0, sizeof(event));
    event.type = down ? SDL_KEYDOWN : SDL_KEYUP;
    event.key.keysym.sym = key;
    SDL_PushEvent(&event);
}
void mouseMenu(int button, int state, int x, int y) { (void)button; (void)state; (void)x; (void)y; }
void handleMenuInput(void) { handleInput(); }
