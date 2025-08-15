#include "video/shader_manager.h"
#include <stdio.h>
#include <stdlib.h>

#include <SDL2/SDL.h>

#ifdef __ANDROID__
  #include <GLES2/gl2.h>
#else
  #include <GL/gl.h>
#endif

// Basic shader variables
GLuint gShaderProgram = 0;
GLint gUniformMVP = -1;
GLint gAttribPosition = -1;

// Skybox shader variables
GLuint gSkyboxShaderProgram = 0;
GLint gSkyboxUniformMVP = -1;
GLint gSkyboxUniformSkybox = -1;
GLint gSkyboxAttribPosition = -1;

static GLuint compileShader(GLenum type, const char *src) {
    printf("Compiling shader of type %d\n", type);

    // Print the shader source for debugging
    printf("Shader source:\n%s\n", src);

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
        char log[1024];  // Increased buffer size for more detailed error messages
        glGetShaderInfoLog(shader, sizeof(log), NULL, log);
        fprintf(stderr, "Shader compile error:\n%s\n", log);
        fprintf(stderr, "Shader source that failed:\n%s\n", src);
        glDeleteShader(shader);
        return 0;
    }

    printf("Shader compiled successfully\n");
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

    // For OpenGL ES 2.0, we need to bind attribute locations before linking
    #ifdef __ANDROID__
    glBindAttribLocation(program, 0, "aPosition");
    #endif

    // Link the program
    glLinkProgram(program);

    // Check link status
    GLint linked;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (!linked) {
        char log[1024];
        glGetProgramInfoLog(program, sizeof(log), NULL, log);
        fprintf(stderr, "Program link error:\n%s\n", log);
        glDeleteProgram(program);
        program = 0;
    }

    // Clean up shaders as they're no longer needed after linking
    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

void initBasicShader() {
    printf("Entering initBasicShader()\n");

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

    // Print shader sources for debugging
    printf("Vertex shader source:\n%s\n", vsrc);
    printf("Fragment shader source:\n%s\n", fsrc);

    // Create the shader program
    printf("Creating shader program...\n");
    gShaderProgram = createShaderProgram(vsrc, fsrc);
    if (!gShaderProgram) {
        fprintf(stderr, "Failed to create shader program\n");
        return;
    }

    printf("Shader program created successfully\n");

    // Get uniform and attribute locations
    printf("Getting uniform and attribute locations...\n");
    gUniformMVP = glGetUniformLocation(gShaderProgram, "uMVP");
    gAttribPosition = glGetAttribLocation(gShaderProgram, "aPosition");

    printf("Uniform MVP location: %d\n", gUniformMVP);
    printf("Attribute position location: %d\n", gAttribPosition);

    // Verify that the attribute location was set correctly
    if (gAttribPosition != 0) {
        fprintf(stderr, "Warning: Attribute 'aPosition' was not bound to location 0\n");
    }

    printf("Shader initialization complete\n");
}

void initSkyboxShaders() {
    printf("Initializing skybox shaders...\n");

    // Vertex shader for skybox with version directive
    const char *vsrc =
        "#version 100\n"  // OpenGL ES 2.0
        "uniform mat4 uMVP;\n"
        "attribute vec3 aPosition;\n"
        "varying vec3 vTexCoord;\n"
        "void main() {\n"
        "    vTexCoord = aPosition;\n"
        "    gl_Position = uMVP * vec4(aPosition, 1.0);\n"
        "}\n";

    // Fragment shader for skybox with version directive
    const char *fsrc =
        "#version 100\n"  // OpenGL ES 2.0
        "precision mediump float;\n"
        "uniform samplerCube uSkybox;\n"
        "varying vec3 vTexCoord;\n"
        "void main() {\n"
        "    gl_FragColor = textureCube(uSkybox, vTexCoord);\n"
        "}\n";

    // Print shader sources for debugging
    printf("Skybox vertex shader source:\n%s\n", vsrc);
    printf("Skybox fragment shader source:\n%s\n", fsrc);

    // Create the shader program
    printf("Creating skybox shader program...\n");
    gSkyboxShaderProgram = createShaderProgram(vsrc, fsrc);
    if (!gSkyboxShaderProgram) {
        fprintf(stderr, "Failed to create skybox shader program\n");
        return;
    }

    printf("Skybox shader program created successfully\n");

    // Get uniform and attribute locations
    printf("Getting skybox uniform and attribute locations...\n");
    gSkyboxUniformMVP = glGetUniformLocation(gSkyboxShaderProgram, "uMVP");
    gSkyboxUniformSkybox = glGetUniformLocation(gSkyboxShaderProgram, "uSkybox");
    gSkyboxAttribPosition = glGetAttribLocation(gSkyboxShaderProgram, "aPosition");

    printf("Skybox MVP location: %d\n", gSkyboxUniformMVP);
    printf("Skybox sampler location: %d\n", gSkyboxUniformSkybox);
    printf("Skybox position location: %d\n", gSkyboxAttribPosition);

    // Verify that the attribute location was set correctly
    if (gSkyboxAttribPosition != 0) {
        fprintf(stderr, "Warning: Skybox attribute 'aPosition' was not bound to location 0\n");
    }

    printf("Skybox shader initialization complete\n");
}

void cleanupShader() {
    if (gShaderProgram) {
        glDeleteProgram(gShaderProgram);
        gShaderProgram = 0;
    }
}