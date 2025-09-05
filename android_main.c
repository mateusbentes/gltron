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

static JNIEnv* get_jni_env(JavaVM* vm);

static void get_display_metrics(struct android_app* app, int* width, int* height) {
  JNIEnv* env = get_jni_env(app->activity->vm);
  if (!env) {
    __android_log_print(ANDROID_LOG_ERROR, "gltron", "Failed to get JNI env for display metrics");
    return;
  }

  jobject activity = app->activity->clazz;
  jclass activity_class = (*env)->GetObjectClass(env, activity);
  jmethodID getWindowManager = (*env)->GetMethodID(env, activity_class, "getWindowManager", "()Landroid/view/WindowManager;");
  jobject windowManager = (*env)->CallObjectMethod(env, activity, getWindowManager);

  jclass windowManager_class = (*env)->GetObjectClass(env, windowManager);
  jmethodID getDefaultDisplay = (*env)->GetMethodID(env, windowManager_class, "getDefaultDisplay", "()Landroid/view/Display;");
  jobject display = (*env)->CallObjectMethod(env, windowManager, getDefaultDisplay);

  jclass display_class = (*env)->GetObjectClass(env, display);
  
  // Get rotation (0=landscape, 1=portrait90, 2=landscape180, 3=portrait270)
  jmethodID getRotation = (*env)->GetMethodID(env, display_class, "getRotation", "()I");
  jint rotation = (*env)->CallIntMethod(env, display, getRotation);

  // Get physical (real) size
  jmethodID getRealSize = (*env)->GetMethodID(env, display_class, "getRealSize", "(Landroid/graphics/Point;)V");
  jclass point_class = (*env)->FindClass(env, "android/graphics/Point");
  jmethodID point_constructor = (*env)->GetMethodID(env, point_class, "<init>", "()V");
  jobject point = (*env)->NewObject(env, point_class, point_constructor);

  (*env)->CallVoidMethod(env, display, getRealSize, point);

  jfieldID x_field = (*env)->GetFieldID(env, point_class, "x", "I");
  jfieldID y_field = (*env)->GetFieldID(env, point_class, "y", "I");
  int physical_width = (*env)->GetIntField(env, point, x_field);   // Longer side (e.g., 1920)
  int physical_height = (*env)->GetIntField(env, point, y_field);  // Shorter side (e.g., 1200)

  // For tablets, assume landscape default (no swap for rotation 0/2)
  // Swap only for portrait rotations (1/3)
  if (rotation == 1 || rotation == 3) {
    *width = physical_height;  // e.g., 1200
    *height = physical_width;  // e.g., 1920
  } else {
    *width = physical_width;   // e.g., 1920
    *height = physical_height; // e.g., 1200
  }

  // Cleanup
  (*env)->DeleteLocalRef(env, point);
  (*env)->DeleteLocalRef(env, point_class);
  (*env)->DeleteLocalRef(env, display);
  (*env)->DeleteLocalRef(env, display_class);
  (*env)->DeleteLocalRef(env, windowManager);
  (*env)->DeleteLocalRef(env, windowManager_class);
  (*env)->DeleteLocalRef(env, activity_class);

  __android_log_print(ANDROID_LOG_INFO, "gltron", "Detected physical size: %dx%d | Oriented size: %dx%d (rotation: %d)", physical_width, physical_height, *width, *height, rotation);
}

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
int set_immersive_fullscreen(struct android_app* app) {
    if (!app || !app->activity || !app->activity->vm) {
        __android_log_print(ANDROID_LOG_WARN, "gltron", "Invalid app state for immersive mode");
        return -1;
    }

    JNIEnv* env = get_jni_env(app->activity->vm);
    if (!env) {
        __android_log_print(ANDROID_LOG_ERROR, "gltron", "Failed to get JNI environment");
        return -1;
    }

    jobject activity_obj = app->activity->clazz;

    jclass activity_class = (*env)->GetObjectClass(env, activity_obj);
    if (!activity_class) {
        (*env)->ExceptionDescribe(env);
        (*env)->ExceptionClear(env);
        goto fail;
    }

    // Get Window object
    jmethodID getWindow = (*env)->GetMethodID(env, activity_class, "getWindow", "()Landroid/view/Window;");
    if (!getWindow) {
        (*env)->ExceptionDescribe(env);
        (*env)->ExceptionClear(env);
        goto fail;
    }

    jobject window = (*env)->CallObjectMethod(env, activity_obj, getWindow);
    if (!window) {
        (*env)->ExceptionDescribe(env);
        (*env)->ExceptionClear(env);
        goto fail;
    }

    // Get View object from Window
    jclass window_class = (*env)->GetObjectClass(env, window);
    jmethodID getDecorView = (*env)->GetMethodID(env, window_class, "getDecorView", "()Landroid/view/View;");
    if (!getDecorView) {
        (*env)->ExceptionDescribe(env);
        (*env)->ExceptionClear(env);
        (*env)->DeleteLocalRef(env, window);
        (*env)->DeleteLocalRef(env, window_class);
        goto fail;
    }

    jobject decorView = (*env)->CallObjectMethod(env, window, getDecorView);
    if (!decorView) {
        (*env)->ExceptionDescribe(env);
        (*env)->ExceptionClear(env);
        (*env)->DeleteLocalRef(env, window);
        (*env)->DeleteLocalRef(env, window_class);
        goto fail;
    }

    // Get View class
    jclass view_class = (*env)->GetObjectClass(env, decorView);
    if (!view_class) {
        (*env)->ExceptionDescribe(env);
        (*env)->ExceptionClear(env);
        (*env)->DeleteLocalRef(env, decorView);
        (*env)->DeleteLocalRef(env, window);
        (*env)->DeleteLocalRef(env, window_class);
        goto fail;
    }

    // Call setSystemUiVisibility on the decor view
    jmethodID setSystemUiVisibility = (*env)->GetMethodID(env, view_class, "setSystemUiVisibility", "(I)V");
    if (!setSystemUiVisibility) {
        (*env)->ExceptionDescribe(env);
        (*env)->ExceptionClear(env);
        (*env)->DeleteLocalRef(env, view_class);
        (*env)->DeleteLocalRef(env, decorView);
        (*env)->DeleteLocalRef(env, window);
        (*env)->DeleteLocalRef(env, window_class);
        goto fail;
    }

    int flags = SYSTEM_UI_FLAG_LAYOUT_STABLE
              | SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
              | SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
              | SYSTEM_UI_FLAG_HIDE_NAVIGATION
              | SYSTEM_UI_FLAG_FULLSCREEN
              | SYSTEM_UI_FLAG_IMMERSIVE_STICKY;

    (*env)->CallVoidMethod(env, decorView, setSystemUiVisibility, flags);

    // Check for exceptions
    if ((*env)->ExceptionCheck(env)) {
        (*env)->ExceptionDescribe(env);
        (*env)->ExceptionClear(env);
        (*env)->DeleteLocalRef(env, view_class);
        (*env)->DeleteLocalRef(env, decorView);
        (*env)->DeleteLocalRef(env, window);
        (*env)->DeleteLocalRef(env, window_class);
        goto fail;
    }

    // Clean up local references
    (*env)->DeleteLocalRef(env, view_class);
    (*env)->DeleteLocalRef(env, decorView);
    (*env)->DeleteLocalRef(env, window);
    (*env)->DeleteLocalRef(env, window_class);

    __android_log_print(ANDROID_LOG_INFO, "gltron", "Immersive fullscreen applied successfully");
    return 0;

fail:
    if (activity_class) (*env)->DeleteLocalRef(env, activity_class);
    return 0;
}

// Add GL error checking function
static void check_gl_error(const char* operation) {
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        __android_log_print(ANDROID_LOG_ERROR, "gltron", "GL error after %s: 0x%04x", operation, error);
    }
}

int is_surface_valid() {
    return s_display != EGL_NO_DISPLAY && 
           s_surface != EGL_NO_SURFACE && 
           s_context != EGL_NO_CONTEXT;
}

// Helper function to add a small delay
static void short_delay(int milliseconds) {
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;
    nanosleep(&ts, NULL);
}

// Helper function to ensure EGL context is current
int ensure_egl_context() {
    struct android_app* current_state = state;

    if (!is_surface_valid()) {
        __android_log_print(ANDROID_LOG_WARN, "gltron", "eglMakeCurrent skipped: invalid EGL surface or context");
        return 0;
    }

    if (!eglMakeCurrent(s_display, s_surface, s_surface, s_context)) {
        __android_log_print(ANDROID_LOG_ERROR, "gltron", "Failed to make EGL context current after immersive mode: 0x%04x", eglGetError());
        return 0;
    }

    return 1;
}

// Helper function to safely apply immersive mode and ensure rendering continues properly
void apply_immersive_mode_and_refresh(struct android_app* app) {
    // Apply immersive fullscreen mode
    set_immersive_fullscreen(app);
    short_delay(20); // Wait for UI to settle

    if (is_surface_valid() && ensure_egl_context()) {
        // Update viewport with new dimensions
        EGLint width, height;
        eglQuerySurface(s_display, s_surface, EGL_WIDTH, &width);
        eglQuerySurface(s_display, s_surface, EGL_HEIGHT, &height);
        s_width = width;
        s_height = height;

        glViewport(0, 0, (GLint)s_width, (GLint)s_height);
        gltron_resize((int)s_width, (int)s_height);
    }
}

static int init_egl(ANativeWindow* window, struct android_app* app) {
  if (s_egl_initialized) {
    __android_log_print(ANDROID_LOG_WARN, "gltron", "EGL already initialized");
    return 1;
  }

  __android_log_print(ANDROID_LOG_INFO, "gltron", "Initializing EGL...");

  // Apply immersive mode early
  apply_immersive_mode_and_refresh(app);

  // Auto-detect oriented resolution
  int detected_width = 0, detected_height = 0;
  get_display_metrics(app, &detected_width, &detected_height);

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

  // Set buffer geometry to detected oriented resolution
  ANativeWindow_setBuffersGeometry(window, detected_width, detected_height, format);

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

  // Query actual surface size
  eglQuerySurface(s_display, s_surface, EGL_WIDTH, &s_width);
  eglQuerySurface(s_display, s_surface, EGL_HEIGHT, &s_height);

  // If EGL size differs from detected, log and force detected (for tablets)
  if (s_width != detected_width || s_height != detected_height) {
    __android_log_print(ANDROID_LOG_WARN, "gltron", "EGL size mismatch (EGL: %dx%d, Detected: %dx%d) - forcing detected size", s_width, s_height, detected_width, detected_height);
    s_width = detected_width;
    s_height = detected_height;
  }

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
  __android_log_print(ANDROID_LOG_INFO, "gltron", "Final surface size: %dx%d", s_width, s_height);

  // Initialize game components
  __android_log_print(ANDROID_LOG_INFO, "gltron", "Initializing game components...");

  gltron_init();
  check_gl_error("gltron_init");

  __android_log_print(ANDROID_LOG_DEBUG, "gltron", "Compiling shaders...");
  init_shaders_android();
  check_gl_error("init_shaders_android");

  // Resize using final s_width/s_height (which is now orientation-corrected)
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
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
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
    if (!is_surface_valid()) return;

    if (!(g_finish_requested)) {
        __android_log_print(ANDROID_LOG_WARN, "gltron", "Destroying EGL during normal operation...");
    }

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

void gltron_frame(void) {
  // Check initialization first
  if (!initialized) gltron_init();

  // Ensure EGL context is current
  if (!ensure_egl_context()) {
    __android_log_print(ANDROID_LOG_WARN, "gltron", "Skipping frame: EGL context not current");
    return;
  }

  // Ensure we have valid game state
  if (!game) {
    __android_log_print(ANDROID_LOG_ERROR, "gltron", "gltron_frame: game is NULL!");
    return;
  }

  // Safely call idle callback to update animations/state
  if (current_android_callbacks.idle && current_android_callbacks.idle != (void*)0xdeadbeef) {
    current_android_callbacks.idle();
  }

  // Single clear at the beginning
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  check_gl_error("Initial clear");

  // Ensure shader is bound before rendering
  GLuint prog = shader_get_basic();
  if (prog) {
    glUseProgram(prog);
  }

  // Call the appropriate display function
  if (current_android_callbacks.display) {
    current_android_callbacks.display();
  } else {
    __android_log_print(ANDROID_LOG_WARN, "gltron", "No display callback set");
  }

  // Draw overlay if in game mode
  if (!is_gui_active()) {
    draw_android_overlay();
  }

  // Swap buffers
  if (!eglSwapBuffers(s_display, s_surface)) {
    EGLint error = eglGetError();
    __android_log_print(ANDROID_LOG_ERROR, "gltron", "eglSwapBuffers failed: 0x%04x", error);
    if (error == EGL_BAD_SURFACE || error == EGL_BAD_CONTEXT) {
      __android_log_print(ANDROID_LOG_WARN, "gltron", "Invalid surface/context - reinitializing");
      term_egl();
    }
  } else {
    __android_log_print(ANDROID_LOG_DEBUG, "gltron", "Frame swapped successfully");
  }
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

        case APP_CMD_CONFIG_CHANGED:
        case APP_CMD_RESUME:
            apply_immersive_mode_and_refresh(app);
            int new_width = 0, new_height = 0;
            get_display_metrics(app, &new_width, &new_height);
            // Add tolerance (e.g., ignore small changes like navigation bar adjustments)
            int delta_w = abs(new_width - s_width);
            int delta_h = abs(new_height - s_height);
            if (delta_w > 10 || delta_h > 10) {  // Tolerance of 10 pixels
                __android_log_print(ANDROID_LOG_INFO, "gltron", "Significant resolution change detected, updating to %dx%d", new_width, new_height);
                gltron_resize(new_width, new_height);
                glViewport(0, 0, new_width, new_height);
                s_width = new_width;
                s_height = new_height;
            } else {
                __android_log_print(ANDROID_LOG_DEBUG, "gltron", "Minor size change ignored (%dx%d -> %dx%d)", s_width, s_height, new_width, new_height);
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
            
            // Safely call idle callback to update animations/state
            if (current_android_callbacks.idle && current_android_callbacks.idle != (void*)0xdeadbeef) {
                current_android_callbacks.idle();
            }

            // Single clear at the beginning
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            check_gl_error("Initial clear");

            // Ensure shader is bound before rendering
            GLuint prog = shader_get_basic();
            if (prog) {
                glUseProgram(prog);
            } else {
                __android_log_print(ANDROID_LOG_WARN, "gltron", "No basic shader available");
            }

            // Call the appropriate display function
            if (current_android_callbacks.display) {
                current_android_callbacks.display();
            } else {
                __android_log_print(ANDROID_LOG_WARN, "gltron", "No display callback set");
            }

            // Draw overlay if in game mode
            if (!is_gui_active()) {
                draw_android_overlay();
            }

            // Swap buffers
            if (!eglSwapBuffers(s_display, s_surface)) {
                EGLint error = eglGetError();
                __android_log_print(ANDROID_LOG_ERROR, "gltron", "eglSwapBuffers failed: 0x%04x", error);

                if (error == EGL_BAD_SURFACE || error == EGL_BAD_NATIVE_WINDOW) {
                    // Surface was lost, reinitialize
                    __android_log_print(ANDROID_LOG_WARN, "gltron", "Surface lost, reinitializing EGL");
                    term_egl();
                    continue;
                }
            } else {
                __android_log_print(ANDROID_LOG_DEBUG, "gltron", "Frame swapped successfully");
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
