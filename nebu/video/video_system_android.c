#include "video/nebu_video_system.h"

#if defined(ANDROID) || defined(__ANDROID__)
#include <SDL2/SDL.h>

void nebu_Video_GetScreenSize(int *width, int *height) {
    SDL_DisplayMode mode;
    
    // Initialize width and height to default values
    *width = 1280;
    *height = 720;
    
    // Get the display mode of the primary display
    if(SDL_GetCurrentDisplayMode(0, &mode) == 0) {
        *width = mode.w;
        *height = mode.h;
    }
}
#else
// Stub implementation for non-Android platforms
void nebu_Video_GetScreenSize(int *width, int *height) {
    *width = 0;
    *height = 0;
}
#endif