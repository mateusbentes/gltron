#include <SDL2/SDL.h>
#ifdef __ANDROID__
  #include <GLES2/gl2.h>
#else
  #include <GL/gl.h>
#endif
#include <stdio.h>

// Vertex shader source
const char *vertexShaderSrc =
    "attribute vec4 aPosition;\n"
    "attribute vec2 aTexCoord;\n"
    "attribute vec4 aColor;\n"
    "varying vec2 vTexCoord;\n"
    "varying vec4 vColor;\n"
    "uniform mat4 uMVP;\n"
    "void main() {\n"
    "  gl_Position = uMVP * aPosition;\n"
    "  vTexCoord = aTexCoord;\n"
    "  vColor = aColor;\n"
    "}\n";

// Fragment shader source
const char *fragmentShaderSrc =
    "precision mediump float;\n"
    "varying vec2 vTexCoord;\n"
    "varying vec4 vColor;\n"
    "uniform sampler2D uTexture;\n"
    "void main() {\n"
    "  gl_FragColor = texture2D(uTexture, vTexCoord) * vColor;\n"
    "}\n";

// Compile and link shader utilities (same as previous artifacts)
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

// Ortho matrix utility
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

// Vertex data
static const GLfloat vertices[] = {
    -.5f, -.5f, 0.f, 1.f,
     .5f, -.5f, 0.f, 1.f,
     .5f,  .5f, 0.f, 1.f,
    -.5f,  .5f, 0.f, 1.f
};

static const GLfloat tex_coords[] = {
    0.f, 0.f,
    1.f, 0.f,
    1.f, 1.f,
    0.f, 1.f
};

static const GLfloat colors[] = {
    1.f, 1.f, 1.f, 1.f,
    1.f, 0.f, 0.f, 1.f,
    0.f, 1.f, 0.f, 1.f,
    0.f, 0.f, 1.f, 1.f
};

static const GLushort indices[] = { 0, 1, 2, 0, 2, 3 };

// GL objects
static GLuint vbo = 0, ibo = 0, vao = 0, tex = 0, prog = 0;
static GLint aPosition = -1, aTexCoord = -1, aColor = -1, uMVP = -1, uTexture = -1;

// Setup GL objects and shaders
void setupGL(int win_w, int win_h) {
    // Compile/link shaders
    prog = createProgram(vertexShaderSrc, fragmentShaderSrc);
    aPosition = glGetAttribLocation(prog, "aPosition");
    aTexCoord = glGetAttribLocation(prog, "aTexCoord");
    aColor = glGetAttribLocation(prog, "aColor");
    uMVP = glGetUniformLocation(prog, "uMVP");
    uTexture = glGetUniformLocation(prog, "uTexture");

    // Create VBO/IBO
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices) + sizeof(tex_coords) + sizeof(colors), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices), sizeof(tex_coords), tex_coords);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices) + sizeof(tex_coords), sizeof(colors), colors);

    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Create texture (red 2x2)
    unsigned char pixels[] = { 255, 0, 0, 255, 255, 0, 0, 255, 255, 0, 0, 255, 255, 0, 0, 255 };
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

// Fill plain (no texture)
void fill_plain(int win_w, int win_h) {
    float mvp[16];
    ortho_matrix(mvp, 0, (float)win_w, 0, (float)win_h, -1, 1);

    glUseProgram(prog);
    glUniformMatrix4fv(uMVP, 1, GL_FALSE, mvp);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

    glEnableVertexAttribArray(aPosition);
    glVertexAttribPointer(aPosition, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glEnableVertexAttribArray(aTexCoord);
    glVertexAttribPointer(aTexCoord, 2, GL_FLOAT, GL_FALSE, 0, (void*)sizeof(vertices));

    glEnableVertexAttribArray(aColor);
    glVertexAttribPointer(aColor, 4, GL_FLOAT, GL_FALSE, 0, (void*)(sizeof(vertices) + sizeof(tex_coords)));

    glDisable(GL_TEXTURE_2D); // Not needed in ES2/core

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

    glDisableVertexAttribArray(aPosition);
    glDisableVertexAttribArray(aTexCoord);
    glDisableVertexAttribArray(aColor);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glUseProgram(0);
}

// Fill textured
void fill_textured(int win_w, int win_h) {
    float mvp[16];
    ortho_matrix(mvp, 0, (float)win_w, 0, (float)win_h, -1, 1);

    glUseProgram(prog);
    glUniformMatrix4fv(uMVP, 1, GL_FALSE, mvp);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    glUniform1i(uTexture, 0);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

    glEnableVertexAttribArray(aPosition);
    glVertexAttribPointer(aPosition, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glEnableVertexAttribArray(aTexCoord);
    glVertexAttribPointer(aTexCoord, 2, GL_FLOAT, GL_FALSE, 0, (void*)sizeof(vertices));

    glEnableVertexAttribArray(aColor);
    glVertexAttribPointer(aColor, 4, GL_FLOAT, GL_FALSE, 0, (void*)(sizeof(vertices) + sizeof(tex_coords)));

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

    glDisableVertexAttribArray(aPosition);
    glDisableVertexAttribArray(aTexCoord);
    glDisableVertexAttribArray(aColor);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glUseProgram(0);
}
