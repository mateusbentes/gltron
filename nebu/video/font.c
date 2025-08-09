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

#include "video/stb_image.h"

// --- Font Data Structure ---
typedef struct {
    GLuint texture;      // Font atlas texture
    int atlas_w, atlas_h;
    int glyph_w, glyph_h;
    int first_char, n_chars;
    // Add more metrics as needed
} nebu_Font;

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
    "uniform vec4 uColor;\n"
    "void main() {\n"
    "  gl_FragColor = texture2D(uTexture, vTexCoord) * uColor;\n"
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

// Don't forget to free the returned pixel buffer with stbi_image_free(*pixels) when done!

// --- Font Loader ---
nebu_Font* nebu_Font_Load(const char *atlas_path, int glyph_w, int glyph_h, int first_char, int n_chars) {
    unsigned char *pixels = NULL;
    int w = 0, h = 0;
    if (!load_png_rgba(atlas_path, &pixels, &w, &h) || !pixels) {
        fprintf(stderr, "[ERROR] nebu_Font_Load: Failed to load %s\n", atlas_path);
        return NULL;
    }
    nebu_Font *font = (nebu_Font*)calloc(1, sizeof(nebu_Font));
    font->atlas_w = w;
    font->atlas_h = h;
    font->glyph_w = glyph_w;
    font->glyph_h = glyph_h;
    font->first_char = first_char;
    font->n_chars = n_chars;
    glGenTextures(1, &font->texture);
    glBindTexture(GL_TEXTURE_2D, font->texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glBindTexture(GL_TEXTURE_2D, 0);
    free(pixels);
    return font;
}

void nebu_Font_Free(nebu_Font* font) {
    if (!font) return;
    glDeleteTextures(1, &font->texture);
    free(font);
}

// --- Modern Font Renderer ---
void nebu_Font_Render(nebu_Font* font, const char *text, float x, float y, float size, int win_w, int win_h, float r, float g, float b, float a) {
    if (!font || !text) return;

    // Setup MVP matrix for window coordinates
    float mvp[16];
    ortho_matrix(mvp, 0, (float)win_w, 0, (float)win_h, -1, 1);

    static GLuint prog = 0;
    static GLint aPosition = -1, aTexCoord = -1, uMVP = -1, uTexture = -1, uColor = -1;
    if (!prog) {
        prog = createProgram(vertexShaderSrc, fragmentShaderSrc);
        aPosition = glGetAttribLocation(prog, "aPosition");
        aTexCoord = glGetAttribLocation(prog, "aTexCoord");
        uMVP = glGetUniformLocation(prog, "uMVP");
        uTexture = glGetUniformLocation(prog, "uTexture");
        uColor = glGetUniformLocation(prog, "uColor");
    }

    glUseProgram(prog);
    glUniformMatrix4fv(uMVP, 1, GL_FALSE, mvp);
    glUniform4f(uColor, r, g, b, a);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, font->texture);
    glUniform1i(uTexture, 0);

    glEnableVertexAttribArray(aPosition);
    glEnableVertexAttribArray(aTexCoord);

    int glyphs_per_row = font->atlas_w / font->glyph_w;
    int glyphs_per_col = font->atlas_h / font->glyph_h;

    float cursor_x = x;
    float cursor_y = y;

    for (const char *p = text; *p; ++p) {
        unsigned char c = *p;
        if (c < font->first_char || c >= font->first_char + font->n_chars) {
            cursor_x += size; // skip unknown glyph
            continue;
        }
        int glyph_index = c - font->first_char;
        int row = glyph_index / glyphs_per_row;
        int col = glyph_index % glyphs_per_row;

        float u1 = (float)(col * font->glyph_w) / font->atlas_w;
        float v1 = (float)(row * font->glyph_h) / font->atlas_h;
        float u2 = (float)((col + 1) * font->glyph_w) / font->atlas_w;
        float v2 = (float)((row + 1) * font->glyph_h) / font->atlas_h;

        GLfloat vertices[] = {
            cursor_x,           cursor_y,
            cursor_x + size,    cursor_y,
            cursor_x + size,    cursor_y + size,
            cursor_x,           cursor_y + size
        };
        GLfloat uvs[] = {
            u1, v1,
            u2, v1,
            u2, v2,
            u1, v2
        };
        GLushort indices[] = { 0, 1, 2, 0, 2, 3 };

        glVertexAttribPointer(aPosition, 2, GL_FLOAT, GL_FALSE, 0, vertices);
        glVertexAttribPointer(aTexCoord, 2, GL_FLOAT, GL_FALSE, 0, uvs);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);

        cursor_x += size; // advance cursor
    }

    glDisableVertexAttribArray(aPosition);
    glDisableVertexAttribArray(aTexCoord);

    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
}
