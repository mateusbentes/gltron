#ifndef NEBU_VIDEO_SYSTEM_H
#define NEBU_VIDEO_SYSTEM_H

/* Include SDL2 header */
#include <SDL2/SDL.h>

extern void SystemReshapeFunc(void(*reshape)(int, int));
extern void SystemSetGamma(float r, float g, float b);

void nebu_Video_Init(void); // test ok

void nebu_Video_SetWindowMode(int x, int y, int w, int h); // test ok
void nebu_Video_SetDisplayMode(int flags); // test ok
void nebu_Video_GetDisplayDepth(int *r, int *g, int *b, int *a);
int nebu_Video_Create(char *name); // test ok
void nebu_Video_Destroy(int id); // test ok
void nebu_Video_GetDimension(int *x, int *y);

void nebu_Video_WarpPointer(int x, int y);
void nebu_Video_CheckErrors(const char *where);
void nebu_Video_SwapBuffers(void);

// Get the screen size (primarily for mobile devices)
void nebu_Video_GetScreenSize(int *width, int *height);

/* Screen dimension functions */
void nebu_Video_SetDimension(int width, int height);
int nebu_Video_GetWidth(void);
int nebu_Video_GetHeight(void);

#endif
