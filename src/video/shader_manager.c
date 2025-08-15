#include "video/shader_manager.h"
#include <stdio.h>
#include <stdlib.h>

#include <SDL2/SDL.h>

#ifdef __ANDROID__
  #include <GLES2/gl2.h>
#else
  #include <GL/gl.h>
#endif

GLuint gShaderProgram = 0;
GLint gUniformMVP = -1;
GLint gAttribPosition = -1;

static GLuint compileShader(GLenum type, const char *src) {
    GLuint shader = glCreateShader(type);
    if (!shader) {
        fprintf(stderr, "Failed to create shader object\n");
        return 0;
    }

    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[256];
        glGetShaderInfoLog(shader, sizeof(log), NULL, log);
        fprintf(stderr, "Shader compile error: %s\n", log);
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

GLuint createShaderProgram(const char *vsrc, const char *fsrc) {
    // Compile vertex shader
    GLuint vs = compileShader(GL_VERTEX_SHADER, vsrc);
    if (!vs) {
        fprintf(stderr, "Failed to compile vertex shader\n");
        return 0;
    }

    // Compile fragment shader
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fsrc);
    if (!fs) {
        fprintf(stderr, "Failed to compile fragment shader\n");
        glDeleteShader(vs);
        return 0;
    }

    // Create program and attach shaders
    GLuint program = glCreateProgram();
    if (!program) {
        fprintf(stderr, "Failed to create shader program\n");
        glDeleteShader(vs);
        glDeleteShader(fs);
        return 0;
    }

    glAttachShader(program, vs);
    glAttachShader(program, fs);

    // Bind attribute locations before linking
    glBindAttribLocation(program, 0, "aPosition");

    // Link the program
    glLinkProgram(program);

    // Check link status
    GLint linked;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (!linked) {
        char log[512];
        glGetProgramInfoLog(program, sizeof(log), NULL, log);
        fprintf(stderr, "Program link error: %s\n", log);
        glDeleteProgram(program);
        program = 0;
    }

    // Clean up shaders as they're no longer needed after linking
    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

void initBasicShader() {
    // Vertex shader with proper vec4 output
    const char *vsrc =
        "#version 100\n"  // OpenGL ES 2.0
        "uniform mat4 uMVP;\n"
        "attribute vec4 aPosition;\n"
        "void main() {\n"
        "    gl_Position = uMVP * aPosition;\n"
        "}\n";

    // Fragment shader with proper vec4 output
    const char *fsrc =
        "#version 100\n"  // OpenGL ES 2.0
        "precision mediump float;\n"
        "void main() {\n"
        "    gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);\n"
        "}\n";

    // Create the shader program
    gShaderProgram = createShaderProgram(vsrc, fsrc);
    if (!gShaderProgram) {
        fprintf(stderr, "Failed to create shader program\n");
        return;
    }

    // Get uniform and attribute locations
    gUniformMVP = glGetUniformLocation(gShaderProgram, "uMVP");
    gAttribPosition = glGetAttribLocation(gShaderProgram, "aPosition");

    // Verify that the attribute location was set correctly
    if (gAttribPosition != 0) {
        fprintf(stderr, "Warning: Attribute 'aPosition' was not bound to location 0\n");
    }

    printf("Shader program created successfully\n");
    printf("Uniform MVP location: %d\n", gUniformMVP);
    printf("Attribute position location: %d\n", gAttribPosition);
}

void cleanupShader() {
    if (gShaderProgram) {
        glDeleteProgram(gShaderProgram);
        gShaderProgram = 0;
    }
}