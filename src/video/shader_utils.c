#include "video/shader_utils.h"
#include <stdio.h>
#include <stdlib.h>

GLuint gShaderProgram = 0;
GLint gUniformMVP = -1;
GLint gAttribPosition = -1;

GLuint compileShader(GLenum type, const char *src) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[512];
        glGetShaderInfoLog(shader, sizeof(log), NULL, log);
        fprintf(stderr, "Shader compile error: %s\n", log);
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

void initBasicShader(void) {
    const char *vsrc =
        "uniform mat4 uMVP;\n"
        "attribute vec4 aPosition;\n"
        "void main() {\n"
        "    gl_Position = uMVP * aPosition;\n"
        "}\n";

    const char *fsrc =
        "precision mediump float;\n"
        "void main() {\n"
        "    gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);\n"
        "}\n";

    GLuint vs = compileShader(GL_VERTEX_SHADER, vsrc);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fsrc);

    gShaderProgram = glCreateProgram();
    glAttachShader(gShaderProgram, vs);
    glAttachShader(gShaderProgram, fs);
    glBindAttribLocation(gShaderProgram, 0, "aPosition");
    glLinkProgram(gShaderProgram);

    GLint linked;
    glGetProgramiv(gShaderProgram, GL_LINK_STATUS, &linked);
    if (!linked) {
        char log[512];
        glGetProgramInfoLog(gShaderProgram, sizeof(log), NULL, log);
        fprintf(stderr, "Program link error: %s\n", log);
        glDeleteProgram(gShaderProgram);
        gShaderProgram = 0;
    }

    gUniformMVP = glGetUniformLocation(gShaderProgram, "uMVP");
    gAttribPosition = glGetAttribLocation(gShaderProgram, "aPosition");

    glDeleteShader(vs);
    glDeleteShader(fs);
}
