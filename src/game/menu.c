#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "game/game.h"
#include "game/menu.h"  // declare isMenuActive()
#include "game/gui.h"   // declare drawGuiMenu(), keyGuiMenu(), mouseGuiMenu(), idleGui()
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
    // Initialize menu state, load resources, etc.
    activateMenu();
    printf("[initMenu] Activated meni\n");
}

void initGuiMenuItems(void) {
    printf("[initGuiMenuItems] Menu gui started GUI\n");
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

void keyGuiMenu(SDL_KeyboardEvent *event) {
    if (event->type == SDL_KEYDOWN) {
        switch (event->keysym.sym) {
            case SDLK_RETURN:
                // Start game
                deactivateMenu();
                initGame();  // <- important: here start game logic
                nebu_System_SetCallback_Display(displayGame);
                nebu_System_SetCallback_Idle(Game_Idle);
                nebu_System_SetCallback_Key((void*)keyGame);
                break;

            case SDLK_ESCAPE:
                printf("Exiting game...\n");
                exit(0);
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
            int x = event->x;
            int y = event->y;

            // Simples detecção de botão em y fixo
            if (y > 300 && y < 340) {
                printf("Clicked 'Start Game' button\n");
                deactivateMenu();
                initGame();
                nebu_System_SetCallback_Display(displayGame);
                nebu_System_SetCallback_Idle(Game_Idle);
                nebu_System_SetCallback_Key((void*)keyGame);
            } else if (y > 360 && y < 400) {
                printf("Clicked 'Exit' button\n");
                exit(0);
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
    printf("[returnToMenu] Deactivated menu\n");
}
