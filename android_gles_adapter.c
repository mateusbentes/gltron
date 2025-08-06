#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <android/log.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LOG_TAG "GLTron_GLES"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// Shader sources
static const char* vertex_shader_source = 
    "#version 100\n"
    "attribute vec4 a_position;\n"
    "attribute vec4 a_color;\n"
    "attribute vec2 a_texcoord;\n"
    "uniform mat4 u_mvp_matrix;\n"
    "varying vec4 v_color;\n"
    "varying vec2 v_texcoord;\n"
    "void main() {\n"
    "    gl_Position = u_mvp_matrix * a_position;\n"
    "    v_color = a_color;\n"
    "    v_texcoord = a_texcoord;\n"
    "}\n";

static const char* fragment_shader_source = 
    "#version 100\n"
    "precision mediump float;\n"
    "varying vec4 v_color;\n"
    "varying vec2 v_texcoord;\n"
    "uniform sampler2D u_texture;\n"
    "uniform int u_use_texture;\n"
    "void main() {\n"
    "    if (u_use_texture == 1) {\n"
    "        gl_FragColor = texture2D(u_texture, v_texcoord) * v_color;\n"
    "    } else {\n"
    "        gl_FragColor = v_color;\n"
    "    }\n"
    "}\n";

// Global shader program
static GLuint g_shader_program = 0;
static GLint g_position_attrib = -1;
static GLint g_color_attrib = -1;
static GLint g_texcoord_attrib = -1;
static GLint g_mvp_uniform = -1;
static GLint g_texture_uniform = -1;
static GLint g_use_texture_uniform = -1;

// Matrix stack
static float g_matrix_stack[16][16];
static int g_matrix_stack_depth = 0;
static float g_current_matrix[16];

// Utility functions
static GLuint compile_shader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    
    GLint compiled;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        GLint info_len = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_len);
        if (info_len > 1) {
            char* info_log = malloc(info_len);
            glGetShaderInfoLog(shader, info_len, NULL, info_log);
            LOGE("Error compiling shader: %s", info_log);
            free(info_log);
        }
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

static void identity_matrix(float* matrix) {
    memset(matrix, 0, 16 * sizeof(float));
    matrix[0] = matrix[5] = matrix[10] = matrix[15] = 1.0f;
}

// OpenGL ES adapter functions
int gles_init() {
    GLuint vertex_shader = compile_shader(GL_VERTEX_SHADER, vertex_shader_source);
    GLuint fragment_shader = compile_shader(GL_FRAGMENT_SHADER, fragment_shader_source);
    
    if (!vertex_shader || !fragment_shader) {
        return 0;
    }
    
    g_shader_program = glCreateProgram();
    glAttachShader(g_shader_program, vertex_shader);
    glAttachShader(g_shader_program, fragment_shader);
    glLinkProgram(g_shader_program);
    
    GLint linked;
    glGetProgramiv(g_shader_program, GL_LINK_STATUS, &linked);
    if (!linked) {
        GLint info_len = 0;
        glGetProgramiv(g_shader_program, GL_INFO_LOG_LENGTH, &info_len);
        if (info_len > 1) {
            char* info_log = malloc(info_len);
            glGetProgramInfoLog(g_shader_program, info_len, NULL, info_log);
            LOGE("Error linking program: %s", info_log);
            free(info_log);
        }
        glDeleteProgram(g_shader_program);
        return 0;
    }
    
    // Get attribute and uniform locations
    g_position_attrib = glGetAttribLocation(g_shader_program, "a_position");
    g_color_attrib = glGetAttribLocation(g_shader_program, "a_color");
    g_texcoord_attrib = glGetAttribLocation(g_shader_program, "a_texcoord");
    g_mvp_uniform = glGetUniformLocation(g_shader_program, "u_mvp_matrix");
    g_texture_uniform = glGetUniformLocation(g_shader_program, "u_texture");
    g_use_texture_uniform = glGetUniformLocation(g_shader_program, "u_use_texture");
    
    glUseProgram(g_shader_program);
    
    // Initialize matrix
    identity_matrix(g_current_matrix);
    g_matrix_stack_depth = 0;
    
    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    
    // Enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    LOGI("OpenGL ES adapter initialized successfully");
    return 1;
}

void gles_cleanup() {
    if (g_shader_program) {
        glDeleteProgram(g_shader_program);
        g_shader_program = 0;
    }
}

void gles_set_viewport(int width, int height) {
    glViewport(0, 0, width, height);
}

void gles_clear(float r, float g, float b, float a) {
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void gles_push_matrix() {
    if (g_matrix_stack_depth < 15) {
        memcpy(g_matrix_stack[g_matrix_stack_depth], g_current_matrix, 16 * sizeof(float));
        g_matrix_stack_depth++;
    }
}

void gles_pop_matrix() {
    if (g_matrix_stack_depth > 0) {
        g_matrix_stack_depth--;
        memcpy(g_current_matrix, g_matrix_stack[g_matrix_stack_depth], 16 * sizeof(float));
    }
}

void gles_load_identity() {
    identity_matrix(g_current_matrix);
}

void gles_update_mvp_matrix() {
    glUniformMatrix4fv(g_mvp_uniform, 1, GL_FALSE, g_current_matrix);
}

// Rendering functions
void gles_draw_triangles(float* vertices, float* colors, int vertex_count) {
    glUseProgram(g_shader_program);
    gles_update_mvp_matrix();
    
    glUniform1i(g_use_texture_uniform, 0);
    
    glVertexAttribPointer(g_position_attrib, 3, GL_FLOAT, GL_FALSE, 0, vertices);
    glEnableVertexAttribArray(g_position_attrib);
    
    glVertexAttribPointer(g_color_attrib, 4, GL_FLOAT, GL_FALSE, 0, colors);
    glEnableVertexAttribArray(g_color_attrib);
    
    glDrawArrays(GL_TRIANGLES, 0, vertex_count);
    
    glDisableVertexAttribArray(g_position_attrib);
    glDisableVertexAttribArray(g_color_attrib);
}

void gles_draw_lines(float* vertices, float* colors, int vertex_count) {
    glUseProgram(g_shader_program);
    gles_update_mvp_matrix();
    
    glUniform1i(g_use_texture_uniform, 0);
    
    glVertexAttribPointer(g_position_attrib, 3, GL_FLOAT, GL_FALSE, 0, vertices);
    glEnableVertexAttribArray(g_position_attrib);
    
    glVertexAttribPointer(g_color_attrib, 4, GL_FLOAT, GL_FALSE, 0, colors);
    glEnableVertexAttribArray(g_color_attrib);
    
    glDrawArrays(GL_LINES, 0, vertex_count);
    
    glDisableVertexAttribArray(g_position_attrib);
    glDisableVertexAttribArray(g_color_attrib);
}

