#ifdef ANDROID
#include <android/log.h>
#include <android/native_window.h>
#include <android/native_activity.h>
#include <android/configuration.h>
#include <android_native_app_glue.h>
#include <sys/system_properties.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include "android_glue.h"
#include "shaders.h"

#define ANATIVEWINDOW_FLAG_IMMERSIVE_STICKY 0x00000800
#define ANATIVEWINDOW_FLAG_LAYOUT_STABLE 0x00000100
#define ANATIVEWINDOW_FLAG_LAYOUT_HIDE_NAVIGATION 0x00000200
#define ANATIVEWINDOW_FLAG_LAYOUT_FULLSCREEN 0x00000400
#define ANATIVEWINDOW_FLAG_HIDE_NAVIGATION 0x00000002
#define ANATIVEWINDOW_FLAG_FULLSCREEN 0x00000004

static const int SYSTEM_UI_FLAG_LAYOUT_STABLE = 0x00000100;
static const int SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION = 0x00000200;
static const int SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN = 0x00000400;
static const int SYSTEM_UI_FLAG_HIDE_NAVIGATION = 0x00000002;
static const int SYSTEM_UI_FLAG_FULLSCREEN = 0x00000004;
static const int SYSTEM_UI_FLAG_IMMERSIVE = 0x00000800;

void initGLGui(void);

static EGLDisplay s_display = EGL_NO_DISPLAY;
static EGLSurface s_surface = EGL_NO_SURFACE;
static EGLContext s_context = EGL_NO_CONTEXT;
static int32_t s_width = 0, s_height = 0;

static int get_android_api_level() {
    char value[PROP_VALUE_MAX] = {0};
    __system_property_get("ro.build.version.sdk", value);
    return atoi(value);
}

static int build_immersive_flags() {
    int flags = 0;
    int api_level = get_android_api_level();
    if (api_level >= 19) {
        flags |= SYSTEM_UI_FLAG_LAYOUT_STABLE;
        flags |= SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION;
        flags |= SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN;
        flags |= SYSTEM_UI_FLAG_HIDE_NAVIGATION;
        flags |= SYSTEM_UI_FLAG_FULLSCREEN;
        flags |= SYSTEM_UI_FLAG_IMMERSIVE;
    } else {
        flags |= SYSTEM_UI_FLAG_HIDE_NAVIGATION;
        flags |= SYSTEM_UI_FLAG_FULLSCREEN;
    }
    return flags;
}

/* Set_immersive_fullscreen */
static void set_immersive_fullscreen(struct android_app* app) {
    JNIEnv* env; // Not needed anymore
    jobject activity = app->activity->clazz;
    jclass activityCls = NULL;

    if (!app || !app->window) {
        __android_log_print(ANDROID_LOG_ERROR, "gltron", "Window is NULL");
        return;
    }

    int flags = build_immersive_flags();

    __android_log_print(ANDROID_LOG_INFO, "gltron", "Native: Applying system UI visibility flags: 0x%x", flags);

    // Set system UI visibility on native window
    struct ANativeActivity* nativeActivity = app->activity;
    if (!app->window) {
        __android_log_print(ANDROID_LOG_ERROR, "gltron", "nativeActivity->window is NULL");
        return;
    }

    // ANativeWindow doesn't directly support setting visibility
    ANativeWindow* native_window = app->window;
    if (!native_window) {
        __android_log_print(ANDROID_LOG_ERROR, "gltron", "Failed to get native window");
        return;
    }

    // Use Android's API via Java (if unavoidable)
    // However, you still need Java if you rely on Android's visibility flags.

    // Alternatively, you could use Android's Native API to set UI flags, but it's not straightforward without Java.
    // So Java is still required — but only for immersive flags.
}

static int init_egl(ANativeWindow* window, struct android_app* app) {
  s_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
  eglInitialize(s_display, 0, 0);
  EGLint eglErr = eglGetError();
  if (eglErr != EGL_SUCCESS) { __android_log_print(ANDROID_LOG_ERROR, "gltron", "eglInitialize error: 0x%04x", eglErr); }
  // Ensure asset manager/base path are set before any resource loads
  // Note: We can only set them here if we have a valid activity (passed via android_main)

  const EGLint cfgAttribs[] = {
    EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
    EGL_RED_SIZE, 8,
    EGL_GREEN_SIZE, 8,
    EGL_BLUE_SIZE, 8,
    EGL_DEPTH_SIZE, 16,
    EGL_NONE
  };
  EGLConfig config; EGLint numCfg;
  eglChooseConfig(s_display, cfgAttribs, &config, 1, &numCfg);
  eglErr = eglGetError();
  if (eglErr != EGL_SUCCESS) { __android_log_print(ANDROID_LOG_ERROR, "gltron", "eglChooseConfig error: 0x%04x", eglErr); }
  EGLint format;
  eglGetConfigAttrib(s_display, config, EGL_NATIVE_VISUAL_ID, &format);
  ANativeWindow_setBuffersGeometry(window, 0, 0, format);

  s_surface = eglCreateWindowSurface(s_display, config, window, NULL);
  eglErr = eglGetError();
  if (eglErr != EGL_SUCCESS) { __android_log_print(ANDROID_LOG_ERROR, "gltron", "eglCreateWindowSurface error: 0x%04x", eglErr); }

  const EGLint ctxAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
  s_context = eglCreateContext(s_display, config, EGL_NO_CONTEXT, ctxAttribs);
  eglErr = eglGetError();
  if (eglErr != EGL_SUCCESS) { __android_log_print(ANDROID_LOG_ERROR, "gltron", "eglCreateContext error: 0x%04x", eglErr); }
  if (s_display == EGL_NO_DISPLAY || s_surface == EGL_NO_SURFACE || s_context == EGL_NO_CONTEXT) {
    __android_log_print(ANDROID_LOG_ERROR, "gltron", "EGL create failed");
    return 0;
  }
  eglMakeCurrent(s_display, s_surface, s_surface, s_context);
  eglErr = eglGetError();
  if (eglErr != EGL_SUCCESS) { __android_log_print(ANDROID_LOG_ERROR, "gltron", "eglMakeCurrent error: 0x%04x", eglErr); }
  eglQuerySurface(s_display, s_surface, EGL_WIDTH, &s_width);
  eglQuerySurface(s_display, s_surface, EGL_HEIGHT, &s_height);

  // Log GL info and confirm we use OpenGL ES via EGL (not Vulkan)
  const char* vendor   = (const char*)glGetString(GL_VENDOR);
  const char* renderer = (const char*)glGetString(GL_RENDERER);
  const char* version  = (const char*)glGetString(GL_VERSION);
  const char* slver    = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
  __android_log_print(ANDROID_LOG_INFO, "gltron", "OpenGL ES context created");
  __android_log_print(ANDROID_LOG_INFO, "gltron", "GL_VENDOR: %s", vendor ? vendor : "(null)");
  __android_log_print(ANDROID_LOG_INFO, "gltron", "GL_RENDERER: %s", renderer ? renderer : "(null)");
  __android_log_print(ANDROID_LOG_INFO, "gltron", "GL_VERSION: %s", version ? version : "(null)");
  __android_log_print(ANDROID_LOG_INFO, "gltron", "GL_SHADING_LANGUAGE_VERSION: %s", slver ? slver : "(null)");
  __android_log_print(ANDROID_LOG_INFO, "gltron", "Using OpenGL ES via EGL (no Vulkan)");

  gltron_init();
  // Initialize centralized shaders after GL context is current
  init_shaders_android();
  gltron_resize((int)s_width, (int)s_height);
  glViewport(0, 0, (GLint)s_width, (GLint)s_height);
  glClearColor(0.f, 0.f, 0.f, 1.f);
  // Initialize textures/fonts and any display-related GL resources on Android
  setupDisplay(game->screen);
  // Initialize GUI GL state explicitly after GLES context is ready
  initGLGui();
  // Set a neutral clear color; GUI will paint over it
  glClearColor(0.f, 0.f, 0.f, 1.f);
  // Ensure we start in GUI (menu)
  extern callbacks guiCallbacks;
  switchCallbacks(&guiCallbacks);
  return 1;
}

static void term_egl() {
  if (s_display != EGL_NO_DISPLAY) {
    eglMakeCurrent(s_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    if (s_context != EGL_NO_CONTEXT) eglDestroyContext(s_display, s_context);
    if (s_surface != EGL_NO_SURFACE) eglDestroySurface(s_display, s_surface);
    eglTerminate(s_display);
  }
  s_display = EGL_NO_DISPLAY; s_surface = EGL_NO_SURFACE; s_context = EGL_NO_CONTEXT;
}

static int32_t handle_input(struct android_app* app, AInputEvent* event) {
  (void)app;
  int32_t type = AInputEvent_getType(event);
  if (type == AINPUT_EVENT_TYPE_MOTION) {
    int32_t action = AMotionEvent_getAction(event) & AMOTION_EVENT_ACTION_MASK;
    float x = AMotionEvent_getX(event, 0);
    float y = AMotionEvent_getY(event, 0);
    gltron_on_touch(x, y, action);
    return 1;
  } else if (type == AINPUT_EVENT_TYPE_KEY) {
    int32_t action = AKeyEvent_getAction(event);
    int32_t code = AKeyEvent_getKeyCode(event);
    int key = 0;
    switch (code) {
      case AKEYCODE_DPAD_LEFT: key = 100; break; // GLUT_KEY_LEFT
      case AKEYCODE_DPAD_UP: key = 101; break;   // GLUT_KEY_UP
      case AKEYCODE_DPAD_RIGHT: key = 102; break;// GLUT_KEY_RIGHT
      case AKEYCODE_DPAD_DOWN: key = 103; break; // GLUT_KEY_DOWN
      case AKEYCODE_SPACE: key = ' '; break;
      case AKEYCODE_A: key = 'a'; break; case AKEYCODE_D: key = 'd'; break;
      case AKEYCODE_S: key = 's'; break; case AKEYCODE_K: key = 'k'; break;
      case AKEYCODE_L: key = 'l'; break;
      default: key = 0; break;
    }
    if (key) {
      // Key input disabled; touch-only control.
      return 1;
    }
  }
  return 0;
}

int g_finish_requested = 0;

static void finish_activity(struct android_app* app) {
  if (!app || !app->activity || !app->activity->clazz) return;
  JNIEnv* env = NULL;
  (*app->activity->vm)->AttachCurrentThread(app->activity->vm, &env, NULL);
  if (!env) return;
  jobject activity = app->activity->clazz;
  jclass activityClass = (*env)->GetObjectClass(env, activity);
  jmethodID finish = (*env)->GetMethodID(env, activityClass, "finish", "()V");
  if (finish) {
    (*env)->CallVoidMethod(env, activity, finish);
  } else {
    __android_log_print(ANDROID_LOG_ERROR, "gltron", "Failed to find Activity.finish method");
    (*env)->ExceptionClear(env);
  }
  (*app->activity->vm)->DetachCurrentThread(app->activity->vm);
}

static void handle_cmd(struct android_app* app, int32_t cmd) {
  switch (cmd) {
    case APP_CMD_INIT_WINDOW:
      if (app->window) {
        if (!init_egl(app->window, app)) {
          __android_log_print(ANDROID_LOG_ERROR, "gltron", "Failed to init EGL");
        } else {
          // Apply immersive fullscreen from native code
          if (game && game->settings && game->settings->fullscreen) {
            __android_log_print(ANDROID_LOG_INFO, "gltron", "Applying immersive fullscreen: game->settings->fullscreen = %d", game->settings->fullscreen);

            set_immersive_fullscreen(app);
          }
        }
      }
      break;
    case APP_CMD_TERM_WINDOW:
      term_egl();
      break;
    case APP_CMD_WINDOW_RESIZED:
      if (s_display != EGL_NO_DISPLAY && app->window) {
        eglQuerySurface(s_display, s_surface, EGL_WIDTH, &s_width);
        eglQuerySurface(s_display, s_surface, EGL_HEIGHT, &s_height);
        gltron_resize((int)s_width, (int)s_height);
        glViewport(0, 0, (GLint)s_width, (GLint)s_height);
      }
      break;
    default:
      break;
  }
}

void android_main(struct android_app* state) {
  state->onAppCmd = handle_cmd;
  state->onInputEvent = handle_input;
  if (state->activity && state->activity->assetManager) {
    gltron_set_asset_manager((void*)state->activity->assetManager);
    // Set base path to the app's internal files dir if available
    // NativeActivity provides internalDataPath
    if (state->activity->internalDataPath) {
      gltron_set_base_path(state->activity->internalDataPath);
    }
  }

  int events;
  struct android_poll_source* source;
  int ident;
  int timeout;

  while (1) {
    // Use ALooper_pollOnce instead of ALooper_pollAll
    while ((ident = ALooper_pollOnce(0, NULL, &events, (void**)&source)) >= 0) {
      if (source != NULL) {
        source->process(state, source);
      }

      if (state->destroyRequested) {
        term_egl();
        return;
      }
    }

    if (s_display != EGL_NO_DISPLAY && s_surface != EGL_NO_SURFACE) {
      gltron_frame();
      EGLBoolean ok = eglSwapBuffers(s_display, s_surface);
      if (!ok) {
        EGLint e = eglGetError();
        __android_log_print(ANDROID_LOG_ERROR, "gltron", "eglSwapBuffers failed: 0x%04x", e);
      }
      if (g_finish_requested) {
        __android_log_print(ANDROID_LOG_INFO, "gltron", "Finishing activity on BACK at top-level");
        finish_activity(state);
        g_finish_requested = 0;
      }
    }
  }
}

#endif // ANDROID
