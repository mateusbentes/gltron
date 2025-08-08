#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <android/log.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LOG_TAG "GLTron_GLES"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// FIXED: Add OpenGL error checking macro
#define CHECK_GL_ERROR(op) \
    do { \
        GLenum error = glGetError(); \
        if (error != GL_NO_ERROR) { \
            LOGE("OpenGL error after %s: 0x%x", op, error); \
        } \
    } while(0)

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

// FIXED: Add initialization flag
static int g_gles_initialized = 0;

// Matrix stack
#define MAX_MATRIX_STACK_DEPTH 16
static float g_matrix_stack[MAX_MATRIX_STACK_DEPTH][16];
static int g_matrix_stack_depth = 0;
static float g_current_matrix[16];

// Utility functions
static GLuint compile_shader(GLenum type, const char* source) {
    if (!source) {
        LOGE("Shader source is NULL");
        return 0;
    }
    
    GLuint shader = glCreateShader(type);
    if (shader == 0) {
        LOGE("Failed to create shader");
        return 0;
    }
    
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    
    GLint compiled;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        GLint info_len = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_len);
        if (info_len > 1) {
            char* info_log = malloc(info_len);
            if (info_log) {
                glGetShaderInfoLog(shader, info_len, NULL, info_log);
                LOGE("Error compiling shader: %s", info_log);
                free(info_log);
            }
        }
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

static void identity_matrix(float* matrix) {
    if (!matrix) return;
    
    memset(matrix, 0, 16 * sizeof(float));
    matrix[0] = matrix[5] = matrix[10] = matrix[15] = 1.0f;
}

// OpenGL ES adapter functions
int gles_init() {
    if (g_gles_initialized) {
        LOGI("GLES already initialized");
        return 1;
    }
    
    GLuint vertex_shader = compile_shader(GL_VERTEX_SHADER, vertex_shader_source);
    GLuint fragment_shader = compile_shader(GL_FRAGMENT_SHADER, fragment_shader_source);
    
    if (!vertex_shader || !fragment_shader) {
        // FIXED: Clean up shaders on failure
        if (vertex_shader) glDeleteShader(vertex_shader);
        if (fragment_shader) glDeleteShader(fragment_shader);
        return 0;
    }
    
    g_shader_program = glCreateProgram();
    if (g_shader_program == 0) {
        LOGE("Failed to create shader program");
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
        return 0;
    }
    
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
            if (info_log) {
                glGetProgramInfoLog(g_shader_program, info_len, NULL, info_log);
                LOGE("Error linking program: %s", info_log);
                free(info_log);
            }
        }
        glDeleteProgram(g_shader_program);
        g_shader_program = 0;
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
        return 0;
    }
    
    // FIXED: Delete shaders after successful linking to prevent memory leak
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    
    // Get attribute and uniform locations
    g_position_attrib = glGetAttribLocation(g_shader_program, "a_position");
    g_color_attrib = glGetAttribLocation(g_shader_program, "a_color");
    g_texcoord_attrib = glGetAttribLocation(g_shader_program, "a_texcoord");
    g_mvp_uniform = glGetUniformLocation(g_shader_program, "u_mvp_matrix");
    g_texture_uniform = glGetUniformLocation(g_shader_program, "u_texture");
    g_use_texture_uniform = glGetUniformLocation(g_shader_program, "u_use_texture");
    
    // FIXED: Validate attribute and uniform locations
    if (g_position_attrib == -1) {
        LOGE("Failed to get position attribute location");
    }
    if (g_color_attrib == -1) {
        LOGE("Failed to get color attribute location");
    }
    if (g_mvp_uniform == -1) {
        LOGE("Failed to get MVP uniform location");
    }
    
    glUseProgram(g_shader_program);
    CHECK_GL_ERROR("glUseProgram");
    
    // Initialize matrix
    identity_matrix(g_current_matrix);
    g_matrix_stack_depth = 0;
    
    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    CHECK_GL_ERROR("depth test setup");
    
    // Enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    CHECK_GL_ERROR("blend setup");
    
    g_gles_initialized = 1;
    LOGI("OpenGL ES adapter initialized successfully");
    return 1;
}

void gles_cleanup() {
    if (!g_gles_initialized) {
        return;
    }
    
    if (g_shader_program) {
        glDeleteProgram(g_shader_program);
        g_shader_program = 0;
    }
    
    // Reset state
    g_position_attrib = -1;
    g_color_attrib = -1;
    g_texcoord_attrib = -1;
    g_mvp_uniform = -1;
    g_texture_uniform = -1;
    g_use_texture_uniform = -1;
    g_matrix_stack_depth = 0;
    g_gles_initialized = 0;
    
    LOGI("OpenGL ES adapter cleaned up");
}

void gles_set_viewport(int width, int height) {
    if (!g_gles_initialized) {
        LOGE("GLES not initialized");
        return;
    }
    
    // FIXED: Validate viewport dimensions
    if (width <= 0 || height <= 0) {
        LOGE("Invalid viewport dimensions: %dx%d", width, height);
        return;
    }
    
    glViewport(0, 0, width, height);
    CHECK_GL_ERROR("glViewport");
    LOGI("Viewport set to %dx%d", width, height);
}

void gles_clear(float r, float g, float b, float a) {
    if (!g_gles_initialized) {
        return;
    }
    
    // FIXED: Clamp color values
    if (r < 0.0f) r = 0.0f; if (r > 1.0f) r = 1.0f;
    if (g < 0.0f) g = 0.0f; if (g > 1.0f) g = 1.0f;
    if (b < 0.0f) b = 0.0f; if (b > 1.0f) b = 1.0f;
    if (a < 0.0f) a = 0.0f; if (a > 1.0f) a = 1.0f;
    
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    CHECK_GL_ERROR("glClear");
}

void gles_push_matrix() {
    if (!g_gles_initialized) {
        return;
    }
    
    // FIXED: Check stack overflow
    if (g_matrix_stack_depth >= MAX_MATRIX_STACK_DEPTH - 1) {
        LOGE("Matrix stack overflow");
        return;
    }
    
    memcpy(g_matrix_stack[g_matrix_stack_depth], g_current_matrix, 16 * sizeof(float));
    g_matrix_stack_depth++;
}

void gles_pop_matrix() {
    if (!g_gles_initialized) {
        return;
    }
    
    // FIXED: Check stack underflow
    if (g_matrix_stack_depth <= 0) {
        LOGE("Matrix stack underflow");
        return;
    }
    
    g_matrix_stack_depth--;
    memcpy(g_current_matrix, g_matrix_stack[g_matrix_stack_depth], 16 * sizeof(float));
}

void gles_load_identity() {
    if (!g_gles_initialized) {
        return;
    }
    
    identity_matrix(g_current_matrix);
}

void gles_update_mvp_matrix() {
    if (!g_gles_initialized || g_mvp_uniform == -1) {
        return;
    }
    
    glUniformMatrix4fv(g_mvp_uniform, 1, GL_FALSE, g_current_matrix);
    CHECK_GL_ERROR("glUniformMatrix4fv");
}

// Rendering functions
void gles_draw_triangles(float* vertices, float* colors, int vertex_count) {
    if (!g_gles_initialized) {
        LOGE("GLES not initialized");
        return;
    }
    
    // FIXED: Validate input parameters
    if (!vertices || !colors || vertex_count <= 0) {
        LOGE("Invalid parameters for gles_draw_triangles");
        return;
    }
    
    if (vertex_count % 3 != 0) {
        LOGE("Triangle vertex count must be multiple of 3, got %d", vertex_count);
        return;
    }
    
    glUseProgram(g_shader_program);
    gles_update_mvp_matrix();
    
    glUniform1i(g_use_texture_uniform, 0);
    CHECK_GL_ERROR("glUniform1i");
    
    if (g_position_attrib != -1) {
        glVertexAttribPointer(g_position_attrib, 3, GL_FLOAT, GL_FALSE, 0, vertices);
        glEnableVertexAttribArray(g_position_attrib);
        CHECK_GL_ERROR("position attribute");
    }
    
    if (g_color_attrib != -1) {
        glVertexAttribPointer(g_color_attrib, 4, GL_FLOAT, GL_FALSE, 0, colors);
        glEnableVertexAttribArray(g_color_attrib);
        CHECK_GL_ERROR("color attribute");
    }
    
    glDrawArrays(GL_TRIANGLES, 0, vertex_count);
    CHECK_GL_ERROR("glDrawArrays");
    
    // FIXED: Disable vertex attribute arrays
    if (g_position_attrib != -1) {
        glDisableVertexAttribArray(g_position_attrib);
    }
    if (g_color_attrib != -1) {
        glDisableVertexAttribArray(g_color_attrib);
    }
}

void gles_draw_lines(float* vertices, float* colors, int vertex_count) {
    if (!g_gles_initialized) {
        LOGE("GLES not initialized");
        return;
    }
    
    // FIXED: Validate input parameters
    if (!vertices || !colors || vertex_count <= 0) {
        LOGE("Invalid parameters for gles_draw_lines");
        return;
    }
    
    if (vertex_count % 2 != 0) {
        LOGE("Line vertex count must be multiple of 2, got %d", vertex_count);
        return;
    }
    
    glUseProgram(g_shader_program);
    gles_update_mvp_matrix();
    
    glUniform1i(g_use_texture_uniform, 0);
    CHECK_GL_ERROR("glUniform1i");
    
    if (g_position_attrib != -1) {
        glVertexAttribPointer(g_position_attrib, 3, GL_FLOAT, GL_FALSE, 0, vertices);
        glEnableVertexAttribArray(g_position_attrib);
        CHECK_GL_ERROR("position attribute");
    }
    
    if (g_color_attrib != -1) {
        glVertexAttribPointer(g_color_attrib, 4, GL_FLOAT, GL_FALSE, 0, colors);
        glEnableVertexAttribArray(g_color_attrib);
        CHECK_GL_ERROR("color attribute");
    }
    
    glDrawArrays(GL_LINES, 0, vertex_count);
    CHECK_GL_ERROR("glDrawArrays");
    
    // FIXED: Disable vertex attribute arrays
    if (g_position_attrib != -1) {
        glDisableVertexAttribArray(g_position_attrib);
    }
    if (g_color_attrib != -1) {
        glDisableVertexAttribArray(g_color_attrib);
    }
}

// FIXED: Add texture rendering function
void gles_draw_textured_triangles(float* vertices, float* texcoords, float* colors, int vertex_count, GLuint texture) {
    if (!g_gles_initialized) {
        LOGE("GLES not initialized");
        return;
    }
    
    if (!vertices || !texcoords || !colors || vertex_count <= 0) {
        LOGE("Invalid parameters for gles_draw_textured_triangles");
        return;
    }
    
    if (vertex_count % 3 != 0) {
        LOGE("Triangle vertex count must be multiple of 3, got %d", vertex_count);
        return;
    }
    
    glUseProgram(g_shader_program);
    gles_update_mvp_matrix();
    
    // Bind texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1i(g_texture_uniform, 0);
    glUniform1i(g_use_texture_uniform, 1);
    CHECK_GL_ERROR("texture setup");
    
    if (g_position_attrib != -1) {
        glVertexAttribPointer(g_position_attrib, 3, GL_FLOAT, GL_FALSE, 0, vertices);
        glEnableVertexAttribArray(g_position_attrib);
    }
    
    if (g_texcoord_attrib != -1) {
        glVertexAttribPointer(g_texcoord_attrib, 2, GL_FLOAT, GL_FALSE, 0, texcoords);
        glEnableVertexAttribArray(g_texcoord_attrib);
    }
    
    if (g_color_attrib != -1) {
        glVertexAttribPointer(g_color_attrib, 4, GL_FLOAT, GL_FALSE, 0, colors);
        glEnableVertexAttribArray(g_color_attrib);
    }
    
    glDrawArrays(GL_TRIANGLES, 0, vertex_count);
    CHECK_GL_ERROR("glDrawArrays");
    
    // Disable vertex attribute arrays
    if (g_position_attrib != -1) {
        glDisableVertexAttribArray(g_position_attrib);
    }
    if (g_texcoord_attrib != -1) {
        glDisableVertexAttribArray(g_texcoord_attrib);
    }
    if (g_color_attrib != -1) {
        glDisableVertexAttribArray(g_color_attrib);
    }
}

// FIXED: Add utility functions for matrix operations
void gles_translate(float x, float y, float z) {
    if (!g_gles_initialized) return;
    
    float translation[16];
    identity_matrix(translation);
    translation[12] = x;
    translation[13] = y;
    translation[14] = z;
    
    // Multiply current matrix by translation matrix
    // This is a simplified implementation - in production you'd want optimized matrix math
    float result[16];
    memcpy(result, g_current_matrix, 16 * sizeof(float));
    
    // Apply translation (simplified)
    result[12] += x;
    result[13] += y;
    result[14] += z;
    
    memcpy(g_current_matrix, result, 16 * sizeof(float));
}

void gles_scale(float x, float y, float z) {
    if (!g_gles_initialized) return;
    
    // Apply scaling to current matrix (simplified)
    g_current_matrix[0] *= x;
    g_current_matrix[5] *= y;
    g_current_matrix[10] *= z;
}

// FIXED: Add function to check if GLES is initialized
int gles_is_initialized() {
    return g_gles_initialized;
}

