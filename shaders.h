// shaders.h
#ifndef SHADERS_H
#define SHADERS_H

#ifdef ANDROID
#include <GLES2/gl2.h>
#else
#include <GL/gl.h>
#endif

// Function declarations
void useShaderProgram(GLuint program);
void setProjectionMatrix(GLuint program, float* matrix);
void setModelMatrix(GLuint program, float* matrix);
void setViewMatrix(GLuint program, float* matrix);
void setColor(GLuint program, float r, float g, float b, float a);
void setTexture(GLuint program, GLuint textureUnit);
GLuint createFontTexture();

// Android-specific shader functions
void init_shaders_android();
GLuint shader_get_basic();
void shutdown_shaders_android();

// Helper functions for better state management
void ensureShaderBound();
void resetMatrices();
void setIdentityMatrix(GLuint program, int matrixType);

// Matrix type constants
#define MATRIX_PROJECTION 0
#define MATRIX_VIEW 1
#define MATRIX_MODEL 2

#endif // SHADERS_H
