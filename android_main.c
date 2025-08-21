#ifdef ANDROID
#include <android/log.h>
#include <android/native_activity.h>
#include <android/native_window.h>
#include <android_native_app_glue.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include "android_glue.h"

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "gltron", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "gltron", __VA_ARGS__)

static EGLDisplay s_display = EGL_NO_DISPLAY;
static EGLSurface s_surface = EGL_NO_SURFACE;
static EGLContext s_context = EGL_NO_CONTEXT;
static int32_t s_width = 0, s_height = 0;

static int init_egl(ANativeWindow* window) {
  s_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
  eglInitialize(s_display, 0, 0);

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
  EGLint format;
  eglGetConfigAttrib(s_display, config, EGL_NATIVE_VISUAL_ID, &format);
  ANativeWindow_setBuffersGeometry(window, 0, 0, format);

  s_surface = eglCreateWindowSurface(s_display, config, window, NULL);

  const EGLint ctxAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
  s_context = eglCreateContext(s_display, config, EGL_NO_CONTEXT, ctxAttribs);
  if (s_display == EGL_NO_DISPLAY || s_surface == EGL_NO_SURFACE || s_context == EGL_NO_CONTEXT) {
    LOGE("EGL create failed");
    return 0;
  }
  eglMakeCurrent(s_display, s_surface, s_surface, s_context);
  eglQuerySurface(s_display, s_surface, EGL_WIDTH, &s_width);
  eglQuerySurface(s_display, s_surface, EGL_HEIGHT, &s_height);
  gltron_init();
  gltron_resize((int)s_width, (int)s_height);
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
      gltron_on_key(key, action);
      return 1;
    }
  }
  return 0;
}

static void handle_cmd(struct android_app* app, int32_t cmd) {
  switch (cmd) {
    case APP_CMD_INIT_WINDOW:
      if (app->window) {
        if (!init_egl(app->window)) {
          LOGE("Failed to init EGL");
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
      }
      break;
    default:
      break;
  }
}

void android_main(struct android_app* state) {
  app_dummy();
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
      eglSwapBuffers(s_display, s_surface);
    }
  }
}

#endif // ANDROID
