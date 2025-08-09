#include <SDL2/SDL.h>
#ifdef __ANDROID__
  #include <GLES2/gl2.h>
#else
  #include <GL/glew.h>
  #include <GL/gl.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include "video/trail_geometry.h"
#include "game/game.h"
#include "configuration/settings.h"

// --- Shader sources ---
static const char *trailVertexShaderSrc =
    "attribute vec3 aPosition;\n"
    "attribute vec3 aNormal;\n"
    "attribute vec2 aTexCoord;\n"
    "attribute vec4 aColor;\n"
    "varying vec2 vTexCoord;\n"
    "varying vec4 vColor;\n"
    "uniform mat4 uMVP;\n"
    "void main() {\n"
    "  gl_Position = uMVP * vec4(aPosition, 1.0);\n"
    "  vTexCoord = aTexCoord;\n"
    "  vColor = aColor;\n"
    "}\n";

static const char *trailFragmentShaderSrc =
    "precision mediump float;\n"
    "varying vec2 vTexCoord;\n"
    "varying vec4 vColor;\n"
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

// --- Modern trail render ---
void trailRenderModern(TrailMesh *pMesh, GLuint tex_id, int win_w, int win_h, int useTexture) {
    if (!pMesh || pMesh->iUsed == 0)
        return;

    static GLuint prog = 0;
    static GLint aPosition = -1, aNormal = -1, aTexCoord = -1, aColor = -1, uMVP = -1, uTexture = -1, uUseTexture = -1;
    if (!prog) {
        prog = createProgram(trailVertexShaderSrc, trailFragmentShaderSrc);
        aPosition = glGetAttribLocation(prog, "aPosition");
        aNormal = glGetAttribLocation(prog, "aNormal");
        aTexCoord = glGetAttribLocation(prog, "aTexCoord");
        aColor = glGetAttribLocation(prog, "aColor");
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
    glEnableVertexAttribArray(aNormal);
    glEnableVertexAttribArray(aTexCoord);
    glEnableVertexAttribArray(aColor);

    glVertexAttribPointer(aPosition, 3, GL_FLOAT, 0, 0, pMesh->pVertices);
    glVertexAttribPointer(aNormal, 3, GL_FLOAT, 0, 0, pMesh->pNormals);
    glVertexAttribPointer(aTexCoord, 2, GL_FLOAT, 0, 0, pMesh->pTexCoords);
    glVertexAttribPointer(aColor, 4, GL_UNSIGNED_BYTE, 1, 0, pMesh->pColors);

    glDrawElements(GL_TRIANGLES, pMesh->iUsed, GL_UNSIGNED_SHORT, pMesh->pIndices);

    glDisableVertexAttribArray(aPosition);
    glDisableVertexAttribArray(aNormal);
    glDisableVertexAttribArray(aTexCoord);
    glDisableVertexAttribArray(aColor);

    if (useTexture) glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
}
