// shaders.h
#ifndef SHADERS_H
#define SHADERS_H

#ifdef ANDROID
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

// Android-only shader utilities
void init_shaders_android();
void shutdown_shaders_android();
GLuint shader_get_basic();

void useShaderProgram(GLuint program);
void setProjectionMatrix(GLuint program, float* matrix);
void setModelMatrix(GLuint program, float* matrix);
void setViewMatrix(GLuint program, float* matrix);
void setColor(GLuint program, float r, float g, float b, float a);
void setTexture(GLuint program, GLuint textureUnit);
GLuint createFontTexture();
#endif

#endif // SHADERS_H
