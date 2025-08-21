#ifndef SHADERS_H
#define SHADERS_H

#include <GLES2/gl2.h>

GLuint createShaderProgram();
void useShaderProgram(GLuint program);
void setProjectionMatrix(GLuint program, float* matrix);

#endif