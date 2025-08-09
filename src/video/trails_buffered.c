#include <SDL2/SDL.h>
#ifdef __ANDROID__
  #include <GLES2/gl2.h>
#else
  #include <GL/glew.h>
  #include <GL/gl.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "game/game.h"
#include "video/video.h"

// --- Shader sources ---
static const char *vertexShaderSrc =
    "attribute vec3 aPosition;\n"
    "attribute vec4 aColor;\n"
    "attribute vec2 aTexCoord;\n"
    "varying vec4 vColor;\n"
    "varying vec2 vTexCoord;\n"
    "uniform mat4 uMVP;\n"
    "void main() {\n"
    "  gl_Position = uMVP * vec4(aPosition, 1.0);\n"
    "  vColor = aColor;\n"
    "  vTexCoord = aTexCoord;\n"
    "}\n";

static const char *fragmentShaderSrc =
    "precision mediump float;\n"
    "varying vec4 vColor;\n"
    "varying vec2 vTexCoord;\n"
    "uniform sampler2D uTexture;\n"
    "uniform float uUseTexture;\n"
    "void main() {\n"
    "  vec4 color = vColor;\n"
    "  if (uUseTexture > 0.5)\n"
    "    color *= texture2D(uTexture, vTexCoord);\n"
    "  gl_FragColor = color;\n"
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

// --- MVP matrix utility ---
static void ortho_matrix(float *out, float left, float right, float bottom, float top, float near, float far) {
    memset(out, 0, sizeof(float) * 16);
    out[0] = 2.0f / (right - left);
    out[5] = 2.0f / (top - bottom);
    out[10] = -2.0f / (far - near);
    out[12] = -(right + left) / (right - left);
    out[13] = -(top + bottom) / (top - bottom);
    out[14] = -(far + near) / (far - near);
    out[15] = 1.0f;
}

// --- Trail quad structure ---
typedef struct {
    float position[3];
    float color[4];
    float texcoord[2];
} TrailVertex;

// --- Trail buffer (dynamic array of quads) ---
typedef struct {
    TrailVertex *vertices;
    GLushort *indices;
    int maxQuads;
    int numQuads;
} TrailBuffer;

// --- Create and free trail buffer ---
TrailBuffer* createTrailBuffer(int maxQuads) {
    TrailBuffer *tb = (TrailBuffer*)calloc(1, sizeof(TrailBuffer));
    tb->vertices = (TrailVertex*)malloc(sizeof(TrailVertex) * 4 * maxQuads);
    tb->indices = (GLushort*)malloc(sizeof(GLushort) * 6 * maxQuads);
    tb->maxQuads = maxQuads;
    tb->numQuads = 0;
    return tb;
}
void freeTrailBuffer(TrailBuffer *tb) {
    if (!tb) return;
    free(tb->vertices);
    free(tb->indices);
    free(tb);
}

// --- Add a quad to the buffer ---
void addTrailQuad(TrailBuffer *tb, TrailVertex v[4]) {
    if (tb->numQuads >= tb->maxQuads) return;
    int base = tb->numQuads * 4;
    memcpy(&tb->vertices[base], v, sizeof(TrailVertex) * 4);
    int idx = tb->numQuads * 6;
    tb->indices[idx+0] = base+0;
    tb->indices[idx+1] = base+1;
    tb->indices[idx+2] = base+2;
    tb->indices[idx+3] = base+0;
    tb->indices[idx+4] = base+2;
    tb->indices[idx+5] = base+3;
    tb->numQuads++;
}

// --- Modern trail draw ---
void drawTrailBuffer_modern(TrailBuffer *tb, GLuint tex_id, int win_w, int win_h, int useTexture) {
    static GLuint prog = 0;
    static GLint aPosition = -1, aColor = -1, aTexCoord = -1, uMVP = -1, uTexture = -1, uUseTexture = -1;
    if (!prog) {
        prog = createProgram(vertexShaderSrc, fragmentShaderSrc);
        aPosition = glGetAttribLocation(prog, "aPosition");
        aColor = glGetAttribLocation(prog, "aColor");
        aTexCoord = glGetAttribLocation(prog, "aTexCoord");
        uMVP = glGetUniformLocation(prog, "uMVP");
        uTexture = glGetUniformLocation(prog, "uTexture");
        uUseTexture = glGetUniformLocation(prog, "uUseTexture");
    }

    float mvp[16];
    ortho_matrix(mvp, 0, (float)win_w, 0, (float)win_h, -1, 1);

    glUseProgram(prog);
    glUniformMatrix4fv(uMVP, 1, GL_FALSE, mvp);
    glUniform1f(uUseTexture, useTexture ? 1.0f : 0.0f);

    if (useTexture) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex_id);
        glUniform1i(uTexture, 0);
    }

    glEnableVertexAttribArray(aPosition);
    glEnableVertexAttribArray(aColor);
    glEnableVertexAttribArray(aTexCoord);

    glVertexAttribPointer(aPosition, 3, GL_FLOAT, GL_FALSE, sizeof(TrailVertex), &tb->vertices[0].position);
    glVertexAttribPointer(aColor, 4, GL_FLOAT, GL_FALSE, sizeof(TrailVertex), &tb->vertices[0].color);
    glVertexAttribPointer(aTexCoord, 2, GL_FLOAT, GL_FALSE, sizeof(TrailVertex), &tb->vertices[0].texcoord);

    glDrawElements(GL_TRIANGLES, tb->numQuads * 6, GL_UNSIGNED_SHORT, tb->indices);

    glDisableVertexAttribArray(aPosition);
    glDisableVertexAttribArray(aColor);
    glDisableVertexAttribArray(aTexCoord);

    if (useTexture) glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
}

// --- Example: Buffering a player's trail (replace with your logic) ---
void bufferPlayerTrailModern(Player *p, TrailBuffer *tb, int tex_id) {
    // For each trail segment, fill TrailVertex v[4] and call addTrailQuad(tb, v);
    // This is a placeholder; adapt your trail logic here.
    // Example: a single vertical quad at (x, y)
    float x = 100, y = 100, h = 50;
    TrailVertex v[4];
    for (int i = 0; i < 4; ++i) {
        memset(&v[i], 0, sizeof(TrailVertex));
        v[i].color[0] = 1.0f; v[i].color[1] = 1.0f; v[i].color[2] = 1.0f; v[i].color[3] = 1.0f;
    }
    v[0].position[0] = x;   v[0].position[1] = y;   v[0].position[2] = 0;   v[0].texcoord[0] = 0; v[0].texcoord[1] = 0;
    v[1].position[0] = x+10;v[1].position[1] = y;   v[1].position[2] = 0;   v[1].texcoord[0] = 1; v[1].texcoord[1] = 0;
    v[2].position[0] = x+10;v[2].position[1] = y;   v[2].position[2] = h;   v[2].texcoord[0] = 1; v[2].texcoord[1] = 1;
    v[3].position[0] = x;   v[3].position[1] = y;   v[3].position[2] = h;   v[3].texcoord[0] = 0; v[3].texcoord[1] = 1;
    addTrailQuad(tb, v);
}

// --- Main function to buffer and draw all trails ---
void doTrailsModern(Player *players, int nPlayers, int tex_id, int win_w, int win_h) {
    int maxQuads = 1024; // Adjust as needed
    TrailBuffer *tb = createTrailBuffer(maxQuads);

    for (int i = 0; i < nPlayers; ++i) {
        bufferPlayerTrailModern(&players[i], tb, tex_id);
    }

    drawTrailBuffer_modern(tb, tex_id, win_w, win_h, 1);

    freeTrailBuffer(tb);
}
