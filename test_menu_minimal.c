/* Minimal test for menu system functionality */

#include "config.h"
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <GL/gl.h>
#include <stdio.h>
#include <stdlib.h>

/* Include our SDL compatibility layer */
#include "src/include/base/sdl_compat.h"

/* Global variables for SDL compatibility */
SDL_Window *g_window = NULL;
SDL_Renderer *g_renderer = NULL;
SDL_GLContext g_context = NULL;

/* Simple menu state */
static int gSelectedOption = 0;
static const int MENU_OPTION_COUNT = 5;
static const char* gMenuOptionText[] = {
    "Start Game",
    "Settings", 
    "Help",
    "Credits",
    "Exit"
};

/* Simple menu drawing function */
void drawSimpleMenu(void) {
    int screenWidth = 1024, screenHeight = 768;
    int i;
    
    printf("[test] Drawing menu\n");
    
    /* Clear screen */
    glClearColor(0.0f, 0.0f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    /* Set up 2D rendering */
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, screenWidth, screenHeight, 0, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    /* Draw menu background */
    glColor4f(0.2f, 0.2f, 0.2f, 0.8f);
    glBegin(GL_QUADS);
    glVertex2f(screenWidth/2 - 200, 50);
    glVertex2f(screenWidth/2 + 200, 50);
    glVertex2f(screenWidth/2 + 200, 150);
    glVertex2f(screenWidth/2 - 200, 150);
    glEnd();
    
    /* Draw menu options */
    for (i = 0; i < MENU_OPTION_COUNT; i++) {
        int y = 200 + i * 60;
        
        if (i == gSelectedOption) {
            /* Highlight selected option */
            glColor4f(0.0f, 0.5f, 1.0f, 0.7f);
            glBegin(GL_QUADS);
            glVertex2f(screenWidth/2 - 180, y - 5);
            glVertex2f(screenWidth/2 + 180, y - 5);
            glVertex2f(screenWidth/2 + 180, y + 45);
            glVertex2f(screenWidth/2 - 180, y + 45);
            glEnd();
            
            glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        } else {
            glColor4f(0.7f, 0.7f, 0.7f, 1.0f);
        }
        
        /* Draw option background */
        glBegin(GL_QUADS);
        glVertex2f(screenWidth/2 - 120, y + 5);
        glVertex2f(screenWidth/2 + 120, y + 5);
        glVertex2f(screenWidth/2 + 120, y + 35);
        glVertex2f(screenWidth/2 - 120, y + 35);
        glEnd();
    }
    
    SDL_GL_SwapWindow_Compat();
}

/* Handle input */
void handleInput(SDL_Event *event) {
    if (event->type == SDL_KEYDOWN) {
        switch (event->key.keysym.sym) {
            case SDLK_UP:
                gSelectedOption = (gSelectedOption + MENU_OPTION_COUNT - 1) % MENU_OPTION_COUNT;
                printf("[test] Selected: %s\n", gMenuOptionText[gSelectedOption]);
                break;
            case SDLK_DOWN:
                gSelectedOption = (gSelectedOption + 1) % MENU_OPTION_COUNT;
                printf("[test] Selected: %s\n", gMenuOptionText[gSelectedOption]);
                break;
            case SDLK_RETURN:
                printf("[test] Activated: %s\n", gMenuOptionText[gSelectedOption]);
                if (gSelectedOption == 4) { /* Exit */
                    exit(0);
                }
                break;
            case SDLK_ESCAPE:
                exit(0);
                break;
        }
    }
}

int main(int argc, char *argv[]) {
    SDL_Event event;
    int running = 1;
    
    printf("[test] Starting minimal menu test\n");
    
    /* Initialize SDL */
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("[test] SDL init failed: %s\n", SDL_GetError());
        return 1;
    }
    
    /* Set OpenGL attributes */
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    
    /* Create window using compatibility layer */
    if (!SDL_SetVideoMode_Compat(1024, 768, 32, SDL_WINDOW_OPENGL)) {
        printf("[test] Failed to create window: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    
    printf("[test] Window created successfully\n");
    
    /* Main loop */
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            } else {
                handleInput(&event);
            }
        }
        
        drawSimpleMenu();
        SDL_Delay(16); /* ~60 FPS */
    }
    
    /* Cleanup */
    SDL_DestroyWindow_Compat();
    SDL_Quit();
    
    printf("[test] Test completed successfully\n");
    return 0;
}
