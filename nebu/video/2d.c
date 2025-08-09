#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>

#ifdef __ANDROID__
  #include <GLES2/gl2.h>
#else
  #include <GL/glew.h>
  #include <GL/gl.h>
#endif

// --- 2D Texture Object ---
typedef struct {
    GLuint tex_id;
    int w, h;
} nebu_2d;

// --- Shader Sources ---
static const char *vertexShaderSrc =
    "attribute vec2 aPosition;\n"
    "attribute vec2 aTexCoord;\n"
    "varying vec2 vTexCoord;\n"
    "uniform mat4 uMVP;\n"
    "void main() {\n"
    "  gl_Position = uMVP * vec4(aPosition, 0.0, 1.0);\n"
    "  vTexCoord = aTexCoord;\n"
    "}\n";

static const char *fragmentShaderSrc =
    "precision mediump float;\n"
    "varying vec2 vTexCoord;\n"
    "uniform sampler2D uTexture;\n"
    "void main() {\n"
    "  gl_FragColor = texture2D(uTexture, vTexCoord);\n"
    "}\n";

// --- Shader Utilities ---
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

// --- PNG Loader Placeholder ---
// Replace this with stb_image, lodepng, or SDL_image for real PNG loading.
static int load_png_rgba(const char *path, unsigned char **pixels, int *w, int *h) {
    // For demonstration, always fail.
    fprintf(stderr, "[WARN] load_png_rgba: Not implemented. Cannot load %s\n", path);
    *pixels = NULL; *w = *h = 0;
    return 0;
}

// --- 2D Texture Loader ---
nebu_2d* nebu_2d_LoadPNG(const char* path, int flags) {
    (void)flags;
    unsigned char *pixels = NULL;
    int w = 0, h = 0;
    if (!load_png_rgba(path, &pixels, &w, &h) || !pixels) {
        fprintf(stderr, "[ERROR] nebu_2d_LoadPNG: Failed to load %s\n", path);
        return NULL;
    }
    nebu_2d *obj = (nebu_2d*)calloc(1, sizeof(nebu_2d));
    obj->w = w;
    obj->h = h;
    glGenTextures(1, &obj->tex_id);
    glBindTexture(GL_TEXTURE_2D, obj->tex_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glBindTexture(GL_TEXTURE_2D, 0);
    free(pixels);
    return obj;
}

// --- 2D Texture Free ---
void nebu_2d_Free(nebu_2d* obj) {
    if (!obj) return;
    if (obj->tex_id) glDeleteTextures(1, &obj->tex_id);
    free(obj);
}

// --- MVP Matrix Utility (column-major, OpenGL style) ---
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

// --- Modern 2D Draw ---
void nebu_2d_Draw(nebu_2d* obj, int win_w, int win_h, float x, float y) {
    if (!obj) return;

    // Setup MVP matrix for placing the quad at (x, y)
    float mvp[16];
    ortho_matrix(mvp, 0, (float)win_w, 0, (float)win_h, -1, 1);

    // Vertex data for a quad at (x, y)
    GLfloat vertices[] = {
        x,         y,
        x+obj->w,  y,
        x+obj->w,  y+obj->h,
        x,         y+obj->h
    };
    GLfloat uvs[] = {
        0, 0,
        1, 0,
        1, 1,
        0, 1
    };
    GLushort indices[] = { 0, 1, 2, 0, 2, 3 };

    static GLuint prog = 0;
    static GLint aPosition = -1, aTexCoord = -1, uMVP = -1, uTexture = -1;
    if (!prog) {
        prog = createProgram(vertexShaderSrc, fragmentShaderSrc);
        aPosition = glGetAttribLocation(prog, "aPosition");
        aTexCoord = glGetAttribLocation(prog, "aTexCoord");
        uMVP = glGetUniformLocation(prog, "uMVP");
        uTexture = glGetUniformLocation(prog, "uTexture");
    }

    glUseProgram(prog);

    glUniformMatrix4fv(uMVP, 1, GL_FALSE, mvp);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, obj->tex_id);
    glUniform1i(uTexture, 0);

    glEnableVertexAttribArray(aPosition);
    glEnableVertexAttribArray(aTexCoord);

    glVertexAttribPointer(aPosition, 2, GL_FLOAT, GL_FALSE, 0, vertices);
    glVertexAttribPointer(aTexCoord, 2, GL_FLOAT, GL_FALSE, 0, uvs);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);

    glDisableVertexAttribArray(aPosition);
    glDisableVertexAttribArray(aTexCoord);

    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
}
