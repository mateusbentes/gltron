/* SDL2 compatibility layer for SDL 1.2 code */
#ifndef SDL_COMPAT_H
#define SDL_COMPAT_H

#include <SDL2/SDL.h>

/* Define SDL 1.2 constants that don't exist in SDL2 */
#define SDL_FULLSCREEN SDL_WINDOW_FULLSCREEN
#define SDL_OPENGL SDL_WINDOW_OPENGL
#define SDL_RESIZABLE SDL_WINDOW_RESIZABLE

/* Global variables to store SDL2 objects */
extern SDL_Window *g_window;
extern SDL_Renderer *g_renderer;
extern SDL_GLContext g_context;

/* SDL 1.2 to SDL2 key mapping */
#define SDLK_KP0 SDLK_KP_0
#define SDLK_KP1 SDLK_KP_1
#define SDLK_KP2 SDLK_KP_2
#define SDLK_KP3 SDLK_KP_3
#define SDLK_KP4 SDLK_KP_4
#define SDLK_KP5 SDLK_KP_5
#define SDLK_KP6 SDLK_KP_6
#define SDLK_KP7 SDLK_KP_7
#define SDLK_KP8 SDLK_KP_8
#define SDLK_KP9 SDLK_KP_9

/* SDL 1.2 video mode functions - renamed to avoid conflicts */
static inline SDL_Surface* SDL_SetVideoMode_Compat(int width, int height, int bpp, Uint32 flags) {
    Uint32 sdl2_flags = 0;
    
    if (flags & SDL_FULLSCREEN) sdl2_flags |= SDL_WINDOW_FULLSCREEN;
    if (flags & SDL_OPENGL) sdl2_flags |= SDL_WINDOW_OPENGL;
    if (flags & SDL_RESIZABLE) sdl2_flags |= SDL_WINDOW_RESIZABLE;
    
    /* Create window if it doesn't exist */
    if (!g_window) {
        g_window = SDL_CreateWindow("GLtron", 
                                  SDL_WINDOWPOS_CENTERED, 
                                  SDL_WINDOWPOS_CENTERED, 
                                  width, height, sdl2_flags);
        
        if (!g_window) {
            return NULL;
        }
        
        /* Create OpenGL context if needed */
        if (flags & SDL_WINDOW_OPENGL) {
            g_context = SDL_GL_CreateContext(g_window);
            if (!g_context) {
                SDL_DestroyWindow(g_window);
                g_window = NULL;
                return NULL;
            }
        }
    } else {
        /* Resize existing window */
        SDL_SetWindowSize(g_window, width, height);
        
        /* Update window flags */
        if (flags & SDL_WINDOW_FULLSCREEN) {
            SDL_SetWindowFullscreen(g_window, SDL_WINDOW_FULLSCREEN_DESKTOP);
        } else {
            SDL_SetWindowFullscreen(g_window, 0);
        }
    }
    
    /* Get the window surface */
    return SDL_GetWindowSurface(g_window);
}

/* Get the current window */
static inline SDL_Window* SDL_GetWindow_Compat(void) {
    return g_window;
}

/* Destroy the current window */
static inline void SDL_DestroyWindow_Compat(void) {
    if (g_context) {
        SDL_GL_DeleteContext(g_context);
        g_context = NULL;
    }
    
    if (g_window) {
        SDL_DestroyWindow(g_window);
        g_window = NULL;
    }
}

/* Swap buffers for OpenGL */
static inline void SDL_GL_SwapWindow_Compat(void) {
    if (g_window) {
        SDL_GL_SwapWindow(g_window);
    }
}

/* SDL 1.2 surface functions - renamed to avoid recursive calls */
static inline void SDL_FreeSurface_Compat(SDL_Surface* surface) {
    if (surface) {
        SDL_FreeSurface(surface);
    }
}

/* SDL 1.2 event handling - renamed to avoid recursive calls */
static inline int SDL_WaitEvent_Compat(SDL_Event* event) {
    return SDL_WaitEvent(event);
}

/* SDL 1.2 surface update function */
static inline void SDL_UpdateRect_Compat(SDL_Surface* surface, int x, int y, int w, int h) {
    if (surface && g_window) {
        SDL_UpdateWindowSurface(g_window);
    }
}

/* SDL 1.2 flip function */
static inline int SDL_Flip_Compat(SDL_Surface* surface) {
    if (g_window) {
        return SDL_UpdateWindowSurface(g_window);
    }
    return -1;
}

/* SDL 1.2 audio functions */
typedef struct {
    SDL_AudioSpec spec;
    Uint8* sound;
    Uint32 soundlen;
    int active;
} Mix_Chunk_Compat;

static inline void Mix_FreeChunk_Compat(Mix_Chunk_Compat* chunk) {
    if (chunk) {
        if (chunk->sound) {
            SDL_free(chunk->sound);
        }
        SDL_free(chunk);
    }
}

/* SDL 1.2 joystick functions - renamed to avoid recursive calls */
static inline int SDL_JoystickNumButtons_Compat(SDL_Joystick* joystick) {
    return SDL_JoystickNumButtons(joystick);
}

/* SDL 1.2 joystick open function */
static inline SDL_Joystick* SDL_JoystickOpen_Compat(int device_index) {
    return SDL_JoystickOpen(device_index);
}

/* SDL 1.2 joystick close function */
static inline void SDL_JoystickClose_Compat(SDL_Joystick* joystick) {
    SDL_JoystickClose(joystick);
}

/* SDL 1.2 joystick name function */
static inline const char* SDL_JoystickName_Compat(int device_index) {
    SDL_Joystick* joystick = SDL_JoystickOpen(device_index);
    const char* name = SDL_JoystickName(joystick);
    SDL_JoystickClose(joystick);
    return name;
}

/* SDL 1.2 event constants */
#define SDL_KEYDOWN SDL_KEYDOWN
#define SDL_KEYUP SDL_KEYUP
#define SDL_MOUSEMOTION SDL_MOUSEMOTION
#define SDL_MOUSEBUTTONDOWN SDL_MOUSEBUTTONDOWN
#define SDL_MOUSEBUTTONUP SDL_MOUSEBUTTONUP
#define SDL_QUIT SDL_QUIT

/* SDL 1.2 key modifiers */
#define KMOD_NONE KMOD_NONE
#define KMOD_LSHIFT KMOD_LSHIFT
#define KMOD_RSHIFT KMOD_RSHIFT
#define KMOD_LCTRL KMOD_LCTRL
#define KMOD_RCTRL KMOD_RCTRL
#define KMOD_LALT KMOD_LALT
#define KMOD_RALT KMOD_RALT
#define KMOD_SHIFT KMOD_SHIFT
#define KMOD_CTRL KMOD_CTRL
#define KMOD_ALT KMOD_ALT

/* SDL 1.2 mouse buttons - use the actual numeric values */
#define SDL_BUTTON_LEFT 1
#define SDL_BUTTON_MIDDLE 2
#define SDL_BUTTON_RIGHT 3

/* SDL 1.2 initialization */
static inline int SDL_Init_Compat(Uint32 flags) {
    return SDL_Init(flags);
}

/* SDL 1.2 quit function */
static inline void SDL_Quit_Compat(void) {
    SDL_DestroyWindow_Compat();
    SDL_Quit();
}

/* SDL 1.2 GL attribute functions */
static inline int SDL_GL_SetAttribute_Compat(SDL_GLattr attr, int value) {
    return SDL_GL_SetAttribute(attr, value);
}

/* SDL 1.2 GL swap buffers */
static inline void SDL_GL_SwapBuffers_Compat(void) {
    if (g_window) {
        SDL_GL_SwapWindow(g_window);
    }
}

#endif /* SDL_COMPAT_H */
