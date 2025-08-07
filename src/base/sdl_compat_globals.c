/* Global variables for SDL compatibility layer */

#include <SDL2/SDL.h>

/* These variables are declared as extern in sdl_compat.h */
SDL_Window *g_window = NULL;
SDL_Renderer *g_renderer = NULL;
SDL_GLContext g_context = NULL;
