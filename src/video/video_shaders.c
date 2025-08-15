#include <GLES2/gl2.h>
#include <stdio.h>
#include <stdlib.h>

GLuint gShaderProgram = 0;
GLint gUniformMVP = -1;
GLint gAttribPosition = -1;

static GLuint compileShader(GLenum type, const char *src) {
    GLuint shader = glCreateShader(type);
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

static GLuint createProgram(const char *vsrc, const char *fsrc) {
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

void initBasicShader(void) {
    // Vertex shader with version directive for OpenGL ES 2.0
    const char *vsrc =
        "#ifdef GL_ES\n"
        "precision highp float;\n"
        "#endif\n"
        "uniform mat4 uMVP;\n"
        "attribute vec4 aPosition;\n"
        "void main() {\n"
        "    gl_Position = uMVP * aPosition;\n"
        "}\n";

    // Fragment shader with version directive for OpenGL ES 2.0
    const char *fsrc =
        "#ifdef GL_ES\n"
        "precision mediump float;\n"
        "#endif\n"
        "void main() {\n"
        "#ifdef GL_ES\n"
        "    gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);\n"
        "#else\n"
        "    gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);\n"
        "#endif\n"
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
}
