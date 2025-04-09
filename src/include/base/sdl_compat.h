/* SDL2 compatibility layer for SDL 1.2 code */
#ifndef SDL_COMPAT_H
#define SDL_COMPAT_H

#include <SDL2/SDL.h>

/* Define SDL 1.2 constants that don't exist in SDL2 */
#define SDL_FULLSCREEN SDL_WINDOW_FULLSCREEN
#define SDL_OPENGL SDL_WINDOW_OPENGL
#define SDL_RESIZABLE SDL_WINDOW_RESIZABLE

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

/* SDL 1.2 video mode functions - renamed to avoid recursive calls */
static inline SDL_Window* SDL_SetVideoMode_Compat(int width, int height, int bpp, Uint32 flags) {
    Uint32 sdl2_flags = 0;
    
    if (flags & SDL_FULLSCREEN) sdl2_flags |= SDL_WINDOW_FULLSCREEN;
    if (flags & SDL_OPENGL) sdl2_flags |= SDL_WINDOW_OPENGL;
    if (flags & SDL_RESIZABLE) sdl2_flags |= SDL_WINDOW_RESIZABLE;
    
    return SDL_CreateWindow("GLtron", 
                           SDL_WINDOWPOS_CENTERED, 
                           SDL_WINDOWPOS_CENTERED, 
                           width, height, sdl2_flags);
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

/* SDL 1.2 event constants */
/* These are already defined in SDL2 with the same values */
/* #define SDL_KEYDOWN SDL_KEYDOWN */
/* #define SDL_KEYUP SDL_KEYUP */
/* #define SDL_MOUSEMOTION SDL_MOUSEMOTION */
/* #define SDL_MOUSEBUTTONDOWN SDL_MOUSEBUTTONDOWN */
/* #define SDL_MOUSEBUTTONUP SDL_MOUSEBUTTONUP */
/* #define SDL_QUIT SDL_QUIT */

/* SDL 1.2 key modifiers */
/* These are already defined in SDL2 with the same values */
/* #define KMOD_NONE KMOD_NONE */
/* #define KMOD_LSHIFT KMOD_LSHIFT */
/* #define KMOD_RSHIFT KMOD_RSHIFT */
/* #define KMOD_LCTRL KMOD_LCTRL */
/* #define KMOD_RCTRL KMOD_RCTRL */
/* #define KMOD_LALT KMOD_LALT */
/* #define KMOD_RALT KMOD_RALT */
/* #define KMOD_SHIFT KMOD_SHIFT */
/* #define KMOD_CTRL KMOD_CTRL */
/* #define KMOD_ALT KMOD_ALT */

/* SDL 1.2 mouse buttons */
/* These are already defined in SDL2 with the same values */
/* #define SDL_BUTTON_LEFT SDL_BUTTON_LEFT */
/* #define SDL_BUTTON_MIDDLE SDL_BUTTON_MIDDLE */
/* #define SDL_BUTTON_RIGHT SDL_BUTTON_RIGHT */

#endif /* SDL_COMPAT_H */
