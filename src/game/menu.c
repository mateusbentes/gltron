#include <SDL2/SDL.h>
#ifdef __ANDROID__
    #include <GLES2/gl2.h>
#else
    #include <GL/glew.h>   // GLEW must come before gl.h!
    #include <GL/gl.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "game/game.h"
#include "game/menu.h"
#include "game/gui.h"
#include "video/video.h"

static int menuActive = 0;

int isMenuActive(void) {
    return menuActive;
}

void activateMenu(void) {
    menuActive = 1;
}

void deactivateMenu(void) {
    menuActive = 0;
}

void initMenu(void) {
    activateMenu();

    initGui(); // Start GUI first

    runGUI(); // Start GUI

    nebu_System_SetCallback_Display(displayMenuCallback);
    nebu_System_SetCallback_Idle(menuIdle);

#ifdef __ANDROID__
    nebu_System_SetCallback_Key((void*)keyboardGui);
    nebu_System_SetCallback_Touch((void*)touchGuiMenu);
#else
    nebu_System_SetCallback_Key((void*)keyboardGui);
    nebu_System_SetCallback_Mouse((void*)mouseGuiMenu);
#endif
}

void initGuiMenuItems(void) {
    printf("[initGuiMenuItems] Menu GUI started\n");
}

void drawGuiMenuWrapper(void) {
    drawGuiMenu(gScreen);
}

void displayMenuCallback(void) {
    if (isMenuActive()) {
        drawGuiMenuWrapper();
    }
}

void menuIdle(void) {
    if (isMenuActive()) {
        idleGui();
    }
}

void keyMenu(SDL_KeyboardEvent *event) {
    if (isMenuActive()) {
        keyGuiMenu(event);
    }
}

void mouseMenu(SDL_MouseButtonEvent *event) {
    if (isMenuActive()) {
        mouseGuiMenu(event);
    }
}

// --- Touch support for Android and mobile ---
void touchMenu(SDL_TouchFingerEvent *event) {
    if (!isMenuActive()) return;

    int screenW = gScreen ? gScreen->vp_w : 800;
    int screenH = gScreen ? gScreen->vp_h : 600;
    int y = (int)(event->y * screenH);

    if (event->type == SDL_FINGERDOWN) {
        if (y > 300 && y < 340) {
            printf("Touched 'Start Game' button\n");
            deactivateMenu();
            initGame();
            nebu_System_SetCallback_Display(displayGame);
            nebu_System_SetCallback_Idle(Game_Idle);
            nebu_System_SetCallback_Key((void*)keyGame);
        } else if (y > 360 && y < 400) {
            printf("Touched 'Exit' button\n");
            SDL_Quit();
        }
    }
}

void keyGuiMenu(SDL_KeyboardEvent *event) {
    if (event->type == SDL_KEYDOWN) {
        switch (event->keysym.sym) {
            case SDLK_RETURN:
                deactivateMenu();
                initGame();
                nebu_System_SetCallback_Display(displayGame);
                nebu_System_SetCallback_Idle(Game_Idle);
                nebu_System_SetCallback_Key((void*)keyGame);
                break;
            case SDLK_ESCAPE:
                printf("Exiting game...\n");
                SDL_Quit();
                break;
            default:
                printf("Pressed key on menu: %d\n", event->keysym.sym);
                break;
        }
    }
}

void mouseGuiMenu(SDL_MouseButtonEvent *event) {
    if (event->type == SDL_MOUSEBUTTONDOWN) {
        if (event->button == SDL_BUTTON_LEFT) {
            int y = event->y;
            if (y > 300 && y < 340) {
                printf("Clicked 'Start Game' button\n");
                deactivateMenu();
                initGame();
                nebu_System_SetCallback_Display(displayGame);
                nebu_System_SetCallback_Idle(Game_Idle);
                nebu_System_SetCallback_Key((void*)keyGame);
            } else if (y > 360 && y < 400) {
                printf("Clicked 'Exit' button\n");
                SDL_Quit();
            }
        }
    }
}

void returnToMenu(void) {
    activateMenu();
    nebu_System_SetCallback_Display(displayMenuCallback);
    nebu_System_SetCallback_Idle(menuIdle);
    nebu_System_SetCallback_Key((void*)keyGuiMenu);
    nebu_System_SetCallback_Mouse((void*)mouseGuiMenu);
    printf("[returnToMenu] Activated menu\n");
}

// --- Event dispatcher for SDL2 main loop ---
void menuHandleSDLEvent(const SDL_Event *event) {
    switch (event->type) {
        case SDL_KEYDOWN:
        case SDL_KEYUP:
            keyMenu((SDL_KeyboardEvent *)&event->key);
            break;
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
            mouseMenu((SDL_MouseButtonEvent *)&event->button);
            break;
        case SDL_FINGERDOWN:
        case SDL_FINGERUP:
        case SDL_FINGERMOTION:
            touchMenu((SDL_TouchFingerEvent *)&event->tfinger);
            break;
        default:
            break;
    }
}