#include <SDL2/SDL.h>
#ifdef __ANDROID__
  #include <GLES2/gl2.h>
#else
  #include <GL/gl.h>
#endif
#include <math.h>
#include <string.h>
#include "video/video.h"
#include "game/game.h"
#include "game/game_level.h"
#include "game/resource.h"
#include "base/nebu_resource.h"
#include "configuration/settings.h"
#include "base/nebu_math.h"
#include "video/nebu_renderer_gl.h"
#include "video/nebu_mesh.h"

// --- Recognizer state ---
static float alpha = 0;
const static float rec_scale_factor = 0.25f;

// --- Recognizer movement ---
static float xv[] = { 0.5f, 0.3245f, 0.6f, 0.5f, 0.68f, -0.3f };
static float yv[] = { 0.8f, 1.0f, 0.0f, 0.2f, 0.2f, 0.0f };

static float x(void) { return xv[0] * sinf(xv[1] * alpha + xv[2]) - xv[3] * sinf(xv[4] * alpha + xv[5]); }
static float y(void) { return yv[0] * cosf(yv[1] * alpha + yv[2]) - yv[3] * sinf(yv[4] * alpha + yv[5]); }
static float dx(void) { return xv[1] * xv[0] * cosf(xv[1] * alpha + xv[2]) - xv[4] * xv[3] * cosf(xv[4] * alpha + xv[5]); }
static float dy(void) { return - yv[1] * yv[0] * sinf(yv[1] * alpha + yv[2]) - yv[4] * yv[3] * sinf(yv[4] * alpha + yv[5]); }

float getRecognizerAngle(vec2 *velocity)
{
    float dxval = velocity->v[0];
    float dyval = velocity->v[1];
    float phi = acosf(dxval / sqrtf(dxval * dxval + dyval * dyval));
    if (dyval < 0) {
        phi = 2 * M_PI - phi;
    }
    return (phi + M_PI / 2) * 180.0f / M_PI;
}

void getRecognizerPositionVelocity(vec2 *p, vec2 *v)
{
    float rec_boundry = box2_Diameter(&game2->level->boundingBox) * (1 - rec_scale_factor);
    box2_Center(p, &game2->level->boundingBox);
    p->v[0] += x() * rec_boundry / 2.0f;
    p->v[1] += y() * rec_boundry / 2.0f;
    v->v[0] = dx() * rec_boundry / 100.0f;
    v->v[1] = dy() * rec_boundry / 100.0f;
}

// --- Matrix helpers ---
static void mat4_identity(float *m) {
    memset(m, 0, sizeof(float) * 16);
    m[0] = m[5] = m[10] = m[15] = 1.0f;
}
static void mat4_translate(float *m, float x, float y, float z) {
    mat4_identity(m);
    m[12] = x;
    m[13] = y;
    m[14] = z;
}
static void mat4_scale(float *m, float sx, float sy, float sz) {
    mat4_identity(m);
    m[0] = sx;
    m[5] = sy;
    m[10] = sz;
}
static void mat4_rotate_z(float *m, float angle_deg) {
    mat4_identity(m);
    float rad = angle_deg * M_PI / 180.0f;
    float c = cosf(rad), s = sinf(rad);
    m[0] = c; m[1] = -s;
    m[4] = s; m[5] = c;
}
static void mat4_mult(float *out, const float *a, const float *b) {
    for (int row = 0; row < 4; ++row)
        for (int col = 0; col < 4; ++col) {
            out[col*4+row] = 0.0f;
            for (int k = 0; k < 4; ++k)
                out[col*4+row] += a[k*4+row] * b[col*4+k];
        }
}

// --- Compose recognizer model matrix ---
static void recognizer_model_matrix(float *out, const vec2 *p, float dirx, float scale) {
    float T[16], R[16], S[16], tmp[16];
    mat4_translate(T, p->v[0], p->v[1], RECOGNIZER_HEIGHT);
    mat4_rotate_z(R, dirx);
    mat4_scale(S, scale, scale, scale);
    mat4_mult(tmp, R, S);      // tmp = R * S
    mat4_mult(out, T, tmp);    // out = T * (R * S)
}

// --- Modern edge draw ---
void drawSharpEdges_modern(gltron_Mesh *mesh, const float *mvp, const float *color) {
    // You must implement this function to draw mesh edges as lines using VBOs and shaders.
    printf("[drawSharpEdges_modern] Drawing mesh edges with color %f %f %f %f\n",
           color[0], color[1], color[2], color[3]);
    // TODO: Implement actual edge rendering here.
}

// --- Modern recognizer shadow draw ---
void drawRecognizerShadowModern(const float *viewProj) {
    vec2 p, v;
    getRecognizerPositionVelocity(&p, &v);
    float dirx = getRecognizerAngle(&v);

    float model[16], shadowModel[16], mvp[16];
    recognizer_model_matrix(model, &p, dirx, rec_scale_factor);
    mat4_mult(shadowModel, shadow_matrix, model);
    mat4_mult(mvp, viewProj, shadowModel);

    float shadowColor[4] = {0.0f, 0.0f, 0.0f, 0.5f};
    gltron_Mesh_Draw_modern((gltron_Mesh*)resource_Get(gTokenRecognizer, eRT_GLtronTriMesh), mvp, shadowColor, 0);
}

// --- Modern recognizer draw ---
void drawRecognizerModern(const float *viewProj) {
    vec2 p, v;
    getRecognizerPositionVelocity(&p, &v);
    float dirx = getRecognizerAngle(&v);

    float model[16], mvp[16];
    recognizer_model_matrix(model, &p, dirx, rec_scale_factor);
    mat4_mult(mvp, viewProj, model);

    // Fill (black)
    float fillColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    gltron_Mesh_Draw_modern((gltron_Mesh*)resource_Get(gTokenRecognizer, eRT_GLtronTriMesh), mvp, fillColor, gSettingsCache.light_cycles);

    // Outline
    float outlineColor[4] = {
        rec_outline_color[0],
        rec_outline_color[1],
        rec_outline_color[2],
        1.0f
    };
    drawSharpEdges_modern((gltron_Mesh*)resource_Get(gTokenRecognizer, eRT_GLtronTriMesh), mvp, outlineColor);
}

// --- Recognizer movement and reset ---
void doRecognizerMovement(void) {
    alpha += game2->time.dt / 2000.0f;
}
void resetRecognizer(void) {
    alpha = 0;
}
