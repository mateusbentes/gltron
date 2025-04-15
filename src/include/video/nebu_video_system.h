#ifndef NEBU_VIDEO_SYSTEM_H
#define NEBU_VIDEO_SYSTEM_H

void nebu_Video_Init(void);
void nebu_Video_SetWindowMode(int x, int y, int w, int h);
void nebu_Video_SetDisplayMode(int flags);
void nebu_Video_SetWindowTitle(const char *title);
void nebu_Video_Quit(void);

/* Screen dimension functions */
void nebu_Video_SetDimension(int width, int height);
void nebu_Video_GetDimension(int *width, int *height);
int nebu_Video_GetWidth(void);
int nebu_Video_GetHeight(void);

#endif /* NEBU_VIDEO_SYSTEM_H */