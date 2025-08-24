#include "shaders.h"

#ifdef ANDROID
#include <android/log.h>
#include <string.h>
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "GLTron", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "GLTron", __VA_ARGS__)

// Centralized program storage
static GLuint g_shader_basic = 0;

// Simple vertex shader source
static const char* vertexShaderSource =
    "attribute vec4 position;\n"
    "attribute vec2 texCoord;\n"
    "uniform mat4 projectionMatrix;\n"
    "uniform mat4 modelMatrix;\n"
    "uniform mat4 viewMatrix;\n"
    "varying vec2 vTexCoord;\n"
    "void main()\n"
    "{\n"
    "    gl_Position = projectionMatrix * viewMatrix * modelMatrix * position;\n"
    "    vTexCoord = texCoord;\n"
    "}\n";

// Simple fragment shader source
static const char* fragmentShaderSource =
    "precision mediump float;\n"
    "varying vec2 vTexCoord;\n"
    "uniform sampler2D texture;\n"
    "uniform vec4 color;\n"
    "void main()\n"
    "{\n"
    "    vec4 texColor = texture2D(texture, vTexCoord);\n"
    "    gl_FragColor = vec4(color.rgb * texColor.rgb, color.a * texColor.a);\n"
    "}\n";

static GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    if (shader == 0) {
        LOGE("Failed to create shader");
        return 0;
    }

    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    // Check for compilation errors
    GLint success = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        GLsizei len = 0;
        glGetShaderInfoLog(shader, sizeof(infoLog), &len, infoLog);
        LOGE("Shader compilation error (%s): %s", type == GL_VERTEX_SHADER ? "VERTEX" : "FRAGMENT", infoLog);
        LOGE("Source:\n%.*s", len ? len : (GLsizei)strlen(source), source);
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

static GLuint createShaderProgram() {
    // Compile shaders
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    if (vertexShader == 0) {
        LOGE("Failed to compile vertex shader");
        return 0;
    }

    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    if (fragmentShader == 0) {
        LOGE("Failed to compile fragment shader");
        glDeleteShader(vertexShader);
        return 0;
    }

    // Create shader program
    GLuint shaderProgram = glCreateProgram();
    if (shaderProgram == 0) {
        LOGE("Failed to create shader program");
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return 0;
    }

    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);

    // Optionally bind attribute locations before linking
    glBindAttribLocation(shaderProgram, 0, "position");
    glBindAttribLocation(shaderProgram, 1, "texCoord");

    glLinkProgram(shaderProgram);

    // Check for linking errors
    GLint success = GL_FALSE;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        LOGE("Shader program linking error: %s", infoLog);
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
        LOGE("Invalid shader program");
        return;
    }
    glUseProgram(program);
}

void setProjectionMatrix(GLuint program, float* matrix) {
    if (program == 0 || matrix == NULL) {
        LOGE("Invalid parameters for setProjectionMatrix");
        return;
    }

    GLint projectionMatrixLocation = glGetUniformLocation(program, "projectionMatrix");
    if (projectionMatrixLocation == -1) {
        LOGE("Failed to get projection matrix location");
        return;
    }

    glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, matrix);
}

void setModelMatrix(GLuint program, float* matrix) {
    if (program == 0 || matrix == NULL) {
        LOGE("Invalid parameters for setModelMatrix");
        return;
    }

    GLint modelMatrixLocation = glGetUniformLocation(program, "modelMatrix");
    if (modelMatrixLocation == -1) {
        LOGE("Failed to get model matrix location");
        return;
    }

    glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, matrix);
}

void setViewMatrix(GLuint program, float* matrix) {
    if (program == 0 || matrix == NULL) {
        LOGE("Invalid parameters for setViewMatrix");
        return;
    }

    GLint viewMatrixLocation = glGetUniformLocation(program, "viewMatrix");
    if (viewMatrixLocation == -1) {
        LOGE("Failed to get view matrix location");
        return;
    }

    glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, matrix);
}

void setColor(GLuint program, float r, float g, float b, float a) {
    if (program == 0) {
        LOGE("Invalid shader program");
        return;
    }

    GLint colorLocation = glGetUniformLocation(program, "color");
    if (colorLocation == -1) {
        LOGE("Failed to get color location");
        return;
    }

    glUniform4f(colorLocation, r, g, b, a);
}

void setTexture(GLuint program, GLuint textureUnit) {
    if (program == 0) {
        LOGE("Invalid shader program");
        return;
    }

    GLint textureLocation = glGetUniformLocation(program, "texture");
    if (textureLocation == -1) {
        LOGE("Failed to get texture location");
        return;
    }

    glUniform1i(textureLocation, textureUnit);
}

void init_shaders_android() {
    if (g_shader_basic != 0) return;
    g_shader_basic = createShaderProgram();
    if (!g_shader_basic) {
        LOGE("init_shaders_android: failed to create basic shader");
    } else {
        LOGI("init_shaders_android: basic shader created %u", (unsigned)g_shader_basic);
    }
}

void shutdown_shaders_android() {
    if (g_shader_basic) {
        glDeleteProgram(g_shader_basic);
        g_shader_basic = 0;
    }
}

GLuint shader_get_basic() {
    return g_shader_basic;
}

// Function to create a font texture (simplified)
GLuint createFontTexture() {
    // In a real implementation, you would load a proper font texture here
    // This is a simplified placeholder that creates a basic texture

    // Create a simple 16x16 font texture (ASCII characters)
    unsigned char fontData[16*16*4]; // RGBA data

    // Fill with white color
    for (int i = 0; i < 16*16*4; i += 4) {
        fontData[i] = 255;     // R
        fontData[i+1] = 255;   // G
        fontData[i+2] = 255;  // B
        fontData[i+3] = 255;  // A
    }

    // Create texture
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Upload texture data
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE, fontData);

    return texture;
}
#endif
