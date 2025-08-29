#include "shaders.h"

#ifdef ANDROID
#include <android/log.h>
#include <stdlib.h>
#include <string.h>
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "GLTron", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "GLTron", __VA_ARGS__)

// Centralized program storage
static GLuint g_shader_unified = 0;
// Cached uniform/attrib locations for unified shader
static GLint u_proj = -1;
static GLint u_view = -1;
static GLint u_model = -1;
static GLint u_normal = -1;
static GLint u_color = -1;
static GLint u_tex = -1;
static GLint u_lightPos = -1;
static GLint u_lightColor = -1;
static GLint u_ambientLight = -1;
static GLint u_is2D = -1;  // New uniform for 2D/3D mode switching
static GLint a_pos = -1;
static GLint a_texcoord = -1;
static GLint a_normal = -1;

// Unified vertex shader that handles both 2D and 3D
static const char* vertexShaderSource =
    "attribute vec3 position;\n"
    "attribute vec2 texCoord;\n"
    "attribute vec3 normal;\n"
    "uniform mat4 projectionMatrix;\n"
    "uniform mat4 modelMatrix;\n"
    "uniform mat4 viewMatrix;\n"
    "uniform mat4 normalMatrix;\n"
    "uniform bool is2D;\n"
    "varying vec2 vTexCoord;\n"
    "varying vec3 vNormal;\n"
    "varying vec3 vPosition;\n"
    "varying float vIs2D;\n"
    "void main()\n"
    "{\n"
    "    vTexCoord = texCoord;\n"
    "    vIs2D = is2D ? 1.0 : 0.0;\n"
    "    \n"
    "    if (is2D) {\n"
    "        // 2D mode: no lighting calculations needed\n"
    "        vNormal = vec3(0.0, 0.0, 1.0);\n"
    "        vPosition = vec3(0.0);\n"
    "        gl_Position = projectionMatrix * modelMatrix * vec4(position, 1.0);\n"
    "    } else {\n"
    "        // 3D mode: full lighting calculations\n"
    "        vNormal = mat3(normalMatrix) * normal;\n"
    "        vPosition = vec3(modelMatrix * vec4(position, 1.0));\n"
    "        gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(position, 1.0);\n"
    "    }\n"
    "}\n";

// Unified fragment shader that handles both 2D and 3D
static const char* fragmentShaderSource =
    "precision mediump float;\n"
    "varying vec2 vTexCoord;\n"
    "varying vec3 vNormal;\n"
    "varying vec3 vPosition;\n"
    "varying float vIs2D;\n"
    "uniform sampler2D texture;\n"
    "uniform vec4 color;\n"
    "uniform vec3 lightPosition;\n"
    "uniform vec3 lightColor;\n"
    "uniform vec3 ambientLight;\n"
    "void main()\n"
    "{\n"
    "    vec4 texColor = texture2D(texture, vTexCoord);\n"
    "    \n"
    "    if (vIs2D > 0.5) {\n"
    "        // 2D mode: simple color * texture, no lighting\n"
    "        gl_FragColor = color * texColor;\n"
    "    } else {\n"
    "        // 3D mode: full lighting calculations\n"
    "        vec3 normal = normalize(vNormal);\n"
    "        vec3 lightDir = normalize(lightPosition - vPosition);\n"
    "        float diffuse = max(dot(normal, lightDir), 0.0);\n"
    "        vec3 lighting = ambientLight + lightColor * diffuse;\n"
    "        gl_FragColor = vec4(color.rgb * texColor.rgb * lighting, color.a * texColor.a);\n"
    "    }\n"
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

static GLuint createUnifiedShaderProgram() {
    // Reset cached locations
    u_proj = u_view = u_model = u_normal = u_color = u_tex = -1;
    u_lightPos = u_lightColor = u_ambientLight = u_is2D = -1;
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

    // Get 2D mode flag location
    u_is2D = glGetUniformLocation(shaderProgram, "is2D");

    // Get attribute locations
    a_pos = glGetAttribLocation(shaderProgram, "position");
    a_texcoord = glGetAttribLocation(shaderProgram, "texCoord");
    a_normal = glGetAttribLocation(shaderProgram, "normal");

    // Check if all critical locations were found
    if (u_proj == -1 || u_color == -1 || u_tex == -1 || u_is2D == -1 ||
        a_pos == -1 || a_texcoord == -1) {
        LOGE("Failed to get critical shader locations");
        glUseProgram(0);
        glDeleteProgram(shaderProgram);
        return 0;
    }

    // Note: u_normal, lighting uniforms, and a_normal can be -1 for 2D-only usage
    // but we'll warn if they're missing
    if (u_view == -1 || u_model == -1 || u_normal == -1 ||
        u_lightPos == -1 || u_lightColor == -1 || u_ambientLight == -1 || a_normal == -1) {
        LOGI("Some 3D shader locations not found - 3D rendering may not work properly");
    }

    glUseProgram(0);
    return shaderProgram;
}

// New function to set 2D/3D mode
void setRenderMode2D(GLuint program, int is2D) {
    if (program == 0) {
        LOGE("Invalid shader program");
        return;
    }
    if (program != g_shader_unified || u_is2D == -1) {
        u_is2D = glGetUniformLocation(program, "is2D");
    }
    if (u_is2D == -1) {
        LOGE("Failed to get is2D location");
        return;
    }
    glUniform1i(u_is2D, is2D ? 1 : 0);
}

// Updated matrix functions with better 2D/3D handling
void setProjectionMatrix(GLuint program, float* matrix) {
    if (program == 0 || matrix == NULL) {
        LOGE("Invalid parameters for setProjectionMatrix");
        return;
    }
    if (program != g_shader_unified || u_proj == -1) {
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
    if (program != g_shader_unified || u_model == -1) {
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
    if (program != g_shader_unified || u_view == -1) {
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

    // Calculate the normal matrix from the model matrix
    // The normal matrix is the inverse transpose of the upper-left 3x3 of the model matrix
    float normalMatrix[9] = {
        matrix[0], matrix[1], matrix[2],  // First row of model matrix
        matrix[4], matrix[5], matrix[6],  // Second row of model matrix
        matrix[8], matrix[9], matrix[10]  // Third row of model matrix
    };

    // Calculate the determinant of the 3x3 matrix
    float det = normalMatrix[0] * (normalMatrix[4] * normalMatrix[8] - normalMatrix[5] * normalMatrix[7]) -
                normalMatrix[1] * (normalMatrix[3] * normalMatrix[8] - normalMatrix[5] * normalMatrix[6]) +
                normalMatrix[2] * (normalMatrix[3] * normalMatrix[7] - normalMatrix[4] * normalMatrix[6]);

    // Calculate the inverse of the 3x3 matrix
    float invDet = 1.0f / det;
    float invNormalMatrix[9] = {
        (normalMatrix[4] * normalMatrix[8] - normalMatrix[5] * normalMatrix[7]) * invDet,
        -(normalMatrix[1] * normalMatrix[8] - normalMatrix[2] * normalMatrix[7]) * invDet,
        (normalMatrix[1] * normalMatrix[5] - normalMatrix[2] * normalMatrix[4]) * invDet,
        -(normalMatrix[3] * normalMatrix[8] - normalMatrix[5] * normalMatrix[6]) * invDet,
        (normalMatrix[0] * normalMatrix[8] - normalMatrix[2] * normalMatrix[6]) * invDet,
        -(normalMatrix[0] * normalMatrix[5] - normalMatrix[2] * normalMatrix[3]) * invDet,
        (normalMatrix[3] * normalMatrix[7] - normalMatrix[4] * normalMatrix[6]) * invDet,
        -(normalMatrix[0] * normalMatrix[7] - normalMatrix[1] * normalMatrix[6]) * invDet,
        (normalMatrix[0] * normalMatrix[4] - normalMatrix[1] * normalMatrix[3]) * invDet
    };

    // Transpose the inverse matrix to get the normal matrix
    float finalNormalMatrix[9] = {
        invNormalMatrix[0], invNormalMatrix[3], invNormalMatrix[6],
        invNormalMatrix[1], invNormalMatrix[4], invNormalMatrix[7],
        invNormalMatrix[2], invNormalMatrix[5], invNormalMatrix[8]
    };

    // Convert the 3x3 matrix to a 4x4 matrix for OpenGL
    float normalMatrix4x4[16] = {
        finalNormalMatrix[0], finalNormalMatrix[1], finalNormalMatrix[2], 0.0f,
        finalNormalMatrix[3], finalNormalMatrix[4], finalNormalMatrix[5], 0.0f,
        finalNormalMatrix[6], finalNormalMatrix[7], finalNormalMatrix[8], 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };

    // Set the normal matrix in the shader
    if (program != g_shader_unified || u_normal == -1) {
        u_normal = glGetUniformLocation(program, "normalMatrix");
    }
    if (u_normal == -1) {
        LOGE("Failed to get normal matrix location");
        return;
    }
    glUniformMatrix4fv(u_normal, 1, GL_FALSE, normalMatrix4x4);
}

void setColor(GLuint program, float r, float g, float b, float a) {
    if (program == 0) {
        LOGE("Invalid shader program");
        return;
    }
    if (program != g_shader_unified || u_color == -1) {
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
    if (program != g_shader_unified || u_tex == -1) {
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
    if (program != g_shader_unified || u_lightPos == -1) {
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
    if (program != g_shader_unified || u_lightColor == -1) {
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
    if (program != g_shader_unified || u_ambientLight == -1) {
        u_ambientLight = glGetUniformLocation(program, "ambientLight");
    }
    if (u_ambientLight == -1) {
        LOGE("Failed to get ambientLight location");
        return;
    }
    glUniform3f(u_ambientLight, r, g, b);
}

void init_shaders_android() {
    if (g_shader_unified != 0) return;

    g_shader_unified = createUnifiedShaderProgram();
    if (!g_shader_unified) {
        LOGE("init_shaders_android: failed to create unified shader");
        return;
    }

    LOGI("init_shaders_android: unified shader created %u", (unsigned)g_shader_unified);

    // Bind once and set defaults
    glUseProgram(g_shader_unified);

    // Set default texture unit
    GLint texLoc = glGetUniformLocation(g_shader_unified, "texture");
    if (texLoc != -1) {
        glUniform1i(texLoc, 0);
    }

    // Set default to 3D mode
    setRenderMode2D(g_shader_unified, 0);

    // Set default lighting values
    setLightPosition(g_shader_unified, 1.0f, 1.0f, 1.0f);
    setLightColor(g_shader_unified, 1.0f, 1.0f, 1.0f);
    setAmbientLight(g_shader_unified, 0.2f, 0.2f, 0.2f);

    // Set default color
    setColor(g_shader_unified, 1.0f, 1.0f, 1.0f, 1.0f);

    glUseProgram(0);
}

void shutdown_shaders_android() {
    if (g_shader_unified) {
        glDeleteProgram(g_shader_unified);
        g_shader_unified = 0;
    }
}

GLuint shader_get_unified() {
    return g_shader_unified;
}

GLuint shader_get_basic() {
    return g_shader_unified;  // Backward compatibility
}

void ensureShaderBound() {
    if (g_shader_unified == 0) {
        init_shaders_android();
    }
    if (g_shader_unified != 0) {
        GLint currentProgram = 0;
        glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
        if (currentProgram != (GLint)g_shader_unified) {
            glUseProgram(g_shader_unified);
        }
    }
}

// Helper functions for common use cases
void setup2DRendering() {
    ensureShaderBound();
    setRenderMode2D(g_shader_unified, 1);
    
    // Set up orthographic projection for 2D
    GLfloat orthoMatrix[16] = {
        2.0f/800.0f, 0, 0, 0,  // Assuming 800x600 screen, adjust as needed
        0, 2.0f/600.0f, 0, 0,
        0, 0, -1, 0,
        -1, -1, 0, 1
    };
    setProjectionMatrix(g_shader_unified, orthoMatrix);
    
    GLfloat identity[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
    setModelMatrix(g_shader_unified, identity);
}

void setup3DRendering() {
    ensureShaderBound();
    setRenderMode2D(g_shader_unified, 0);
}

void useShaderProgram(GLuint program) {
    if (program == 0) {
        LOGE("Invalid shader program");
        return;
    }
    glUseProgram(program);
}

void resetMatrices() {
    if (g_shader_unified == 0) return;

    GLfloat identity[16] = {
        1,0,0,0,
        0,1,0,0,
        0,0,1,0,
        0,0,0,1
    };

    setModelMatrix(g_shader_unified, identity);
    setViewMatrix(g_shader_unified, identity);
    setNormalMatrix(g_shader_unified, identity);
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

// Improved font texture creation
GLuint createFontTexture() {
    // Create a simple 8x8 bitmap for each ASCII character (32-127)
    // This is still simplified - in practice you'd load a proper font atlas
    
    const int CHARS_PER_ROW = 16;
    const int CHAR_ROWS = 6; // 96 printable ASCII chars / 16 = 6 rows
    const int CHAR_SIZE = 8;
    const int TEXTURE_WIDTH = CHARS_PER_ROW * CHAR_SIZE;
    const int TEXTURE_HEIGHT = CHAR_ROWS * CHAR_SIZE;
    
    unsigned char* fontData = (unsigned char*)malloc(TEXTURE_WIDTH * TEXTURE_HEIGHT * 4);
    
    // Fill with transparent black
    memset(fontData, 0, TEXTURE_WIDTH * TEXTURE_HEIGHT * 4);
    
    // Create simple patterns for some basic characters
    // This is very basic - you'd normally load actual font data
    for (int row = 0; row < CHAR_ROWS; row++) {
        for (int col = 0; col < CHARS_PER_ROW; col++) {
            int charCode = 32 + row * CHARS_PER_ROW + col;
            
            // Simple pattern for demonstration
            for (int y = 1; y < CHAR_SIZE-1; y++) {
                for (int x = 1; x < CHAR_SIZE-1; x++) {
                    int pixelX = col * CHAR_SIZE + x;
                    int pixelY = row * CHAR_SIZE + y;
                    int pixelIndex = (pixelY * TEXTURE_WIDTH + pixelX) * 4;
                    
                    // Create a simple border for visible characters
                    if (charCode >= 33 && charCode <= 126) {
                        if (x == 1 || x == CHAR_SIZE-2 || y == 1 || y == CHAR_SIZE-2) {
                            fontData[pixelIndex] = 255;     // R
                            fontData[pixelIndex+1] = 255;   // G
                            fontData[pixelIndex+2] = 255;   // B
                            fontData[pixelIndex+3] = 255;   // A
                        }
                    }
                }
            }
        }
    }

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, fontData);
    
    free(fontData);
    return texture;
}

#endif // __ANDROID__
