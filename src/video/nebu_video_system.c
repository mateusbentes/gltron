#include "video/nebu_video_system.h"
#include <stdio.h>

static int gScreenWidth = 800;  // Default width
static int gScreenHeight = 600; // Default height

void nebu_Video_SetDimension(int width, int height) {
    printf("[video] Setting screen dimensions to %dx%d\n", width, height);
    gScreenWidth = width;
    gScreenHeight = height;
}

void nebu_Video_GetDimension(int *width, int *height) {
    if (width) *width = gScreenWidth;
    if (height) *height = gScreenHeight;
}

int nebu_Video_GetWidth(void) {
    return gScreenWidth;
}

int nebu_Video_GetHeight(void) {
    return gScreenHeight;
}