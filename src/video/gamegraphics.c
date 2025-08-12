/*
 * Modernized gamegraphics.c for SDL2 + OpenGL ES 2.0+
 * - No fixed-function OpenGL (no glBegin/glEnd, glMatrixMode, glEnable(GL_LIGHTING), etc.)
 * - Uses shaders, VBOs, explicit MVP matrices
 * - Modular: skybox, floor, players, trails, explosions
 * - SDL2 for window/context management
 * - All rendering code is OpenGL ES 2.0+ compatible
 */

#include <SDL2/SDL.h>
#if defined(__ANDROID__)
  #include <GLES2/gl2.h>
#else
  #include <GL/gl.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "game/game.h"
#include "video/video.h"
#include "video/trail_geometry.h"
#include "video/skybox.h"
#include "video/explosion.h"
#include "video/graphics_fx.h"
#include "video/graphics_hud.h"
#include "video/nebu_texture2d.h"
#include "configuration/settings.h"
#include "base/nebu_vector.h"
#include "base/nebu_math.h"

// --- Shader sources (basic color/texture) ---
static const char *basicVertexShaderSrc =
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

static const char *basicFragmentShaderSrc =
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

// --- MVP matrix utility (column-major, OpenGL style) ---
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

static void identity_matrix(float *out) {
    memset(out, 0, sizeof(float) * 16);
    out[0] = out[5] = out[10] = out[15] = 1.0f;
}

// --- Global shader state ---
static struct {
    GLuint prog;
    GLint aPosition, aColor, aTexCoord;
    GLint uMVP, uTexture, uUseTexture;
} basicShader = {0};

// --- Initialize basic shader (call once) ---
static void initBasicShader() {
    if (basicShader.prog) return;
    basicShader.prog = createProgram(basicVertexShaderSrc, basicFragmentShaderSrc);
    basicShader.aPosition = glGetAttribLocation(basicShader.prog, "aPosition");
    basicShader.aColor = glGetAttribLocation(basicShader.prog, "aColor");
    basicShader.aTexCoord = glGetAttribLocation(basicShader.prog, "aTexCoord");
    basicShader.uMVP = glGetUniformLocation(basicShader.prog, "uMVP");
    basicShader.uTexture = glGetUniformLocation(basicShader.prog, "uTexture");
    basicShader.uUseTexture = glGetUniformLocation(basicShader.prog, "uUseTexture");
}

// --- Vertex structure for all geometry ---
typedef struct {
    float position[3];
    float color[4];
    float texcoord[2];
} Vertex;

// --- Draw a colored quad (floor, wall, etc.) ---
static void drawQuad(float *mvp, float x0, float y0, float x1, float y1, float z, float r, float g, float b, float a) {
    Vertex verts[4] = {
        {{x0, y0, z}, {r, g, b, a}, {0, 0}},
        {{x1, y0, z}, {r, g, b, a}, {1, 0}},
        {{x1, y1, z}, {r, g, b, a}, {1, 1}},
        {{x0, y1, z}, {r, g, b, a}, {0, 1}}
    };
    GLushort indices[] = {0, 1, 2, 0, 2, 3};

    glUseProgram(basicShader.prog);
    glUniformMatrix4fv(basicShader.uMVP, 1, GL_FALSE, mvp);
    glUniform1f(basicShader.uUseTexture, 0.0f);

    glEnableVertexAttribArray(basicShader.aPosition);
    glEnableVertexAttribArray(basicShader.aColor);
    glEnableVertexAttribArray(basicShader.aTexCoord);

    glVertexAttribPointer(basicShader.aPosition, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), &verts[0].position);
    glVertexAttribPointer(basicShader.aColor, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), &verts[0].color);
    glVertexAttribPointer(basicShader.aTexCoord, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), &verts[0].texcoord);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);

    glDisableVertexAttribArray(basicShader.aPosition);
    glDisableVertexAttribArray(basicShader.aColor);
    glDisableVertexAttribArray(basicShader.aTexCoord);

    glUseProgram(0);
}

// --- Draw the floor ---
static void drawFloor(float *mvp) {
    drawQuad(mvp, -100.0f, -100.0f, 100.0f, 100.0f, 0.0f, 0.5f, 0.5f, 0.5f, 1.0f);
}

// --- Draw arena walls ---
static void drawArenaWalls(float *mvp) {
    // Front wall
    drawQuad(mvp, -100.0f, 100.0f, 100.0f, 100.0f, 0.0f, 0.7f, 0.7f, 0.7f, 1.0f);
    drawQuad(mvp, -100.0f, 100.0f, 100.0f, 100.0f, 10.0f, 0.7f, 0.7f, 0.7f, 1.0f);
    // Back wall
    drawQuad(mvp, -100.0f, -100.0f, 100.0f, -100.0f, 0.0f, 0.7f, 0.7f, 0.7f, 1.0f);
    drawQuad(mvp, -100.0f, -100.0f, 100.0f, -100.0f, 10.0f, 0.7f, 0.7f, 0.7f, 1.0f);
    // Left wall
    drawQuad(mvp, -100.0f, -100.0f, -100.0f, 100.0f, 0.0f, 0.7f, 0.7f, 0.7f, 1.0f);
    drawQuad(mvp, -100.0f, -100.0f, -100.0f, 100.0f, 10.0f, 0.7f, 0.7f, 0.7f, 1.0f);
    // Right wall
    drawQuad(mvp, 100.0f, -100.0f, 100.0f, 100.0f, 0.0f, 0.7f, 0.7f, 0.7f, 1.0f);
    drawQuad(mvp, 100.0f, -100.0f, 100.0f, 100.0f, 10.0f, 0.7f, 0.7f, 0.7f, 1.0f);
}

// --- Draw a simple player as a colored box ---
static void drawPlayerBox(float *mvp, float x, float y, float z, float angle, float r, float g, float b) {
    // Centered at (x, y, z), rotated by angle (degrees) around Z
    // Box size: 4x8x2
    float s = sinf(angle * M_PI / 180.0f), c = cosf(angle * M_PI / 180.0f);
    float dx[4] = {-2, 2, 2, -2};
    float dy[4] = {-4, -4, 4, 4};
    Vertex verts[8];
    for (int i = 0; i < 4; ++i) {
        float px = c * dx[i] - s * dy[i] + x;
        float py = s * dx[i] + c * dy[i] + y;
        verts[i] = (Vertex){{px, py, z}, {r, g, b, 1.0f}, {0, 0}};
        verts[i+4] = (Vertex){{px, py, z+2}, {r, g, b, 1.0f}, {0, 0}};
    }
    GLushort indices[] = {
        0,1,2, 0,2,3, // bottom
        4,5,6, 4,6,7, // top
        0,1,5, 0,5,4, // side
        1,2,6, 1,6,5,
        2,3,7, 2,7,6,
        3,0,4, 3,4,7
    };
    glUseProgram(basicShader.prog);
    glUniformMatrix4fv(basicShader.uMVP, 1, GL_FALSE, mvp);
    glUniform1f(basicShader.uUseTexture, 0.0f);

    glEnableVertexAttribArray(basicShader.aPosition);
    glEnableVertexAttribArray(basicShader.aColor);
    glEnableVertexAttribArray(basicShader.aTexCoord);

    glVertexAttribPointer(basicShader.aPosition, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), &verts[0].position);
    glVertexAttribPointer(basicShader.aColor, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), &verts[0].color);
    glVertexAttribPointer(basicShader.aTexCoord, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), &verts[0].texcoord);

    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, indices);

    glDisableVertexAttribArray(basicShader.aPosition);
    glDisableVertexAttribArray(basicShader.aColor);
    glDisableVertexAttribArray(basicShader.aTexCoord);

    glUseProgram(0);
}

// --- Draw all players ---
void drawPlayers(Camera *pCamera) {
    // Compute the MVP matrix from the camera
    float mvp[16];
    camera_get_mvp(pCamera, mvp); // You need to implement this function
    for (int i = 0; i < game->players; ++i) {
        Player *p = &game->player[i];
        float x = p->data.posx, y = p->data.posy, z = 1.0f;
        float angle = 0.0f;
        switch (p->data.dir) {
            case 0: angle = 0.0f; break;
            case 1: angle = 90.0f; break;
            case 2: angle = 180.0f; break;
            case 3: angle = 270.0f; break;
        }
        float r = p->profile.pColorDiffuse[0];
        float g = p->profile.pColorDiffuse[1];
        float b = p->profile.pColorDiffuse[2];
        drawPlayerBox(mvp, x, y, z, angle, r, g, b);
    }
}

// --- Draw a simple trail as a colored quad (expand for full trail logic) ---
static void drawSimpleTrail(float *mvp, Player *p) {
    float x = p->data.posx, y = p->data.posy, h = p->data.trail_height;
    float r = p->profile.pColorDiffuse[0];
    float g = p->profile.pColorDiffuse[1];
    float b = p->profile.pColorDiffuse[2];
    drawQuad(mvp, x-1, y-1, x+1, y+1, h/2, r, g, b, 0.7f);
}

// --- Draw a simple explosion as a colored circle (expand for full effect) ---
static void drawSimpleExplosion(float *mvp, Player *p) {
    float x = p->data.posx, y = p->data.posy;
    float r = p->profile.pColorDiffuse[0];
    float g = p->profile.pColorDiffuse[1];
    float b = p->profile.pColorDiffuse[2];
    float radius = p->data.exp_radius;
    int segments = 24;
    Vertex *verts = malloc((segments+2) * sizeof(Vertex));
    verts[0] = (Vertex){{x, y, 1.0f}, {r, g, b, 0.8f}, {0.5f, 0.5f}};
    for (int i = 0; i <= segments; ++i) {
        float angle = 2.0f * M_PI * i / segments;
        verts[i+1] = (Vertex){{x + cosf(angle)*radius, y + sinf(angle)*radius, 1.0f}, {r, g, b, 0.0f}, {0.5f + 0.5f*cosf(angle), 0.5f + 0.5f*sinf(angle)}};
    }
    glUseProgram(basicShader.prog);
    glUniformMatrix4fv(basicShader.uMVP, 1, GL_FALSE, mvp);
    glUniform1f(basicShader.uUseTexture, 0.0f);

    glEnableVertexAttribArray(basicShader.aPosition);
    glEnableVertexAttribArray(basicShader.aColor);
    glEnableVertexAttribArray(basicShader.aTexCoord);

    glVertexAttribPointer(basicShader.aPosition, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), &verts[0].position);
    glVertexAttribPointer(basicShader.aColor, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), &verts[0].color);
    glVertexAttribPointer(basicShader.aTexCoord, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), &verts[0].texcoord);

    glDrawArrays(GL_TRIANGLE_FAN, 0, segments+2);

    glDisableVertexAttribArray(basicShader.aPosition);
    glDisableVertexAttribArray(basicShader.aColor);
    glDisableVertexAttribArray(basicShader.aTexCoord);

    glUseProgram(0);
    free(verts);
}

// --- Main drawGame function ---
void drawGame() {
    initBasicShader();

    // Setup MVP matrix (ortho for demo; replace with perspective/camera as needed)
    float mvp[16];
    ortho_matrix(mvp, -120, 120, -120, 120, -10, 100);

    // Clear screen
    glClearColor(gSettingsCache.clear_color[0], gSettingsCache.clear_color[1], gSettingsCache.clear_color[2], 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Draw skybox (stub: replace with your skybox renderer)
    // drawSkybox_modern(...);

    // Draw floor and arena
    drawFloor(mvp);
    drawArenaWalls(mvp);

    // Draw players
    drawPlayers(mvp);

    // Draw trails (simple version; expand for full trail logic)
    for (int i = 0; i < game->players; ++i) {
        if (game->player[i].data.trail_height > 0)
            drawSimpleTrail(mvp, &game->player[i]);
    }

    // Draw explosions (simple version)
    for (int i = 0; i < game->players; ++i) {
        if (game->player[i].data.exp_radius > 0)
            drawSimpleExplosion(mvp, &game->player[i]);
    }

    // Draw HUD, overlays, etc. (stub)
    // drawHUD_modern(...);
}