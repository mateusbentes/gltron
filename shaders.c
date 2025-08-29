#include "shaders.h"

#ifdef ANDROID
#include <android/log.h>
#include <string.h>
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "GLTron", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "GLTron", __VA_ARGS__)

// Centralized program storage
static GLuint g_shader_basic = 0;
// Cached uniform/attrib locations for basic shader
static GLint u_proj = -1;
static GLint u_view = -1;
static GLint u_model = -1;
static GLint u_normal = -1;
static GLint u_color = -1;
static GLint u_tex = -1;
static GLint u_lightPos = -1;
static GLint u_lightColor = -1;
static GLint u_ambientLight = -1;
static GLint a_pos = -1;
static GLint a_texcoord = -1;
static GLint a_normal = -1;

// Vertex shader source with proper lighting calculations
static const char* vertexShaderSource =
    "attribute vec3 position;\n"
    "attribute vec2 texCoord;\n"
    "attribute vec3 normal;\n"
    "uniform mat4 projectionMatrix;\n"
    "uniform mat4 modelMatrix;\n"
    "uniform mat4 viewMatrix;\n"
    "uniform mat4 normalMatrix;\n"
    "varying vec2 vTexCoord;\n"
    "varying vec3 vNormal;\n"
    "varying vec3 vPosition;\n"
    "void main()\n"
    "{\n"
    "    vTexCoord = texCoord;\n"
    "    vNormal = mat3(normalMatrix) * normal;\n"
    "    vPosition = vec3(modelMatrix * vec4(position, 1.0));\n"
    "    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(position, 1.0);\n"
    "}\n";

// Fragment shader source with proper lighting calculations
static const char* fragmentShaderSource =
    "precision mediump float;\n"
    "varying vec2 vTexCoord;\n"
    "varying vec3 vNormal;\n"
    "varying vec3 vPosition;\n"
    "uniform sampler2D texture;\n"
    "uniform vec4 color;\n"
    "uniform vec3 lightPosition;\n"
    "uniform vec3 lightColor;\n"
    "uniform vec3 ambientLight;\n"
    "void main()\n"
    "{\n"
    "    vec4 texColor = texture2D(texture, vTexCoord);\n"
    "    vec3 normal = normalize(vNormal);\n"
    "    vec3 lightDir = normalize(lightPosition - vPosition);\n"
    "    float diffuse = max(dot(normal, lightDir), 0.0);\n"
    "    vec3 lighting = ambientLight + lightColor * diffuse;\n"
    "    gl_FragColor = vec4(color.rgb * texColor.rgb * lighting, color.a * texColor.a);\n"
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
    // Reset cached locations
    u_proj = u_view = u_model = u_normal = u_color = u_tex = -1;
    u_lightPos = u_lightColor = u_ambientLight = -1;
    a_pos = a_texcoord = a_normal = -1;

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

    // Bind attribute locations before linking
    glBindAttribLocation(shaderProgram, 0, "position");
    glBindAttribLocation(shaderProgram, 1, "texCoord");
    glBindAttribLocation(shaderProgram, 2, "normal");

    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);

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

    // Cache uniform and attribute locations
    glUseProgram(shaderProgram);

    // Get matrix locations
    u_proj = glGetUniformLocation(shaderProgram, "projectionMatrix");
    u_view = glGetUniformLocation(shaderProgram, "viewMatrix");
    u_model = glGetUniformLocation(shaderProgram, "modelMatrix");
    u_normal = glGetUniformLocation(shaderProgram, "normalMatrix");

    // Get color and texture locations
    u_color = glGetUniformLocation(shaderProgram, "color");
    u_tex = glGetUniformLocation(shaderProgram, "texture");

    // Get lighting locations
    u_lightPos = glGetUniformLocation(shaderProgram, "lightPosition");
    u_lightColor = glGetUniformLocation(shaderProgram, "lightColor");
    u_ambientLight = glGetUniformLocation(shaderProgram, "ambientLight");

    // Get attribute locations
    a_pos = glGetAttribLocation(shaderProgram, "position");
    a_texcoord = glGetAttribLocation(shaderProgram, "texCoord");
    a_normal = glGetAttribLocation(shaderProgram, "normal");

    // Check if all locations were found
    if (u_proj == -1 || u_view == -1 || u_model == -1 || u_normal == -1 ||
        u_color == -1 || u_tex == -1 || u_lightPos == -1 || u_lightColor == -1 ||
        u_ambientLight == -1 || a_pos == -1 || a_texcoord == -1 || a_normal == -1) {
        LOGE("Failed to get shader locations");
        glUseProgram(0);
        glDeleteProgram(shaderProgram);
        return 0;
    }

    glUseProgram(0);

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
    if (program != g_shader_basic || u_proj == -1) {
        u_proj = glGetUniformLocation(program, "projectionMatrix");
    }
    if (u_proj == -1) {
        LOGE("Failed to get projection matrix location");
        return;
    }
    glUniformMatrix4fv(u_proj, 1, GL_FALSE, matrix);
}

void setModelMatrix(GLuint program, float* matrix) {
    if (program == 0 || matrix == NULL) {
        LOGE("Invalid parameters for setModelMatrix");
        return;
    }
    if (program != g_shader_basic || u_model == -1) {
        u_model = glGetUniformLocation(program, "modelMatrix");
    }
    if (u_model == -1) {
        LOGE("Failed to get model matrix location");
        return;
    }
    glUniformMatrix4fv(u_model, 1, GL_FALSE, matrix);
}

void setViewMatrix(GLuint program, float* matrix) {
    if (program == 0 || matrix == NULL) {
        LOGE("Invalid parameters for setViewMatrix");
        return;
    }
    if (program != g_shader_basic || u_view == -1) {
        u_view = glGetUniformLocation(program, "viewMatrix");
    }
    if (u_view == -1) {
        LOGE("Failed to get view matrix location");
        return;
    }
    glUniformMatrix4fv(u_view, 1, GL_FALSE, matrix);
}

void setNormalMatrix(GLuint program, float* matrix) {
    if (program == 0 || matrix == NULL) {
        LOGE("Invalid parameters for setNormalMatrix");
        return;
    }
    if (program != g_shader_basic || u_normal == -1) {
        u_normal = glGetUniformLocation(program, "normalMatrix");
    }
    if (u_normal == -1) {
        LOGE("Failed to get normal matrix location");
        return;
    }
    glUniformMatrix4fv(u_normal, 1, GL_FALSE, matrix);
}

void setColor(GLuint program, float r, float g, float b, float a) {
    if (program == 0) {
        LOGE("Invalid shader program");
        return;
    }
    if (program != g_shader_basic || u_color == -1) {
        u_color = glGetUniformLocation(program, "color");
    }
    if (u_color == -1) {
        LOGE("Failed to get color location");
        return;
    }
    glUniform4f(u_color, r, g, b, a);
}

void setTexture(GLuint program, GLuint textureUnit) {
    if (program == 0) {
        LOGE("Invalid shader program");
        return;
    }
    if (program != g_shader_basic || u_tex == -1) {
        u_tex = glGetUniformLocation(program, "texture");
    }
    if (u_tex == -1) {
        LOGE("Failed to get texture location");
        return;
    }
    glUniform1i(u_tex, (GLint)textureUnit);
}

void setLightPosition(GLuint program, float x, float y, float z) {
    if (program == 0) {
        LOGE("Invalid shader program");
        return;
    }
    if (program != g_shader_basic || u_lightPos == -1) {
        u_lightPos = glGetUniformLocation(program, "lightPosition");
    }
    if (u_lightPos == -1) {
        LOGE("Failed to get lightPosition location");
        return;
    }
    glUniform3f(u_lightPos, x, y, z);
}

void setLightColor(GLuint program, float r, float g, float b) {
    if (program == 0) {
        LOGE("Invalid shader program");
        return;
    }
    if (program != g_shader_basic || u_lightColor == -1) {
        u_lightColor = glGetUniformLocation(program, "lightColor");
    }
    if (u_lightColor == -1) {
        LOGE("Failed to get lightColor location");
        return;
    }
    glUniform3f(u_lightColor, r, g, b);
}

void setAmbientLight(GLuint program, float r, float g, float b) {
    if (program == 0) {
        LOGE("Invalid shader program");
        return;
    }
    if (program != g_shader_basic || u_ambientLight == -1) {
        u_ambientLight = glGetUniformLocation(program, "ambientLight");
    }
    if (u_ambientLight == -1) {
        LOGE("Failed to get ambientLight location");
        return;
    }
    glUniform3f(u_ambientLight, r, g, b);
}

void init_shaders_android() {
    if (g_shader_basic != 0) return;

    g_shader_basic = createShaderProgram();
    if (!g_shader_basic) {
        LOGE("init_shaders_android: failed to create basic shader");
        return;
    }

    LOGI("init_shaders_android: basic shader created %u", (unsigned)g_shader_basic);

    // Bind once and set sampler uniform 'texture' to unit 0
    glUseProgram(g_shader_basic);

    // Set default texture unit
    GLint texLoc = glGetUniformLocation(g_shader_basic, "texture");
    if (texLoc != -1) {
        glUniform1i(texLoc, 0);
    } else {
        LOGE("init_shaders_android: sampler uniform 'texture' not found");
    }

    // Set default lighting values
    setLightPosition(g_shader_basic, 1.0f, 1.0f, 1.0f);
    setLightColor(g_shader_basic, 1.0f, 1.0f, 1.0f);
    setAmbientLight(g_shader_basic, 0.2f, 0.2f, 0.2f);

    // Set default color
    setColor(g_shader_basic, 1.0f, 1.0f, 1.0f, 1.0f);

    glUseProgram(0);
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

void ensureShaderBound() {
    if (g_shader_basic == 0) {
        init_shaders_android();
    }
    if (g_shader_basic != 0) {
        GLint currentProgram = 0;
        glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
        if (currentProgram != (GLint)g_shader_basic) {
            glUseProgram(g_shader_basic);
        }
    }
}

void resetMatrices() {
    if (g_shader_basic == 0) return;

    GLfloat identity[16] = {
        1,0,0,0,
        0,1,0,0,
        0,0,1,0,
        0,0,0,1
    };

    setModelMatrix(g_shader_basic, identity);
    setViewMatrix(g_shader_basic, identity);
    setNormalMatrix(g_shader_basic, identity);
}

void setIdentityMatrix(GLuint program, int matrixType) {
    if (program == 0) return;

    GLfloat identity[16] = {
        1,0,0,0,
        0,1,0,0,
        0,0,1,0,
        0,0,0,1
    };

    switch(matrixType) {
        case MATRIX_PROJECTION:
            setProjectionMatrix(program, identity);
            break;
        case MATRIX_VIEW:
            setViewMatrix(program, identity);
            break;
        case MATRIX_MODEL:
            setModelMatrix(program, identity);
            break;
        case MATRIX_NORMAL:
            setNormalMatrix(program, identity);
            break;
    }
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
