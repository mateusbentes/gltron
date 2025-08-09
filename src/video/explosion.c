#include <SDL2/SDL.h>
#ifdef __ANDROID__
  #include <GLES2/gl2.h>
#else
  #include <GL/glew.h>
  #include <GL/gl.h>
#endif
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "base/nebu_math.h"

// --- Minimal vec3 struct and helpers ---
typedef struct {
    float v[3];
} vec3;

// Cross product: out = a x b
static void vec3_Cross(vec3 *out, const vec3 *a, const vec3 *b) {
    out->v[0] = a->v[1]*b->v[2] - a->v[2]*b->v[1];
    out->v[1] = a->v[2]*b->v[0] - a->v[0]*b->v[2];
    out->v[2] = a->v[0]*b->v[1] - a->v[1]*b->v[0];
}

// Normalize: out = a / |a|
static void vec3_Normalize(vec3 *out, const vec3 *a) {
    float len = sqrtf(a->v[0]*a->v[0] + a->v[1]*a->v[1] + a->v[2]*a->v[2]);
    if (len > 1e-6f) {
        out->v[0] = a->v[0] / len;
        out->v[1] = a->v[1] / len;
        out->v[2] = a->v[2] / len;
    } else {
        out->v[0] = out->v[1] = out->v[2] = 0.0f;
    }
}

// Scale: out = a * s
static void vec3_Scale(vec3 *out, const vec3 *a, float s) {
    out->v[0] = a->v[0] * s;
    out->v[1] = a->v[1] * s;
    out->v[2] = a->v[2] * s;
}

// --- Shader sources ---
static const char *vertexShaderSrc =
    "attribute vec2 aPosition;\n"
    "attribute vec4 aColor;\n"
    "attribute vec2 aTexCoord;\n"
    "varying vec4 vColor;\n"
    "varying vec2 vTexCoord;\n"
    "uniform mat4 uMVP;\n"
    "void main() {\n"
    "  gl_Position = uMVP * vec4(aPosition, 0.0, 1.0);\n"
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

// --- Modern shockwave (semi-circular) ---
void drawShockwaves_modern(float cx, float cy, float radius, int win_w, int win_h) {
    const int SHOCKWAVE_SEGMENTS = 25;
    const int NUM_SHOCKWAVES = 3;
    const float SHOCKWAVE_WIDTH = 0.2f;
    const float SHOCKWAVE_SPACING = 6.0f;
    const float SHOCKWAVE_MIN_RADIUS = 0.0f;
    const float SHOCKWAVE_MAX_RADIUS = 45.0f;

    static GLuint prog = 0;
    static GLint aPosition = -1, aColor = -1, uMVP = -1;
    if (!prog) {
        prog = createProgram(vertexShaderSrc, fragmentShaderSrc);
        aPosition = glGetAttribLocation(prog, "aPosition");
        aColor = glGetAttribLocation(prog, "aColor");
        uMVP = glGetUniformLocation(prog, "uMVP");
    }

    float mvp[16];
    ortho_matrix(mvp, 0, (float)win_w, 0, (float)win_h, -1, 1);

    glUseProgram(prog);
    glUniformMatrix4fv(uMVP, 1, GL_FALSE, mvp);

    for (int waves = 0; waves < NUM_SHOCKWAVES; waves++) {
        float r = radius - waves * SHOCKWAVE_SPACING;
        if (r > SHOCKWAVE_MIN_RADIUS && r < SHOCKWAVE_MAX_RADIUS) {
            int nVerts = (SHOCKWAVE_SEGMENTS + 1) * 2;
            float *vertices = (float*)malloc(sizeof(float) * 2 * nVerts);
            float *colors = (float*)malloc(sizeof(float) * 4 * nVerts);

            float delta_radius = SHOCKWAVE_WIDTH / SHOCKWAVE_SEGMENTS;
            float delta_angle = (180.0f / SHOCKWAVE_SEGMENTS) * (M_PI / 180.0f);
            float start_angle = 270.0f * (M_PI / 180.0f);

            for (int i = 0; i < SHOCKWAVE_SEGMENTS; i++) {
                float angle = start_angle;
                for (int j = 0; j <= SHOCKWAVE_SEGMENTS; j++) {
                    float outer_r = r + delta_radius;
                    float inner_r = r;
                    // Outer
                    vertices[(j * 4) + 0] = cx + outer_r * sinf(angle);
                    vertices[(j * 4) + 1] = cy + outer_r * cosf(angle);
                    colors[(j * 8) + 0] = 1.0f; colors[(j * 8) + 1] = 0.0f; colors[(j * 8) + 2] = 0.0f; colors[(j * 8) + 3] = 1.0f;
                    // Inner
                    vertices[(j * 4) + 2] = cx + inner_r * sinf(angle);
                    vertices[(j * 4) + 3] = cy + inner_r * cosf(angle);
                    colors[(j * 8) + 4] = 1.0f; colors[(j * 8) + 5] = 0.0f; colors[(j * 8) + 6] = 0.0f; colors[(j * 8) + 7] = 1.0f;
                    angle += delta_angle;
                }
                glEnableVertexAttribArray(aPosition);
                glEnableVertexAttribArray(aColor);
                glVertexAttribPointer(aPosition, 2, GL_FLOAT, GL_FALSE, 0, vertices);
                glVertexAttribPointer(aColor, 4, GL_FLOAT, GL_FALSE, 0, colors);
                glDrawArrays(GL_TRIANGLE_STRIP, 0, (SHOCKWAVE_SEGMENTS + 1) * 2);
                glDisableVertexAttribArray(aPosition);
                glDisableVertexAttribArray(aColor);
            }
            free(vertices);
            free(colors);
        }
    }
    glUseProgram(0);
}

// --- Modern impact glow (textured quad) ---
void drawImpactGlow_modern(float cx, float cy, float glow_radius, float opacity, int win_w, int win_h, GLuint tex_id) {
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

    float x0 = cx - glow_radius, y0 = cy - glow_radius;
    float x1 = cx + glow_radius, y1 = cy + glow_radius;
    GLfloat vertices[] = {
        x0, y0,
        x1, y0,
        x1, y1,
        x0, y1
    };
    GLfloat uvs[] = {
        0, 0,
        1, 0,
        1, 1,
        0, 1
    };
    GLfloat colors[] = {
        1.0f, 1.0f, 1.0f, opacity,
        1.0f, 1.0f, 1.0f, opacity,
        1.0f, 1.0f, 1.0f, opacity,
        1.0f, 1.0f, 1.0f, opacity
    };
    GLushort indices[] = { 0, 1, 2, 0, 2, 3 };

    glUseProgram(prog);
    glUniformMatrix4fv(uMVP, 1, GL_FALSE, mvp);
    glUniform1f(uUseTexture, 1.0f);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex_id);
    glUniform1i(uTexture, 0);

    glEnableVertexAttribArray(aPosition);
    glEnableVertexAttribArray(aColor);
    glEnableVertexAttribArray(aTexCoord);

    glVertexAttribPointer(aPosition, 2, GL_FLOAT, GL_FALSE, 0, vertices);
    glVertexAttribPointer(aColor, 4, GL_FLOAT, GL_FALSE, 0, colors);
    glVertexAttribPointer(aTexCoord, 2, GL_FLOAT, GL_FALSE, 0, uvs);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);

    glDisableVertexAttribArray(aPosition);
    glDisableVertexAttribArray(aColor);
    glDisableVertexAttribArray(aTexCoord);

    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
}

// --- Modern spires ---
void drawSpires_modern(float cx, float cy, float radius, int win_w, int win_h) {
    #define NUM_SPIRES 21
    #define SPIRE_WIDTH 0.40f
    vec3 zUnit = { { 0, 0, 1} };
    vec3 vectors[NUM_SPIRES] = {
        { {  1.00f,  0.20f,  0.00f  } },
        { {  0.80f,  0.25f,  0.00f  } },
        { {  0.90f,  0.50f,  0.00f  } },
        { {  0.70f,  0.50f,  0.00f  } },
        { {  0.52f,  0.45f,  0.00f  } },
        { {  0.65f,  0.75f,  0.00f  } },
        { {  0.42f,  0.68f,  0.00f  } },
        { {  0.40f,  1.02f,  0.00f  } },
        { {  0.20f,  0.90f,  0.00f  } },
        { {  0.08f,  0.65f,  0.00f  } },
        { {  0.00f,  1.00f,  0.00f  } },
        { { -0.08f,  0.65f,  0.00f  } },
        { { -0.20f,  0.90f,  0.00f  } },
        { { -0.40f,  1.02f,  0.00f  } },
        { { -0.42f,  0.68f,  0.00f  } },
        { { -0.65f,  0.75f,  0.00f  } },
        { { -0.52f,  0.45f,  0.00f  } },
        { { -0.70f,  0.50f,  0.00f  } },
        { { -0.90f,  0.50f,  0.00f  } },
        { { -0.80f,  0.30f,  0.00f  } },
        { { -1.00f,  0.20f,  0.00f  } }
    };

    static GLuint prog = 0;
    static GLint aPosition = -1, aColor = -1, uMVP = -1;
    if (!prog) {
        prog = createProgram(vertexShaderSrc, fragmentShaderSrc);
        aPosition = glGetAttribLocation(prog, "aPosition");
        aColor = glGetAttribLocation(prog, "aColor");
        uMVP = glGetUniformLocation(prog, "uMVP");
    }

    float mvp[16];
    ortho_matrix(mvp, 0, (float)win_w, 0, (float)win_h, -1, 1);

    glUseProgram(prog);
    glUniformMatrix4fv(uMVP, 1, GL_FALSE, mvp);

    for (int i = 0; i < NUM_SPIRES; i++) {
        vec3 right, left;
        vec3_Cross(&right, vectors + i, &zUnit);
        vec3_Normalize(&right, &right);
        vec3_Scale(&right, &right, SPIRE_WIDTH);

        vec3_Cross(&left, &zUnit, vectors + i);
        vec3_Normalize(&left, &left);
        vec3_Scale(&left, &left, SPIRE_WIDTH);

        GLfloat vertices[] = {
            cx + right.v[0], cy + right.v[1],
            cx + radius * vectors[i].v[0], cy + radius * vectors[i].v[1],
            cx + left.v[0], cy + left.v[1]
        };
        GLfloat colors[] = {
            1.0f, 1.0f, 1.0f, 0.0f,
            1.0f, 1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f, 0.0f
        };

        glEnableVertexAttribArray(aPosition);
        glEnableVertexAttribArray(aColor);

        glVertexAttribPointer(aPosition, 2, GL_FLOAT, GL_FALSE, 0, vertices);
        glVertexAttribPointer(aColor, 4, GL_FLOAT, GL_FALSE, 0, colors);

        glDrawArrays(GL_TRIANGLES, 0, 3);

        glDisableVertexAttribArray(aPosition);
        glDisableVertexAttribArray(aColor);
    }
    glUseProgram(0);
}

// --- Main explosion draw ---
void drawExplosion_modern(float cx, float cy, float radius, int win_w, int win_h, GLuint tex_id) {
    const float SHOCKWAVE_SPEED = 1.2f;
    const float IMPACT_MAX_RADIUS = 45.0f;
    const float GLOW_START_OPACITY = 1.2f;
    const float GLOW_INTENSITY = 1.0f;

    float shockwave_radius = (radius * SHOCKWAVE_SPEED);

    drawShockwaves_modern(cx, cy, shockwave_radius, win_w, win_h);

    if (radius < IMPACT_MAX_RADIUS) {
        float opacity = GLOW_START_OPACITY - (radius / IMPACT_MAX_RADIUS);
        drawImpactGlow_modern(cx, cy, radius, opacity, win_w, win_h, tex_id);
        drawSpires_modern(cx, cy, radius, win_w, win_h);
    }
}

// Explosion wrapper
void drawExplosion(void) {
    #ifdef HAVE_DRAWEXPLOSION_MODERN
    drawExplosion_modern();
    #else
    fprintf(stderr, "[WARN] drawExplosion: Not implemented.\n");
    #endif
}