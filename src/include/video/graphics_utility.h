#ifndef GRAPHICS_UTILITY_H
#define GRAPHICS_UTILITY_H

#include "video/nebu_font.h"
#include "video/nebu_video_types.h"
#include <GLES2/gl2.h> // For GLuint, GLint

// Sets up an orthographic projection and viewport for 2D rendering.
// You must pass the shader program and MVP uniform location.
void rasonly(Visual *d, GLuint shaderProgram, GLint uMVP);

// Computes a perspective projection matrix (column-major, OpenGL style).
void doPerspective(float fov, float ratio, float znear, float zfar, float *matrix_out);

// Computes a look-at view matrix (column-major, OpenGL style).
void doLookAt(const float *cam, const float *target, const float *up, float *matrix_out);

// Draws text using a font and a shader-based renderer.
// You must pass the shader program and MVP uniform location.
void drawText(nebu_Font* ftx, float x, float y, float size, const char *text, GLuint shaderProgram, GLint uMVP);

#endif
