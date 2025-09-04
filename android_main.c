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

// Helper to check for and log any Java exceptions
void checkException(JNIEnv* env) {
    if ((*env)->ExceptionCheck(env)) {
        __android_log_print(ANDROID_LOG_ERROR, "gltron", "Exception thrown in JNI call. Logging stack trace:");
        (*env)->ExceptionDescribe(env);
        (*env)->ExceptionClear(env);
    }
}

/* Set_immersive_fullscreen */
static void set_immersive_fullscreen_jni(struct android_app* app) {
  JNIEnv* env;
  jobject activity = app->activity->clazz;

  if (!activity) {
    __android_log_print(ANDROID_LOG_WARN, "gltron", "No activity object found - immersion skipped.");
    return;
  }

  // Attach to current thread if necessary
  JNIEnv* threadEnv;
  if ((*app->activity->vm)->GetEnv(app->activity->vm, (void**)&threadEnv, JNI_VERSION_1_6) != JNI_OK) {
    if ((*app->activity->vm)->AttachCurrentThread(app->activity->vm, &threadEnv, NULL) != JNI_OK) {
      __android_log_print(ANDROID_LOG_WARN, "gltron", "Failed to attach thread for JNI.");
      return;
    }
  }

  jclass activityCls = (*threadEnv)->GetObjectClass(threadEnv, activity);
  if (!activityCls) {
    __android_log_print(ANDROID_LOG_WARN, "gltron", "Failed to get Activity class.");
    goto done;
  }

  // Call Activity.getWindow():Window
  jmethodID getWindow = (*threadEnv)->GetMethodID(threadEnv, activityCls, "getWindow", "()Landroid/view/Window;");
  if (!getWindow) {
    __android_log_print(ANDROID_LOG_WARN, "gltron", "getWindow not found.");
    goto done;
  }
  jobject window = (*threadEnv)->CallObjectMethod(threadEnv, activity, getWindow);
  if (!window) {
    __android_log_print(ANDROID_LOG_WARN, "gltron", "Window is null.");
    goto done;
  }

  // Call Window.getDecorView():View
  jclass windowCls = (*threadEnv)->GetObjectClass(threadEnv, window);
  if (!windowCls) {
    __android_log_print(ANDROID_LOG_WARN, "gltron", "Window class not found.");
    goto done;
  }

  jmethodID getDecorView = (*threadEnv)->GetMethodID(threadEnv, windowCls, "getDecorView", "()Landroid/view/View;");
  if (!getDecorView) {
    __android_log_print(ANDROID_LOG_WARN, "gltron", "getDecorView not found.");
    goto done;
  }
  jobject decorView = (*threadEnv)->CallObjectMethod(threadEnv, window, getDecorView);
  if (!decorView) {
    __android_log_print(ANDROID_LOG_WARN, "gltron", "DecorView is null.");
    goto done;
  }

  // Call View.setSystemUiVisibility(int):int
  jclass viewCls = (*threadEnv)->GetObjectClass(threadEnv, decorView);
  if (!viewCls) {
    __android_log_print(ANDROID_LOG_WARN, "gltron", "View class not found.");
    goto done;
  }

  jmethodID setSystemUiVisibility = (*threadEnv)->GetMethodID(threadEnv, viewCls, "setSystemUiVisibility", "(I)I");
  if (!setSystemUiVisibility) {
    __android_log_print(ANDROID_LOG_WARN, "gltron", "setSystemUiVisibility not found.");
    goto done;
  }

  int flags = SYSTEM_UI_FLAG_LAYOUT_STABLE
              | SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
              | SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
              | SYSTEM_UI_FLAG_HIDE_NAVIGATION
              | SYSTEM_UI_FLAG_FULLSCREEN
              | SYSTEM_UI_FLAG_IMMERSIVE;
  
  int result = (*threadEnv)->CallIntMethod(threadEnv, decorView, setSystemUiVisibility, flags);
  checkException(threadEnv);
  __android_log_print(ANDROID_LOG_INFO, "gltron", "JNI Immersive mode applied. Result code: %d", result);

done:
  // Clean up references
  if (activityCls) (*threadEnv)->DeleteLocalRef(threadEnv, activityCls);
  if (window) (*threadEnv)->DeleteLocalRef(threadEnv, window);
  if (windowCls) (*threadEnv)->DeleteLocalRef(threadEnv, windowCls);
  if (decorView) (*threadEnv)->DeleteLocalRef(threadEnv, decorView);
  if (viewCls) (*threadEnv)->DeleteLocalRef(threadEnv, viewCls);

  // Detach if needed
  if (threadEnv != app->activity->env) {
    (*app->activity->vm)->DetachCurrentThread(app->activity->vm);
  }
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
      if (app->window && app->activity && app->activity->clazz) {
        if (init_egl(app->window, app)) {
          // Apply immersive mode via pure JNI
          set_immersive_fullscreen_jni(app);
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
