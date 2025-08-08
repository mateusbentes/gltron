#include <time.h>
#include <jni.h>
#include <android/log.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define LOG_TAG "GLTron_Main"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// Forward declarations from adapters
extern int gles_init();
extern void gles_cleanup();
extern void gles_set_viewport(int width, int height);
extern void gles_clear(float r, float g, float b, float a);

extern void android_input_init();
extern void android_input_handle_touch_down(float x, float y);
extern void android_input_handle_touch_move(float x, float y);
extern void android_input_handle_touch_up(float x, float y);
extern void android_input_update_gltron_input();

extern int android_audio_init();
extern void android_audio_cleanup();
extern void android_audio_set_master_volume(float volume);
extern void android_audio_set_sfx_volume(float volume);
extern void android_audio_set_music_volume(float volume);
extern float android_audio_get_master_volume();
extern float android_audio_get_sfx_volume();
extern float android_audio_get_music_volume();
extern void android_audio_enable(int enabled);

// Forward declarations from GLTron (stub implementations if not available)
extern int gltron_init();
extern void gltron_cleanup();
extern void gltron_update(float delta_time);
extern void gltron_render();
extern void gltron_set_screen_size(int width, int height);

// Stub implementations if GLTron functions are not available
static int gltron_stub_initialized = 0;

int gltron_init_stub() {
    LOGI("GLTron stub: init called");
    gltron_stub_initialized = 1;
    return 1;
}

void gltron_cleanup_stub() {
    LOGI("GLTron stub: cleanup called");
    gltron_stub_initialized = 0;
}

void gltron_update_stub(float delta_time) {
    // Basic stub update - could draw a simple test pattern
    if (!gltron_stub_initialized) return;
}

void gltron_render_stub() {
    // Basic stub render - draw a simple colored triangle as test
    if (!gltron_stub_initialized) return;
    
    // Simple test triangle vertices
    static float vertices[] = {
        0.0f,  0.5f, 0.0f,
       -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f
    };
    
    static float colors[] = {
        1.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 1.0f
    };
    
    // This would call the GLES adapter to draw the triangle
    // gles_draw_triangles(vertices, colors, 3);
}

void gltron_set_screen_size_stub(int width, int height) {
    LOGI("GLTron stub: screen size set to %dx%d", width, height);
}

// EGL context
typedef struct {
    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;
    EGLConfig config;
    ANativeWindow* window;
    int width;
    int height;
    int initialized;
} GLTronEGLContext;

static GLTronEGLContext g_egl_context = {0};

// Game state
typedef struct {
    int running;
    int paused;
    int screen_width;
    int screen_height;
    long last_frame_time;
} GameState;

static GameState g_game_state = {0};

// Settings
typedef struct {
    int resolution_width;
    int resolution_height;
    float audio_master_volume;
    float audio_sfx_volume;
    float audio_music_volume;
    int audio_enabled;
} GameSettings;

static GameSettings g_game_settings = {
    .resolution_width = 1920,
    .resolution_height = 1080,
    .audio_master_volume = 1.0f,
    .audio_sfx_volume = 1.0f,
    .audio_music_volume = 0.7f,
    .audio_enabled = 1
};

// Utility functions
static long get_time_ms() {
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
        LOGE("Failed to get time");
        return 0;
    }
    return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

// EGL functions
static int init_egl(ANativeWindow* window) {
    // FIXED: Add null pointer check
    if (!window) {
        LOGE("Window is NULL");
        return 0;
    }
    
    EGLint attribs[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_BLUE_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_RED_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 16,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_NONE
    };
    
    EGLint context_attribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };
    
    EGLint num_configs;
    
    g_egl_context.display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (g_egl_context.display == EGL_NO_DISPLAY) {
        LOGE("Failed to get EGL display");
        return 0;
    }
    
    if (!eglInitialize(g_egl_context.display, NULL, NULL)) {
        LOGE("Failed to initialize EGL");
        return 0;
    }
    
    if (!eglChooseConfig(g_egl_context.display, attribs, &g_egl_context.config, 1, &num_configs)) {
        LOGE("Failed to choose EGL config");
        return 0;
    }
    
    if (num_configs == 0) {
        LOGE("No suitable EGL config found");
        return 0;
    }
    
    g_egl_context.surface = eglCreateWindowSurface(g_egl_context.display, 
                                                   g_egl_context.config, 
                                                   window, NULL);
    if (g_egl_context.surface == EGL_NO_SURFACE) {
        LOGE("Failed to create EGL surface");
        return 0;
    }
    
    g_egl_context.context = eglCreateContext(g_egl_context.display, 
                                             g_egl_context.config, 
                                             EGL_NO_CONTEXT, 
                                             context_attribs);
    if (g_egl_context.context == EGL_NO_CONTEXT) {
        LOGE("Failed to create EGL context");
        return 0;
    }
    
    if (!eglMakeCurrent(g_egl_context.display, 
                        g_egl_context.surface, 
                        g_egl_context.surface, 
                        g_egl_context.context)) {
        LOGE("Failed to make EGL context current");
        return 0;
    }
    
    g_egl_context.window = window;
    
    // FIXED: Get window dimensions with null checks
    g_egl_context.width = ANativeWindow_getWidth(window);
    g_egl_context.height = ANativeWindow_getHeight(window);
    
    if (g_egl_context.width <= 0 || g_egl_context.height <= 0) {
        LOGE("Invalid window dimensions: %dx%d", g_egl_context.width, g_egl_context.height);
        return 0;
    }
    
    g_egl_context.initialized = 1;
    
    LOGI("EGL context initialized successfully (%dx%d)", g_egl_context.width, g_egl_context.height);
    return 1;
}

static void cleanup_egl() {
    if (g_egl_context.initialized) {
        eglMakeCurrent(g_egl_context.display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        
        if (g_egl_context.context != EGL_NO_CONTEXT) {
            eglDestroyContext(g_egl_context.display, g_egl_context.context);
            g_egl_context.context = EGL_NO_CONTEXT;
        }
        
        if (g_egl_context.surface != EGL_NO_SURFACE) {
            eglDestroySurface(g_egl_context.display, g_egl_context.surface);
            g_egl_context.surface = EGL_NO_SURFACE;
        }
        
        if (g_egl_context.display != EGL_NO_DISPLAY) {
            eglTerminate(g_egl_context.display);
            g_egl_context.display = EGL_NO_DISPLAY;
        }
        
        g_egl_context.initialized = 0;
        g_egl_context.window = NULL;
    }
}

// Game loop functions
static void game_update() {
    // FIXED: Check if EGL is initialized
    if (!g_egl_context.initialized) {
        return;
    }
    
    long current_time = get_time_ms();
    float delta_time = 0.016f; // Default to 60 FPS
    
    if (g_game_state.last_frame_time > 0 && current_time > g_game_state.last_frame_time) {
        delta_time = (current_time - g_game_state.last_frame_time) / 1000.0f;
        // Clamp delta time to prevent huge jumps
        if (delta_time > 0.1f) {
            delta_time = 0.1f;
        }
    }
    g_game_state.last_frame_time = current_time;
    
    // Update input
    android_input_update_gltron_input();
    
    // FIXED: Call actual GLTron update or stub
    #ifdef GLTRON_AVAILABLE
    gltron_update(delta_time);
    #else
    gltron_update_stub(delta_time);
    #endif
}

static void game_render() {
    // FIXED: Check if EGL is initialized
    if (!g_egl_context.initialized) {
        return;
    }
    
    // Clear screen
    gles_clear(0.0f, 0.0f, 0.2f, 1.0f);
    
    // FIXED: Call actual GLTron render or stub
    #ifdef GLTRON_AVAILABLE
    gltron_render();
    #else
    gltron_render_stub();
    #endif
    
    // Swap buffers
    if (eglSwapBuffers(g_egl_context.display, g_egl_context.surface) != EGL_TRUE) {
        LOGE("Failed to swap EGL buffers");
    }
}

// JNI interface functions
JNIEXPORT jint JNICALL
Java_com_gltron_android_GLTronRenderer_nativeInit(JNIEnv *env, jobject thiz, jobject surface) {
    if (!env || !surface) {
        LOGE("Invalid JNI parameters");
        return 0;
    }
    
    ANativeWindow* window = ANativeWindow_fromSurface(env, surface);
    if (!window) {
        LOGE("Failed to get native window from surface");
        return 0;
    }
    
    if (!init_egl(window)) {
        LOGE("Failed to initialize EGL");
        ANativeWindow_release(window);
        return 0;
    }
    
    // Initialize OpenGL ES
    if (!gles_init()) {
        LOGE("Failed to initialize OpenGL ES");
        cleanup_egl();
        ANativeWindow_release(window);
        return 0;
    }
    
    gles_set_viewport(g_egl_context.width, g_egl_context.height);
    
    // Initialize input
    android_input_init();
    
    // Initialize audio
    if (!android_audio_init()) {
        LOGE("Failed to initialize audio - continuing without audio");
        // Continue without audio
    }
    
    // FIXED: Initialize game (actual GLTron or stub)
    #ifdef GLTRON_AVAILABLE
    if (!gltron_init()) {
        LOGE("Failed to initialize GLTron");
        cleanup_egl();
        ANativeWindow_release(window);
        return 0;
    }
    gltron_set_screen_size(g_egl_context.width, g_egl_context.height);
    #else
    if (!gltron_init_stub()) {
        LOGE("Failed to initialize GLTron stub");
        cleanup_egl();
        ANativeWindow_release(window);
        return 0;
    }
    gltron_set_screen_size_stub(g_egl_context.width, g_egl_context.height);
    #endif
    
    g_game_state.running = 1;
    g_game_state.paused = 0;
    g_game_state.screen_width = g_egl_context.width;
    g_game_state.screen_height = g_egl_context.height;
    g_game_state.last_frame_time = 0;
    
    LOGI("GLTron native initialized successfully (%dx%d)", 
         g_egl_context.width, g_egl_context.height);
    return 1;
}

JNIEXPORT void JNICALL
Java_com_gltron_android_GLTronRenderer_nativeCleanup(JNIEnv *env, jobject thiz) {
    g_game_state.running = 0;
    
    // FIXED: Cleanup game (actual GLTron or stub)
    #ifdef GLTRON_AVAILABLE
    gltron_cleanup();
    #else
    gltron_cleanup_stub();
    #endif
    
    android_audio_cleanup();
    gles_cleanup();
    cleanup_egl();
    
    if (g_egl_context.window) {
        ANativeWindow_release(g_egl_context.window);
        g_egl_context.window = NULL;
    }
    
    LOGI("GLTron native cleaned up");
}

JNIEXPORT void JNICALL
Java_com_gltron_android_GLTronRenderer_nativeRender(JNIEnv *env, jobject thiz) {
    if (!g_game_state.running || g_game_state.paused) {
        return;
    }
    
    game_update();
    game_render();
}

JNIEXPORT void JNICALL
Java_com_gltron_android_GLTronRenderer_nativeResize(JNIEnv *env, jobject thiz, jint width, jint height) {
    if (width <= 0 || height <= 0) {
        LOGE("Invalid resize dimensions: %dx%d", width, height);
        return;
    }
    
    g_egl_context.width = width;
    g_egl_context.height = height;
    g_game_state.screen_width = width;
    g_game_state.screen_height = height;
    
    gles_set_viewport(width, height);
    
    // FIXED: Set screen size (actual GLTron or stub)
    #ifdef GLTRON_AVAILABLE
    gltron_set_screen_size(width, height);
    #else
    gltron_set_screen_size_stub(width, height);
    #endif
    
    LOGI("Screen resized to %dx%d", width, height);
}

JNIEXPORT void JNICALL
Java_com_gltron_android_GLTronView_nativeHandleTouchDown(JNIEnv *env, jobject thiz, jfloat x, jfloat y) {
    android_input_handle_touch_down(x, y);
}

JNIEXPORT void JNICALL
Java_com_gltron_android_GLTronView_nativeHandleTouchMove(JNIEnv *env, jobject thiz, jfloat x, jfloat y) {
    android_input_handle_touch_move(x, y);
}

JNIEXPORT void JNICALL
Java_com_gltron_android_GLTronView_nativeHandleTouchUp(JNIEnv *env, jobject thiz, jfloat x, jfloat y) {
    android_input_handle_touch_up(x, y);
}

// Settings functions
JNIEXPORT void JNICALL
Java_com_gltron_android_MainActivity_nativeSetResolution(JNIEnv *env, jobject thiz, jint width, jint height) {
    if (width > 0 && height > 0) {
        g_game_settings.resolution_width = width;
        g_game_settings.resolution_height = height;
        LOGI("Resolution set to %dx%d", width, height);
    } else {
        LOGE("Invalid resolution: %dx%d", width, height);
    }
}

JNIEXPORT void JNICALL
Java_com_gltron_android_MainActivity_nativeSetMasterVolume(JNIEnv *env, jobject thiz, jfloat volume) {
    // Clamp volume to valid range
    if (volume < 0.0f) volume = 0.0f;
    if (volume > 1.0f) volume = 1.0f;
    
    g_game_settings.audio_master_volume = volume;
    android_audio_set_master_volume(volume);
}

JNIEXPORT void JNICALL
Java_com_gltron_android_MainActivity_nativeSetSfxVolume(JNIEnv *env, jobject thiz, jfloat volume) {
    // Clamp volume to valid range
    if (volume < 0.0f) volume = 0.0f;
    if (volume > 1.0f) volume = 1.0f;
    
    g_game_settings.audio_sfx_volume = volume;
    android_audio_set_sfx_volume(volume);
}

JNIEXPORT void JNICALL
Java_com_gltron_android_MainActivity_nativeSetMusicVolume(JNIEnv *env, jobject thiz, jfloat volume) {
    // Clamp volume to valid range
    if (volume < 0.0f) volume = 0.0f;
    if (volume > 1.0f) volume = 1.0f;
    
    g_game_settings.audio_music_volume = volume;
    android_audio_set_music_volume(volume);
}

JNIEXPORT jfloat JNICALL
Java_com_gltron_android_MainActivity_nativeGetMasterVolume(JNIEnv *env, jobject thiz) {
    return android_audio_get_master_volume();
}

JNIEXPORT jfloat JNICALL
Java_com_gltron_android_MainActivity_nativeGetSfxVolume(JNIEnv *env, jobject thiz) {
    return android_audio_get_sfx_volume();
}

JNIEXPORT jfloat JNICALL
Java_com_gltron_android_MainActivity_nativeGetMusicVolume(JNIEnv *env, jobject thiz) {
    return android_audio_get_music_volume();
}

JNIEXPORT void JNICALL
Java_com_gltron_android_MainActivity_nativeEnableAudio(JNIEnv *env, jobject thiz, jboolean enabled) {
    g_game_settings.audio_enabled = enabled;
    android_audio_enable(enabled);
}
