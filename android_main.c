#ifdef ANDROID
#include <android/log.h>
#include <android/native_activity.h>
#include <android/native_window.h>
#include <android_native_app_glue.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <sys/system_properties.h>
#include "android_glue.h"
#include "shaders.h"

// Define PROP_VALUE_MAX if not already defined
#ifndef PROP_VALUE_MAX
#define PROP_VALUE_MAX 92
#endif

void initGLGui(void);

static EGLDisplay s_display = EGL_NO_DISPLAY;
static EGLSurface s_surface = EGL_NO_SURFACE;
static EGLContext s_context = EGL_NO_CONTEXT;
static int32_t s_width = 0, s_height = 0;
static int s_egl_initialized = 0;

// Constants for immersive fullscreen flags
static const int SYSTEM_UI_FLAG_LAYOUT_STABLE = 0x00000100;
static const int SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION = 0x00000200;
static const int SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN = 0x00000400;
static const int SYSTEM_UI_FLAG_HIDE_NAVIGATION = 0x00000002;
static const int SYSTEM_UI_FLAG_FULLSCREEN = 0x00000004;
static const int SYSTEM_UI_FLAG_IMMERSIVE_STICKY = 0x00001000;

int get_android_sdk_version() {
    char sdk_str[PROP_VALUE_MAX];
    if (__system_property_get("ro.build.version.sdk", sdk_str) > 0) {
        return atoi(sdk_str);
    }
    return 0;
}

// Helper to safely get JNIEnv for current thread
static JNIEnv* get_jni_env(JavaVM* vm) {
    JNIEnv* env = NULL;
    int status = (*vm)->GetEnv(vm, (void**)&env, JNI_VERSION_1_6);

    if (status == JNI_EDETACHED) {
        JavaVMAttachArgs args = {
            .version = JNI_VERSION_1_6,
            .name = "NativeThread",
            .group = NULL
        };
        if ((*vm)->AttachCurrentThread(vm, &env, &args) != JNI_OK) {
            return NULL;
        }
    } else if (status != JNI_OK) {
        return NULL;
    }

    return env;
}

// Improved immersive fullscreen with better error handling
static void set_immersive_fullscreen(struct android_app* app) {
    if (!app || !app->activity || !app->activity->vm) {
        __android_log_print(ANDROID_LOG_WARN, "gltron", "Invalid app state for immersive mode");
        return;
    }

    int sdk_version = get_android_sdk_version();
    if (sdk_version < 19) {
        __android_log_print(ANDROID_LOG_INFO, "gltron", "Immersive mode requires API 19+, current: %d", sdk_version);
        return;
    }

    JNIEnv* env = get_jni_env(app->activity->vm);
    if (!env) {
        __android_log_print(ANDROID_LOG_ERROR, "gltron", "Failed to get JNI environment");
        return;
    }

    // Use the native activity object correctly
    ANativeActivity* activity = app->activity;
    jobject activity_obj = activity->clazz;

    if (!activity_obj) {
        __android_log_print(ANDROID_LOG_ERROR, "gltron", "Activity object is null");
        return;
    }

    // Get activity class
    jclass activity_class = (*env)->GetObjectClass(env, activity_obj);
    if (!activity_class) {
        __android_log_print(ANDROID_LOG_ERROR, "gltron", "Failed to get activity class");
        goto cleanup;
    }

    // Get window
    jmethodID get_window = (*env)->GetMethodID(env, activity_class, "getWindow", "()Landroid/view/Window;");
    if (!get_window) {
        __android_log_print(ANDROID_LOG_ERROR, "gltron", "getWindow method not found");
        goto cleanup;
    }

    jobject window = (*env)->CallObjectMethod(env, activity_obj, get_window);
    if (!window || (*env)->ExceptionCheck(env)) {
        __android_log_print(ANDROID_LOG_ERROR, "gltron", "Failed to get window");
        (*env)->ExceptionClear(env);
        goto cleanup;
    }

    // Get window class
    jclass window_class = (*env)->GetObjectClass(env, window);
    if (!window_class) {
        __android_log_print(ANDROID_LOG_ERROR, "gltron", "Failed to get window class");
        (*env)->DeleteLocalRef(env, window);
        goto cleanup;
    }

    // Get decorView
    jmethodID get_decor_view = (*env)->GetMethodID(env, window_class, "getDecorView", "()Landroid/view/View;");
    if (!get_decor_view) {
        __android_log_print(ANDROID_LOG_ERROR, "gltron", "getDecorView method not found");
        (*env)->DeleteLocalRef(env, window);
        (*env)->DeleteLocalRef(env, window_class);
        goto cleanup;
    }

    jobject decor_view = (*env)->CallObjectMethod(env, window, get_decor_view);
    if (!decor_view || (*env)->ExceptionCheck(env)) {
        __android_log_print(ANDROID_LOG_ERROR, "gltron", "Failed to get decor view");
        (*env)->ExceptionClear(env);
        (*env)->DeleteLocalRef(env, window);
        (*env)->DeleteLocalRef(env, window_class);
        goto cleanup;
    }

    // Get View class
    jclass view_class = (*env)->GetObjectClass(env, decor_view);
    if (!view_class) {
        __android_log_print(ANDROID_LOG_ERROR, "gltron", "Failed to get view class");
        (*env)->DeleteLocalRef(env, window);
        (*env)->DeleteLocalRef(env, window_class);
        (*env)->DeleteLocalRef(env, decor_view);
        goto cleanup;
    }

    // Set system UI visibility
    jmethodID set_system_ui_visibility = (*env)->GetMethodID(env, view_class, "setSystemUiVisibility", "(I)V");
    if (!set_system_ui_visibility) {
        __android_log_print(ANDROID_LOG_ERROR, "gltron", "setSystemUiVisibility method not found");
        (*env)->DeleteLocalRef(env, window);
        (*env)->DeleteLocalRef(env, window_class);
        (*env)->DeleteLocalRef(env, decor_view);
        (*env)->DeleteLocalRef(env, view_class);
        goto cleanup;
    }

    // Use IMMERSIVE_STICKY instead of regular IMMERSIVE for better UX
    int flags = SYSTEM_UI_FLAG_LAYOUT_STABLE
              | SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
              | SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
              | SYSTEM_UI_FLAG_HIDE_NAVIGATION
              | SYSTEM_UI_FLAG_FULLSCREEN
              | SYSTEM_UI_FLAG_IMMERSIVE_STICKY;

    (*env)->CallVoidMethod(env, decor_view, set_system_ui_visibility, flags);
    if ((*env)->ExceptionCheck(env)) {
        __android_log_print(ANDROID_LOG_WARN, "gltron", "JNI exception in setSystemUiVisibility");
        (*env)->ExceptionClear(env);
    } else {
        __android_log_print(ANDROID_LOG_INFO, "gltron", "Immersive mode flags applied: %d", flags);
    }

    if ((*env)->ExceptionCheck(env)) {
        __android_log_print(ANDROID_LOG_ERROR, "gltron", "Exception in setSystemUiVisibility");
        (*env)->ExceptionDescribe(env);
        (*env)->ExceptionClear(env);
    } else {
        __android_log_print(ANDROID_LOG_INFO, "gltron", "Immersive fullscreen mode applied successfully");
    }

    // Clean up local references
    (*env)->DeleteLocalRef(env, window);
    (*env)->DeleteLocalRef(env, window_class);
    (*env)->DeleteLocalRef(env, decor_view);
    (*env)->DeleteLocalRef(env, view_class);

cleanup:
    if (activity_class) {
        (*env)->DeleteLocalRef(env, activity_class);
    }
    
    // Signal that we need to refresh the surface after immersive mode change
    if (s_egl_initialized && s_display != EGL_NO_DISPLAY && s_surface != EGL_NO_SURFACE) {
        // Force a redraw to prevent black screen after immersive mode change
        __android_log_print(ANDROID_LOG_INFO, "gltron", "Forcing surface refresh after immersive mode change");
        
        // Re-query surface dimensions as they might have changed
        eglQuerySurface(s_display, s_surface, EGL_WIDTH, &s_width);
        eglQuerySurface(s_display, s_surface, EGL_HEIGHT, &s_height);
        
        // Update viewport to match new dimensions
        glViewport(0, 0, (GLint)s_width, (GLint)s_height);
        
        // Ensure the game knows about the new dimensions
        if (game && game->screen) {
            game->screen->vp_w = s_width;
            game->screen->vp_h = s_height;
            gltron_resize((int)s_width, (int)s_height);
        }
        
        // Clear the screen to ensure we don't see black
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Force a buffer swap to update the display
        eglSwapBuffers(s_display, s_surface);
    }
}

// Add GL error checking function
static void check_gl_error(const char* operation) {
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        __android_log_print(ANDROID_LOG_ERROR, "gltron", "GL error after %s: 0x%04x", operation, error);
    }
}

static int init_egl(ANativeWindow* window, struct android_app* app) {
    if (s_egl_initialized) {
        __android_log_print(ANDROID_LOG_WARN, "gltron", "EGL already initialized");
        return 1;
    }

    __android_log_print(ANDROID_LOG_INFO, "gltron", "Initializing EGL...");

    s_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (s_display == EGL_NO_DISPLAY) {
        __android_log_print(ANDROID_LOG_ERROR, "gltron", "eglGetDisplay failed");
        return 0;
    }

    if (!eglInitialize(s_display, NULL, NULL)) {
        __android_log_print(ANDROID_LOG_ERROR, "gltron", "eglInitialize failed: 0x%04x", eglGetError());
        return 0;
    }

    const EGLint config_attribs[] = {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 16,
        EGL_STENCIL_SIZE, 0,
        EGL_NONE
    };

    EGLConfig config;
    EGLint num_configs;
    if (!eglChooseConfig(s_display, config_attribs, &config, 1, &num_configs) || num_configs == 0) {
        __android_log_print(ANDROID_LOG_ERROR, "gltron", "eglChooseConfig failed: 0x%04x", eglGetError());
        eglTerminate(s_display);
        s_display = EGL_NO_DISPLAY;
        return 0;
    }

    EGLint format;
    if (!eglGetConfigAttrib(s_display, config, EGL_NATIVE_VISUAL_ID, &format)) {
        __android_log_print(ANDROID_LOG_ERROR, "gltron", "eglGetConfigAttrib failed: 0x%04x", eglGetError());
        eglTerminate(s_display);
        s_display = EGL_NO_DISPLAY;
        return 0;
    }

    ANativeWindow_setBuffersGeometry(window, 0, 0, format);

    s_surface = eglCreateWindowSurface(s_display, config, window, NULL);
    if (s_surface == EGL_NO_SURFACE) {
        __android_log_print(ANDROID_LOG_ERROR, "gltron", "eglCreateWindowSurface failed: 0x%04x", eglGetError());
        eglTerminate(s_display);
        s_display = EGL_NO_DISPLAY;
        return 0;
    }

    const EGLint context_attribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };

    s_context = eglCreateContext(s_display, config, EGL_NO_CONTEXT, context_attribs);
    if (s_context == EGL_NO_CONTEXT) {
        __android_log_print(ANDROID_LOG_ERROR, "gltron", "eglCreateContext failed: 0x%04x", eglGetError());
        eglDestroySurface(s_display, s_surface);
        eglTerminate(s_display);
        s_display = EGL_NO_DISPLAY;
        s_surface = EGL_NO_SURFACE;
        return 0;
    }

    if (!eglMakeCurrent(s_display, s_surface, s_surface, s_context)) {
        __android_log_print(ANDROID_LOG_ERROR, "gltron", "eglMakeCurrent failed: 0x%04x", eglGetError());
        eglDestroyContext(s_display, s_context);
        eglDestroySurface(s_display, s_surface);
        eglTerminate(s_display);
        s_display = EGL_NO_DISPLAY;
        s_surface = EGL_NO_SURFACE;
        s_context = EGL_NO_CONTEXT;
        return 0;
    }

    eglQuerySurface(s_display, s_surface, EGL_WIDTH, &s_width);
    eglQuerySurface(s_display, s_surface, EGL_HEIGHT, &s_height);

    // Log GL info
    const char* vendor = (const char*)glGetString(GL_VENDOR);
    const char* renderer = (const char*)glGetString(GL_RENDERER);
    const char* version = (const char*)glGetString(GL_VERSION);
    const char* sl_version = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);

    __android_log_print(ANDROID_LOG_INFO, "gltron", "OpenGL ES context created successfully");
    __android_log_print(ANDROID_LOG_INFO, "gltron", "GL_VENDOR: %s", vendor ? vendor : "unknown");
    __android_log_print(ANDROID_LOG_INFO, "gltron", "GL_RENDERER: %s", renderer ? renderer : "unknown");
    __android_log_print(ANDROID_LOG_INFO, "gltron", "GL_VERSION: %s", version ? version : "unknown");
    __android_log_print(ANDROID_LOG_INFO, "gltron", "GL_SHADING_LANGUAGE_VERSION: %s", sl_version ? sl_version : "unknown");
    __android_log_print(ANDROID_LOG_INFO, "gltron", "Surface size: %dx%d", s_width, s_height);

    // Initialize game components
    __android_log_print(ANDROID_LOG_INFO, "gltron", "Initializing game components...");

    gltron_init();
    check_gl_error("gltron_init");

    __android_log_print(ANDROID_LOG_DEBUG, "gltron", "Compiling shaders...");
    init_shaders_android();
    check_gl_error("init_shaders_android");

    gltron_resize((int)s_width, (int)s_height);
    check_gl_error("gltron_resize");

    glViewport(0, 0, (GLint)s_width, (GLint)s_height);
    check_gl_error("glViewport");

    if (game && game->screen) {
        setupDisplay(game->screen);
        check_gl_error("setupDisplay");
    } else {
        __android_log_print(ANDROID_LOG_FATAL, "gltron", "setupDisplay: game->screen is NULL");
    }

    initGLGui();
    check_gl_error("initGLGui");

    // Set a more visible test clear color (not black) to verify rendering
    // Using a distinctive blue color to help diagnose black screen issues
    glClearColor(0.2f, 0.2f, 0.4f, 1.0f);
    check_gl_error("glClearColor");
    
    // Force an initial clear with this color
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    eglSwapBuffers(s_display, s_surface);
    check_gl_error("Initial clear");

    // Initialize GUI mode
    extern callbacks guiCallbacks;
    switchCallbacks(&guiCallbacks);

    s_egl_initialized = 1;
    __android_log_print(ANDROID_LOG_INFO, "gltron", "EGL initialization completed successfully");

    return 1;
}

static void term_egl() {
    __android_log_print(ANDROID_LOG_INFO, "gltron", "Terminating EGL...");

    if (s_display != EGL_NO_DISPLAY) {
        eglMakeCurrent(s_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

        if (s_context != EGL_NO_CONTEXT) {
            eglDestroyContext(s_display, s_context);
            s_context = EGL_NO_CONTEXT;
        }

        if (s_surface != EGL_NO_SURFACE) {
            eglDestroySurface(s_display, s_surface);
            s_surface = EGL_NO_SURFACE;
        }

        eglTerminate(s_display);
        s_display = EGL_NO_DISPLAY;
    }

    s_egl_initialized = 0;
    s_width = s_height = 0;
}

static int32_t handle_input(struct android_app* app, AInputEvent* event) {
    (void)app;
    int32_t type = AInputEvent_getType(event);

    if (type == AINPUT_EVENT_TYPE_MOTION) {
        int32_t action = AMotionEvent_getAction(event) & AMOTION_EVENT_ACTION_MASK;
        float x = AMotionEvent_getX(event, 0);
        float y = AMotionEvent_getY(event, 0);

        // Validate touch coordinates
        if (x >= 0 && x <= s_width && y >= 0 && y <= s_height) {
            gltron_on_touch(x, y, action);
        }
        return 1;
    }

    return 0;
}

int g_finish_requested = 0;

static void finish_activity(struct android_app* app) {
    if (!app || !app->activity || !app->activity->vm) return;

    JNIEnv* env = get_jni_env(app->activity->vm);
    if (!env) return;

    jobject activity = app->activity->clazz;
    jclass activity_class = (*env)->GetObjectClass(env, activity);

    if (activity_class) {
        jmethodID finish = (*env)->GetMethodID(env, activity_class, "finish", "()V");
        if (finish) {
            (*env)->CallVoidMethod(env, activity, finish);
        }
        (*env)->DeleteLocalRef(env, activity_class);
    }
}

// Helper function to add a small delay
static void short_delay(int milliseconds) {
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;
    nanosleep(&ts, NULL);
}

// Helper function to safely apply immersive mode and ensure rendering continues properly
static void apply_immersive_mode_and_refresh(struct android_app* app) {
    // Apply immersive fullscreen mode
    set_immersive_fullscreen(app);
    
    // Add a small delay to allow the system to process UI changes
    short_delay(50);
    
    // Ensure EGL context is current
    if (s_egl_initialized && s_display != EGL_NO_DISPLAY && s_surface != EGL_NO_SURFACE && s_context != EGL_NO_CONTEXT) {
        if (!eglMakeCurrent(s_display, s_surface, s_surface, s_context)) {
            __android_log_print(ANDROID_LOG_ERROR, "gltron", "Failed to make context current after immersive mode: 0x%04x", eglGetError());
            return;
        }
        
        // Re-query surface dimensions
        EGLint width, height;
        eglQuerySurface(s_display, s_surface, EGL_WIDTH, &width);
        eglQuerySurface(s_display, s_surface, EGL_HEIGHT, &height);
        
        // Update if dimensions changed
        if (width != s_width || height != s_height) {
            s_width = width;
            s_height = height;
            glViewport(0, 0, (GLint)s_width, (GLint)s_height);
            gltron_resize((int)s_width, (int)s_height);
        }
        
        // Force a clear and swap to ensure screen isn't black
        glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        eglSwapBuffers(s_display, s_surface);
        
        // Do it twice to ensure both buffers are cleared
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        eglSwapBuffers(s_display, s_surface);
    }
}

// Helper function to ensure EGL context is current
static int ensure_egl_context() {
    if (!s_egl_initialized || s_display == EGL_NO_DISPLAY || s_surface == EGL_NO_SURFACE || s_context == EGL_NO_CONTEXT) {
        return 0;
    }
    
    // Check if the context is already current
    EGLContext current_context = eglGetCurrentContext();
    EGLSurface current_draw = eglGetCurrentSurface(EGL_DRAW);
    EGLSurface current_read = eglGetCurrentSurface(EGL_READ);
    
    if (current_context != s_context || current_draw != s_surface || current_read != s_surface) {
        __android_log_print(ANDROID_LOG_INFO, "gltron", "EGL context not current, restoring");
        if (!eglMakeCurrent(s_display, s_surface, s_surface, s_context)) {
            __android_log_print(ANDROID_LOG_ERROR, "gltron", "Failed to make context current: 0x%04x", eglGetError());
            return 0;
        }
    }
    
    return 1;
}

static void handle_cmd(struct android_app* app, int32_t cmd) {
    switch (cmd) {
        case APP_CMD_INIT_WINDOW:
            __android_log_print(ANDROID_LOG_INFO, "gltron", "APP_CMD_INIT_WINDOW");
            if (app->window && !s_egl_initialized) {
                if (init_egl(app->window, app)) {
                    // Apply immersive mode after successful EGL init
                    apply_immersive_mode_and_refresh(app);
                }
            }
            break;

        case APP_CMD_RESUME:
            __android_log_print(ANDROID_LOG_INFO, "gltron", "APP_CMD_RESUME");
            if (app->window && !s_egl_initialized) {
                if (init_egl(app->window, app)) {
                    apply_immersive_mode_and_refresh(app);
                }
            } else if (s_egl_initialized) {
                // Reapply immersive mode on resume
                apply_immersive_mode_and_refresh(app);
            }
            break;
            
        case APP_CMD_TERM_WINDOW:
            __android_log_print(ANDROID_LOG_INFO, "gltron", "APP_CMD_TERM_WINDOW");
            term_egl();
            break;

        case APP_CMD_WINDOW_RESIZED:
            __android_log_print(ANDROID_LOG_INFO, "gltron", "APP_CMD_WINDOW_RESIZED");
            if (s_display != EGL_NO_DISPLAY && app->window) {
                eglQuerySurface(s_display, s_surface, EGL_WIDTH, &s_width);
                eglQuerySurface(s_display, s_surface, EGL_HEIGHT, &s_height);
                gltron_resize((int)s_width, (int)s_height);
                glViewport(0, 0, (GLint)s_width, (GLint)s_height);
                __android_log_print(ANDROID_LOG_INFO, "gltron", "Window resized to %dx%d", s_width, s_height);
            }
            break;

        case APP_CMD_GAINED_FOCUS:
            __android_log_print(ANDROID_LOG_INFO, "gltron", "APP_CMD_GAINED_FOCUS");
            // Reapply immersive mode when gaining focus
            if (s_egl_initialized) {
                apply_immersive_mode_and_refresh(app);
            }
            break;
        
        case APP_CMD_CONFIG_CHANGED:
            __android_log_print(ANDROID_LOG_INFO, "gltron", "APP_CMD_CONFIG_CHANGED");
            // Handle configuration changes (like orientation changes)
            if (s_egl_initialized) {
                // Re-query surface dimensions
                eglQuerySurface(s_display, s_surface, EGL_WIDTH, &s_width);
                eglQuerySurface(s_display, s_surface, EGL_HEIGHT, &s_height);
                
                // Update viewport and game dimensions
                glViewport(0, 0, (GLint)s_width, (GLint)s_height);
                gltron_resize((int)s_width, (int)s_height);
                
                // Reapply immersive mode after config change
                apply_immersive_mode_and_refresh(app);
            }
            break;

        default:
            break;
    }
}

void android_main(struct android_app* state) {
    __android_log_print(ANDROID_LOG_INFO, "gltron", "Starting android_main");

    state->onAppCmd = handle_cmd;
    state->onInputEvent = handle_input;

    // Set up asset manager and paths
    if (state->activity && state->activity->assetManager) {
        gltron_set_asset_manager((void*)state->activity->assetManager);
        if (state->activity->internalDataPath) {
            gltron_set_base_path(state->activity->internalDataPath);
        }
    }

    int events;
    struct android_poll_source* source;

    while (1) {
        // Poll with timeout to prevent excessive CPU usage
        int timeout = s_egl_initialized ? 0 : -1;

        while ((ALooper_pollOnce(timeout, NULL, &events, (void**)&source)) >= 0) {
            if (source != NULL) {
                source->process(state, source);
            }

            if (state->destroyRequested) {
                __android_log_print(ANDROID_LOG_INFO, "gltron", "Destroy requested, cleaning up...");
                term_egl();
                return;
            }
        }

        // Only render if EGL is initialized and we have a valid surface
        if (s_egl_initialized && s_display != EGL_NO_DISPLAY && s_surface != EGL_NO_SURFACE) {
            // Ensure EGL context is current before rendering
            if (!ensure_egl_context()) {
                __android_log_print(ANDROID_LOG_ERROR, "gltron", "Failed to ensure EGL context, skipping frame");
                continue;
            }
            
            // Check if surface is still valid before rendering
            EGLint surface_state = EGL_BAD_SURFACE;
            if (eglQuerySurface(s_display, s_surface, EGL_SURFACE_TYPE, &surface_state)) {
                // Ensure viewport dimensions are up-to-date
                EGLint current_width, current_height;
                eglQuerySurface(s_display, s_surface, EGL_WIDTH, &current_width);
                eglQuerySurface(s_display, s_surface, EGL_HEIGHT, &current_height);
                
                // If dimensions changed, update viewport
                if (current_width != s_width || current_height != s_height) {
                    __android_log_print(ANDROID_LOG_INFO, "gltron", "Surface dimensions changed: %dx%d -> %dx%d", 
                                       s_width, s_height, current_width, current_height);
                    s_width = current_width;
                    s_height = current_height;
                    glViewport(0, 0, (GLint)s_width, (GLint)s_height);
                    gltron_resize((int)s_width, (int)s_height);
                }
            } else {
                EGLint error = eglGetError();
                if (error != EGL_SUCCESS) {
                    __android_log_print(ANDROID_LOG_ERROR, "gltron", "Surface validation failed: 0x%04x", error);
                    // Try to recover by reinitializing
                    term_egl();
                    continue;
                }
            }
            
            // Clear with a non-black color to help diagnose rendering issues
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            gltron_frame();

            if (!eglSwapBuffers(s_display, s_surface)) {
                EGLint error = eglGetError();
                __android_log_print(ANDROID_LOG_ERROR, "gltron", "eglSwapBuffers failed: 0x%04x", error);

                if (error == EGL_BAD_SURFACE || error == EGL_BAD_NATIVE_WINDOW) {
                    // Surface was lost, reinitialize
                    __android_log_print(ANDROID_LOG_WARN, "gltron", "Surface lost, reinitializing EGL");
                    term_egl();
                    continue;
                }
            }

            if (g_finish_requested) {
                __android_log_print(ANDROID_LOG_INFO, "gltron", "Finishing activity on BACK");
                finish_activity(state);
                g_finish_requested = 0;
            }
        }
    }
}

#endif // ANDROID
