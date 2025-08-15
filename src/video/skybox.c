#include "video/skybox.h"
#include "video/texture.h"
#include "video/shader_manager.h"
#include "video/nebu_texture2d.h"
#include "filesystem/path.h"
#include "filesystem/nebu_filesystem.h"
#include "game/resource.h"
#include <SDL2/SDL.h>
#if defined(__ANDROID__)
  #include <GLES2/gl2.h>
#else
  #include <GL/gl.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL_image.h>

// --- Shader sources ---
static const char *skyboxVertexShaderSrc =
    "attribute vec3 aPosition;\n"
    "attribute vec2 aTexCoord;\n"
    "varying vec2 vTexCoord;\n"
    "uniform mat4 uMVP;\n"
    "void main() {\n"
    "  gl_Position = uMVP * vec4(aPosition, 1.0);\n"
    "  vTexCoord = aTexCoord;\n"
    "}\n";

static const char *skyboxFragmentShaderSrc =
    "precision mediump float;\n"
    "varying vec2 vTexCoord;\n"
    "uniform sampler2D uTexture;\n"
    "void main() {\n"
    "  gl_FragColor = texture2D(uTexture, vTexCoord);\n"
    "}\n";

// --- Shader utilities ---
static GLuint compileShader(GLenum type, const char *src) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);
    GLint ok;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[256];
        glGetShaderInfoLog(shader, sizeof(log), NULL, log);
        fprintf(stderr, "Shader compile error: %s\n", log);
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

static GLuint createProgram(const char *vs, const char *fs) {
    GLuint v = compileShader(GL_VERTEX_SHADER, vs);
    GLuint f = compileShader(GL_FRAGMENT_SHADER, fs);
    GLuint prog = glCreateProgram();
    glAttachShader(prog, v);
    glAttachShader(prog, f);
    glLinkProgram(prog);
    GLint ok;
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[256];
        glGetProgramInfoLog(prog, sizeof(log), NULL, log);
        fprintf(stderr, "Program link error: %s\n", log);
        glDeleteProgram(prog);
        return 0;
    }
    glDeleteShader(v);
    glDeleteShader(f);
    return prog;
}

// --- Cube vertex data (positions and texcoords for 6 faces) ---
static const GLfloat skyboxVertices[] = {
    // 6 faces, 4 vertices per face, 3 position + 2 texcoord = 5 floats per vertex
    // Front face (+Z)
    1,  1,  1, 1, 1,
   -1,  1,  1, 0, 1,
   -1, -1,  1, 0, 0,
    1, -1,  1, 1, 0,
    // Back face (-Z)
   -1,  1, -1, 1, 1,
    1,  1, -1, 0, 1,
    1, -1, -1, 0, 0,
   -1, -1, -1, 1, 0,
    // Left face (-X)
   -1,  1,  1, 1, 1,
   -1,  1, -1, 0, 1,
   -1, -1, -1, 0, 0,
   -1, -1,  1, 1, 0,
    // Right face (+X)
    1,  1, -1, 1, 1,
    1,  1,  1, 0, 1,
    1, -1,  1, 0, 0,
    1, -1, -1, 1, 0,
    // Top face (+Y)
    1,  1, -1, 1, 1,
   -1,  1, -1, 0, 1,
   -1,  1,  1, 0, 0,
    1,  1,  1, 1, 0,
    // Bottom face (-Y)
    1, -1,  1, 1, 1,
   -1, -1,  1, 0, 1,
   -1, -1, -1, 0, 0,
    1, -1, -1, 1, 0,
};

static const GLushort skyboxIndices[] = {
    0, 1, 2, 0, 2, 3,       // Front
    4, 5, 6, 4, 6, 7,       // Back
    8, 9,10, 8,10,11,       // Left
   12,13,14,12,14,15,       // Right
   16,17,18,16,18,19,       // Top
   20,21,22,20,22,23        // Bottom
};

// --- Skybox state ---
typedef struct {
    GLuint textures[6]; // One texture per face
    GLuint vao, vbo, ebo;
    GLuint shaderProg;
    GLint aPosition, aTexCoord, uMVP, uTexture;
} ModernSkybox;

// --- Initialize skybox (call once) ---
void initModernSkybox(ModernSkybox *skybox) {
    // Use the shader program from shader_manager
    skybox->shaderProg = gSkyboxShaderProgram;
    skybox->aPosition = gSkyboxAttribPosition;
    skybox->uMVP = gSkyboxUniformMVP;
    skybox->uTexture = gSkyboxUniformSkybox;

    // Create VAO/VBO/EBO
    #ifdef __ANDROID__
    // For OpenGL ES 2.0, we need to use VAOs
    glGenVertexArrays(1, &skybox->vao);
    glBindVertexArray(skybox->vao);
    #endif

    glGenBuffers(1, &skybox->vbo);
    glGenBuffers(1, &skybox->ebo);

    glBindBuffer(GL_ARRAY_BUFFER, skybox->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skybox->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(skyboxIndices), skyboxIndices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(skybox->aPosition);
    glVertexAttribPointer(skybox->aPosition, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)0);

    #ifdef __ANDROID__
    glBindVertexArray(0);
    #endif
}

// --- Load skybox textures using TextureInfo ---
void loadSkyboxTextures(Skybox *skybox, const TextureInfo textures[6]) {
    // Create a cube map texture
    glGenTextures(1, &((ModernSkybox*)skybox)->textures[0]);
    glBindTexture(GL_TEXTURE_CUBE_MAP, ((ModernSkybox*)skybox)->textures[0]);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    // Load each face of the cube map
    for (int i = 0; i < 6; i++) {
        // Get the full path to the texture file
        char* path = nebu_FS_GetPath_WithFilename(PATH_ART, textures[i].filename);
        if (!path) {
            fprintf(stderr, "[FATAL] Failed to locate skybox texture: %s\n", textures[i].filename);
            exit(EXIT_FAILURE);
        }

        // Load the texture data
        SDL_Surface *surface = IMG_Load(path);
        if (!surface) {
            fprintf(stderr, "[FATAL] Failed to load skybox texture: %s\n", path);
            free(path);
            exit(EXIT_FAILURE);
        }

        // Convert to RGBA format
        SDL_Surface *rgbaSurface = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32, 0);
        SDL_FreeSurface(surface);

        // Upload to GPU
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA,
                     rgbaSurface->w, rgbaSurface->h, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, rgbaSurface->pixels);

        SDL_FreeSurface(rgbaSurface);
        free(path);
    }

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

// --- Draw skybox (call each frame) ---
void drawModernSkybox(ModernSkybox *skybox, const float *mvp) {
    glUseProgram(skybox->shaderProg);
    glUniformMatrix4fv(skybox->uMVP, 1, GL_FALSE, mvp);

    glBindVertexArray(skybox->vao);

    // Draw each face with its own texture
    for (int i = 0; i < 6; i++) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, skybox->textures[i]);
        glUniform1i(skybox->uTexture, 0);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (void*)(i * 6 * sizeof(GLushort)));
    }

    glBindVertexArray(0);
    glUseProgram(0);
}

// Skybox wrappers
void drawSkybox(Skybox *skybox) {
    // Create and set up the MVP matrix
    float mvp[16];

    // For a skybox, we typically want to use the view matrix without translation
    // and combine it with the projection matrix

    // Get the current view matrix
    GLfloat view[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, view);

    // Remove translation from the view matrix
    view[12] = view[13] = view[14] = 0.0f;

    // Get the current projection matrix
    GLfloat projection[16];
    glGetFloatv(GL_PROJECTION_MATRIX, projection);

    // Combine the projection and view matrices to create the MVP matrix
    // Note: In OpenGL, matrix multiplication is right-to-left
    // So we multiply projection * view to get the final MVP matrix
    matrixMultiply(projection, view, mvp);

    // Use the skybox shader program
    glUseProgram(skybox->shaderProg);

    // Set the MVP matrix uniform
    glUniformMatrix4fv(skybox->uMVP, 1, GL_FALSE, mvp);

    // Bind the skybox texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skybox->textures[0]);
    glUniform1i(skybox->uSkybox, 0);

    // Enable the position attribute
    glEnableVertexAttribArray(skybox->aPosition);

    // Bind the VAO and draw the skybox
    glBindVertexArray(skybox->vao);
    glDrawElements(GL_TRIANGLES, sizeof(skyboxIndices)/sizeof(skyboxIndices[0]), GL_UNSIGNED_SHORT, 0);

    // Disable the attribute when done
    glDisableVertexAttribArray(skybox->aPosition);

    // Unbind the shader program when done
    glUseProgram(0);
}

void renderSkybox(Skybox *skybox) {
    // Use the skybox shader program
    glUseProgram(skybox->shaderProg);

    // Set the MVP matrix uniform
    float mvp[16];
    // Set up your MVP matrix here (identity matrix for simplicity)
    for (int i = 0; i < 16; i++) {
        mvp[i] = (i % 5 == 0) ? 1.0f : 0.0f; // Simple identity matrix
    }
    glUniformMatrix4fv(skybox->uMVP, 1, GL_FALSE, mvp);

    // Bind the skybox texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skybox->textures[0]);
    glUniform1i(skybox->uSkybox, 0);

    // Enable the position attribute
    glEnableVertexAttribArray(skybox->aPosition);

    // Set up your vertex data and draw calls here
    // ...

    // Disable the attribute when done
    glDisableVertexAttribArray(skybox->aPosition);

    // Unbind the shader program when done
    glUseProgram(0);
}
