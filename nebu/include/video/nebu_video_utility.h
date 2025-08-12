#ifndef NEBU_VIDEO_UTILITY_H
#define NEBU_VIDEO_UTILITY_H

#include <SDL2/SDL.h>
#if defined(__ANDROID__)
  #include <GLES2/gl2.h>
#else
  #include <GL/gl.h>
#endif

#include "base/nebu_surface.h"

void nebu_Video_rasonly(int x, int y, int width, int height, GLuint shaderProgram, GLint uMVP);
nebu_Surface* nebu_Video_GrabScreen(int width, int height);

#endif