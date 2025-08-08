#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "game/menu.h"  // declare isMenuActive()
#include "game/gui.h"   // declare drawGuiMenu(), keyGuiMenu(), mouseGuiMenu(), idleGui()

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

void drawGuiMenuWrapper(void) {
    drawGuiMenu(NULL);
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

// ---- Implementations you requested ----

void initMenu(void) {
    // Initialize menu state, load resources, etc.
    // For example:
    activateMenu();
    // ... any other initialization ...
}

void initGuiMenuItems(void) {
    // Initialize GUI menu items, buttons, etc.
    // For example:
    // setupMenuButtons();
}

void keyGuiMenu(SDL_KeyboardEvent *event) {
    // Handle keyboard events for the GUI menu
    // Example: if (event->keysym.sym == SDLK_RETURN) { ... }
}

void mouseGuiMenu(SDL_MouseButtonEvent *event) {
    // Handle mouse button events for the GUI menu
    // Example: if (event->button == SDL_BUTTON_LEFT) { ... }
}

void returnToMenu(void) {
    // Example: set menu state, stop game, etc.
}