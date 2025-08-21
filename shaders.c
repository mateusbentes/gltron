#include "shaders.h"
#include <stdio.h>

// Simple vertex shader source
const char* vertexShaderSource =
    "attribute vec4 position;\n"
    "uniform mat4 projectionMatrix;\n"
    "void main()\n"
    "{\n"
    "    gl_Position = projectionMatrix * position;\n"
    "}\n";

// Simple fragment shader source
const char* fragmentShaderSource =
    "precision mediump float;\n"
    "void main()\n"
    "{\n"
    "    gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);\n"
    "}\n";

GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    if (shader == 0) {
        printf("Failed to create shader\n");
        return 0;
    }

    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    // Check for compilation errors
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        printf("Shader compilation error: %s\n", infoLog);
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

GLuint createShaderProgram() {
    // Compile shaders
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    if (vertexShader == 0) {
        printf("Failed to compile vertex shader\n");
        return 0;
    }

    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    if (fragmentShader == 0) {
        printf("Failed to compile fragment shader\n");
        glDeleteShader(vertexShader);
        return 0;
    }

    // Create shader program
    GLuint shaderProgram = glCreateProgram();
    if (shaderProgram == 0) {
        printf("Failed to create shader program\n");
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return 0;
    }

    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Check for linking errors
    GLint success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        printf("Shader program linking error: %s\n", infoLog);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        glDeleteProgram(shaderProgram);
        return 0;
    }

    // Clean up shaders
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

void useShaderProgram(GLuint program) {
    if (program == 0) {
        printf("Invalid shader program\n");
        return;
    }
    glUseProgram(program);
}

void setProjectionMatrix(GLuint program, float* matrix) {
    if (program == 0 || matrix == NULL) {
        printf("Invalid parameters for setProjectionMatrix\n");
        return;
    }

    GLint projectionMatrixLocation = glGetUniformLocation(program, "projectionMatrix");
    if (projectionMatrixLocation == -1) {
        printf("Failed to get projection matrix location\n");
        return;
    }

    glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, matrix);
}
