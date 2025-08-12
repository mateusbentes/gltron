#include <SDL2/SDL.h>
#if defined(__ANDROID__)
  #include <GLES2/gl2.h>
#else
  #include <GL/gl.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    // Compile/link shaders
    skybox->shaderProg = createProgram(skyboxVertexShaderSrc, skyboxFragmentShaderSrc);
    skybox->aPosition = glGetAttribLocation(skybox->shaderProg, "aPosition");
    skybox->aTexCoord = glGetAttribLocation(skybox->shaderProg, "aTexCoord");
    skybox->uMVP = glGetUniformLocation(skybox->shaderProg, "uMVP");
    skybox->uTexture = glGetUniformLocation(skybox->shaderProg, "uTexture");

    // Create VAO/VBO/EBO
    glGenVertexArrays(1, &skybox->vao);
    glGenBuffers(1, &skybox->vbo);
    glGenBuffers(1, &skybox->ebo);

    glBindVertexArray(skybox->vao);
    glBindBuffer(GL_ARRAY_BUFFER, skybox->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skybox->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(skyboxIndices), skyboxIndices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(skybox->aPosition);
    glVertexAttribPointer(skybox->aPosition, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)0);

    glEnableVertexAttribArray(skybox->aTexCoord);
    glVertexAttribPointer(skybox->aTexCoord, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));

    glBindVertexArray(0);
}

// --- Load skybox textures (one per face) ---
void loadModernSkyboxTextures(ModernSkybox *skybox, const char* filenames[6], GLuint (*loadTextureFunc)(const char*)) {
    for (int i = 0; i < 6; i++) {
        skybox->textures[i] = loadTextureFunc(filenames[i]);
        if (!skybox->textures[i]) {
            fprintf(stderr, "[FATAL] Failed to load skybox texture: %s\n", filenames[i]);
            exit(EXIT_FAILURE);
        }
    }
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
void drawSkybox(void) {
    #ifdef HAVE_DRAWSKYBOX_MODERN
    drawSkybox_modern();
    #else
    fprintf(stderr, "[WARN] drawSkybox: Not implemented.\n");
    #endif
}

void loadSkyboxTextures(void) {
    #ifdef HAVE_LOADSKYBOXTEXTURES_MODERN
    loadSkyboxTextures_modern();
    #else
    fprintf(stderr, "[WARN] loadSkyboxTextures: Not implemented.\n");
    #endif
}