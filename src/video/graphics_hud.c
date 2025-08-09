#include <SDL2/SDL.h>
#ifdef __ANDROID__
  #include <GLES2/gl2.h>
#else
  #include <GL/gl.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Simple shader sources for colored geometry
static const char *vertexShaderSrc =
    "attribute vec3 aPosition;\n"
    "attribute vec3 aColor;\n"
    "uniform mat4 uMVP;\n"
    "varying vec3 vColor;\n"
    "void main() {\n"
    "  gl_Position = uMVP * vec4(aPosition, 1.0);\n"
    "  vColor = aColor;\n"
    "}\n";

static const char *fragmentShaderSrc =
    "precision mediump float;\n"
    "varying vec3 vColor;\n"
    "void main() {\n"
    "  gl_FragColor = vec4(vColor, 1.0);\n"
    "}\n";

// Compile and link a shader program
static GLuint createShaderProgram(const char *vs, const char *fs) {
    GLuint v = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(v, 1, &vs, NULL);
    glCompileShader(v);
    GLuint f = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(f, 1, &fs, NULL);
    glCompileShader(f);
    GLuint prog = glCreateProgram();
    glAttachShader(prog, v);
    glAttachShader(prog, f);
    glLinkProgram(prog);
    glDeleteShader(v);
    glDeleteShader(f);
    return prog;
}

// Ortho matrix helper (column-major)
static void make_ortho_matrix(float *out, float left, float right, float bottom, float top, float near, float far) {
    memset(out, 0, sizeof(float) * 16);
    out[0] = 2.0f / (right - left);
    out[5] = 2.0f / (top - bottom);
    out[10] = -2.0f / (far - near);
    out[12] = -(right + left) / (right - left);
    out[13] = -(top + bottom) / (top - bottom);
    out[14] = -(far + near) / (far - near);
    out[15] = 1.0f;
}

// Modern drawRect using VBOs and shaders
void drawRectModern(float x, float y, float width, float height, float *colors, float *mvp, GLuint shaderProg) {
    GLfloat vertices[12] = {
        x, y, 0,
        x, y + height, 0,
        x + width, y + height, 0,
        x + width, y, 0
    };
    GLfloat vcolors[12];
    memcpy(vcolors, colors, sizeof(float) * 12);

    GLushort indices[] = {0, 1, 2, 0, 2, 3};

    GLuint vbo[2], ibo;
    glGenBuffers(2, vbo);
    glGenBuffers(1, &ibo);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vcolors), vcolors, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glUseProgram(shaderProg);

    GLint aPosition = glGetAttribLocation(shaderProg, "aPosition");
    GLint aColor = glGetAttribLocation(shaderProg, "aColor");
    GLint uMVP = glGetUniformLocation(shaderProg, "uMVP");

    glUniformMatrix4fv(uMVP, 1, GL_FALSE, mvp);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glVertexAttribPointer(aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(aPosition);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glVertexAttribPointer(aColor, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(aColor);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

    glDisableVertexAttribArray(aPosition);
    glDisableVertexAttribArray(aColor);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glDeleteBuffers(2, vbo);
    glDeleteBuffers(1, &ibo);

    glUseProgram(0);
}

// Modern drawCircle using VBOs and shaders
void drawCircleModern(float cx, float cy, float r1, float r2, float phiStart, float phiEnd, int nSegments, float *c1, float *c2, float *c3, float *c4, float *mvp, GLuint shaderProg) {
    int nVerts = (nSegments + 1) * 2;
    GLfloat *vertices = malloc(sizeof(GLfloat) * 3 * nVerts);
    GLfloat *colors = malloc(sizeof(GLfloat) * 3 * nVerts);
    GLushort *indices = malloc(sizeof(GLushort) * 6 * nSegments);

    for (int i = 0; i <= nSegments; ++i) {
        float t = i / (float)nSegments;
        float rad = (1 - t) * phiStart + t * phiEnd;
        float cosr = cosf(rad), sinr = sinf(rad);

        // Inner ring
        vertices[3 * (2 * i + 0) + 0] = cx + cosr * r1;
        vertices[3 * (2 * i + 0) + 1] = cy + sinr * r1;
        vertices[3 * (2 * i + 0) + 2] = 0;

        // Outer ring
        vertices[3 * (2 * i + 1) + 0] = cx + cosr * r2;
        vertices[3 * (2 * i + 1) + 1] = cy + sinr * r2;
        vertices[3 * (2 * i + 1) + 2] = 0;

        // Color interpolation
        for (int k = 0; k < 3; ++k) {
            colors[3 * (2 * i + 0) + k] = c1[k] * (1 - t) + c2[k] * t;
            colors[3 * (2 * i + 1) + k] = c3[k] * (1 - t) + c4[k] * t;
        }
    }

    for (int i = 0; i < nSegments; ++i) {
        indices[6 * i + 0] = 2 * i + 0;
        indices[6 * i + 1] = 2 * i + 1;
        indices[6 * i + 2] = 2 * (i + 1) + 0;
        indices[6 * i + 3] = 2 * (i + 1) + 0;
        indices[6 * i + 4] = 2 * i + 1;
        indices[6 * i + 5] = 2 * (i + 1) + 1;
    }

    GLuint vbo[2], ibo;
    glGenBuffers(2, vbo);
    glGenBuffers(1, &ibo);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * nVerts, vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * nVerts, colors, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * 6 * nSegments, indices, GL_STATIC_DRAW);

    glUseProgram(shaderProg);

    GLint aPosition = glGetAttribLocation(shaderProg, "aPosition");
    GLint aColor = glGetAttribLocation(shaderProg, "aColor");
    GLint uMVP = glGetUniformLocation(shaderProg, "uMVP");

    glUniformMatrix4fv(uMVP, 1, GL_FALSE, mvp);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glVertexAttribPointer(aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(aPosition);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glVertexAttribPointer(aColor, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(aColor);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glDrawElements(GL_TRIANGLES, 6 * nSegments, GL_UNSIGNED_SHORT, 0);

    glDisableVertexAttribArray(aPosition);
    glDisableVertexAttribArray(aColor);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glDeleteBuffers(2, vbo);
    glDeleteBuffers(1, &ibo);

    glUseProgram(0);

    free(vertices);
    free(colors);
    free(indices);
}

// Example: Setup and use in your HUD rendering
void drawHUDModern(/* ... HUD params ... */) {
    static GLuint shaderProg = 0;
    if (!shaderProg)
        shaderProg = createShaderProgram(vertexShaderSrc, fragmentShaderSrc);

    // Setup ortho projection for HUD
    float mvp[16];
    make_ortho_matrix(mvp, 0, 1024, 0, 768, -1, 1);

    // Example: draw a rectangle at (100,100) size 200x50 with four corner colors
    float rectColors[12] = {
        1,0,0,  // bottom-left
        0,1,0,  // top-left
        0,0,1,  // top-right
        1,1,0   // bottom-right
    };
    drawRectModern(100, 100, 200, 50, rectColors, mvp, shaderProg);

    // Example: draw a circle at (512,384) with radii 50, 100, and color gradients
    float c1[3] = {1,0,0}, c2[3] = {0,1,0}, c3[3] = {0,0,1}, c4[3] = {1,1,0};
    drawCircleModern(512, 384, 50, 100, 0, 2 * 3.1415926f, 32, c1, c2, c3, c4, mvp, shaderProg);
}
