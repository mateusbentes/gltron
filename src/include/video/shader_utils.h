#ifndef SHADER_UTILS_H
#define SHADER_UTILS_H

#include <GLES2/gl2.h>

// Global shader variables
extern GLuint gShaderProgram;
extern GLint gUniformMVP;
extern GLint gAttribPosition;

// Initializes a basic shader (white color, MVP transform)
void initBasicShader(void);

// Helper for compiling a shader from source
GLuint compileShader(GLenum type, const char *src);

#endif // SHADER_UTILS_H