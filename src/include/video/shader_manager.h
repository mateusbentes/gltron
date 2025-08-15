#ifndef SHADER_MANAGER_H
#define SHADER_MANAGER_H

#include <GLES2/gl2.h>

// Basic shader program and uniform/attribute locations
extern GLuint gShaderProgram;
extern GLint gUniformMVP;
extern GLint gAttribPosition;

// Skybox shader program and uniform/attribute locations
extern GLuint gSkyboxShaderProgram;
extern GLint gSkyboxUniformMVP;
extern GLint gSkyboxUniformSkybox;
extern GLint gSkyboxAttribPosition;

// Initialize the basic shader program
void initBasicShader();

// Initialize the skybox shader program
void initSkyboxShaders();

// Clean up shader resources
void cleanupShader();

// Set up the shader for rendering
void useShader();

// Release the shader after rendering
void releaseShader();

// Set the MVP matrix uniform
void setMVPMatrix(const float *matrix);

// Enable the position attribute
void enablePositionAttribute(GLfloat *vertices, GLsizei stride);

// Disable the position attribute
void disablePositionAttribute();

#endif // SHADER_MANAGER_H
