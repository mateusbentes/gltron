/* SDL2 compatibility layer implementation */
#include "base/sdl_compat.h"

/* Global variables to store SDL2 objects */
SDL_Window *g_window = NULL;
SDL_Renderer *g_renderer = NULL;
SDL_GLContext g_context = NULL;