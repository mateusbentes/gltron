// shaders.h
#ifndef SHADERS_H
#define SHADERS_H

#ifdef ANDROID
#include <GLES2/gl2.h>
#else
#include <GL/gl.h>
#endif

// Function declarations
GLuint compileShader(GLenum type, const char* source);
GLuint createShaderProgram();
void useShaderProgram(GLuint program);
void setProjectionMatrix(GLuint program, float* matrix);
void setModelMatrix(GLuint program, float* matrix);
void setViewMatrix(GLuint program, float* matrix);
void setColor(GLuint program, float r, float g, float b, float a);
void setTexture(GLuint program, GLuint textureUnit);
GLuint createFontTexture();

#endif // SHADERS_H
