#include "android_glue.h"
#include "shaders.h"
#include "globals.h"
#include "gltron.h"
#include <string.h>

extern int scr_w, scr_h;
void update_buttons_layout();
extern callbacks *current_callback;

// base path buffer exposed for file.c
#ifndef PATH_MAX
#define PATH_MAX 1024
#endif
char s_base_path[PATH_MAX] = "/data/data/org.gltron.game/files"; // default fallback

#ifdef ANDROID

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

static int initialized = 0;
// base path defined above (non-static) and declared in header

// Allow host (NativeActivity) to pass in the AAssetManager
void gltron_set_asset_manager(void* mgr) {
  g_android_asset_mgr = (AAssetManager*)mgr;
}

void gltron_set_base_path(const char* base_path) {
  if (!base_path) return;
  size_t n = strlen(base_path);
  if (n >= sizeof(s_base_path)) n = sizeof(s_base_path) - 1;
  memcpy(s_base_path, base_path, n);
  s_base_path[n] = '\0';
}

void gltron_init(void) {
  if (initialized) return;
#ifdef ANDROID
  __android_log_print(ANDROID_LOG_INFO, "gltron", "gltron_init: base_path='%s' mgr=%p", s_base_path, g_android_asset_mgr);
#endif
  // Initialize core game structures and data
#ifdef ANDROID
  // Prefer internal writable settings file under s_base_path
  char settingsPathBuf[PATH_MAX];
  settingsPathBuf[0] = '\0';
  if (s_base_path[0]) {
    snprintf(settingsPathBuf, sizeof(settingsPathBuf), "%s/%s", s_base_path, "settings.txt");
    initMainGameSettings(settingsPathBuf);
  } else {
    initMainGameSettings("settings.txt");
  }
#else
  initMainGameSettings("settings.txt");
#endif
  // Load menu.txt similar to desktop gltron.c
  char *path = getFullPath("menu.txt");
  if (path) {
    pMenuList = loadMenuFile(path);
#ifdef ANDROID
    __android_log_print(ANDROID_LOG_INFO, "gltron", "menu.txt loaded from %s", path);
#endif
    free(path);
  } else {
#ifdef ANDROID
    __android_log_print(ANDROID_LOG_ERROR, "gltron", "Failed to load menu.txt via getFullPath");
#endif
  }
  initGameStructures();
  resetScores();
  initData();

  // Initialize shaders and font system early on Android
#ifdef ANDROID
  init_shaders_android();
  // initialize font texture/renderer
  if (ftx == NULL) {
    initFonts();
  }
#endif
  // Ensure screen struct exists
  if (!game->screen) {
    game->screen = (gDisplay*)malloc(sizeof(gDisplay));
    memset(game->screen, 0, sizeof(gDisplay));
  }
  initialized = 1;
  // Ensure we start in GUI (menu) on Android
  switchCallbacks(&guiCallbacks);
}

void gltron_resize(int width, int height) {
  if (!initialized) gltron_init();
  // Update settings and screen dimensions
  game->settings->width = width;
  game->settings->height = height;
  if (game && game->screen) {
    game->screen->w = width;
    game->screen->h = height;
    game->screen->vp_w = width;
    game->screen->vp_h = height;
  }
  scr_w = width;
  scr_h = height;
  update_buttons_layout();
  // Let the game react to display changes
  requestDisplayApply();
  applyDisplaySettingsDeferred();
}

// Simple Android on-screen controls
static int btn_left[4];   // x, y, w, h
static int btn_right[4];  // x, y, w, h
static int btn_pause[4];  // x, y, w, h

void update_buttons_layout() {
  if (!scr_w || !scr_h) return;
  int margin = scr_w / 40;
  int btnSize = scr_w / 6;
  if (btnSize < 64) btnSize = 64;
  // Left bottom corner
  btn_left[0] = margin;
  btn_left[1] = scr_h - btnSize - margin;
  btn_left[2] = btnSize;
  btn_left[3] = btnSize;
  // Right bottom corner
  btn_right[0] = scr_w - btnSize - margin;
  btn_right[1] = scr_h - btnSize - margin;
  btn_right[2] = btnSize;
  btn_right[3] = btnSize;
  // Pause at top center
  btn_pause[2] = btnSize * 0.8f;
  btn_pause[3] = btnSize * 0.6f;
  btn_pause[0] = (scr_w - btn_pause[2]) / 2;
  btn_pause[1] = margin;
}
static int active_left = 0;
static int active_right = 0;
static int active_pause = 0;

static int hit_btn(int x, int y, int* btn) {
  return (x >= btn[0] && x <= btn[0]+btn[2] && y >= btn[1] && y <= btn[1]+btn[3]);
}

static void draw_rect(int x, int y, int w, int h, float r, float g, float b, float a) {
  // Set up orthographic projection
  GLfloat projection[16] = {
    2.0f/scr_w, 0, 0, 0,
    0, -2.0f/scr_h, 0, 0,
    0, 0, -1, 0,
    -1, 1, 0, 1
  };

  // Set up model-view matrix (identity)
  GLfloat modelView[16] = {
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1
  };

  // Set up vertex data
  GLfloat vertices[8] = {
    x, y,
    x+w, y,
    x+w, y+h,
    x, y+h
  };

  // Use shader program
  GLuint shaderProgram = shader_get_basic();
  if (!shaderProgram) return;
  glUseProgram(shaderProgram);

  // Set up matrices
  GLint projectionLoc = glGetUniformLocation(shaderProgram, "projectionMatrix");
  GLint modelViewLoc = glGetUniformLocation(shaderProgram, "modelView");
  GLint colorLoc = glGetUniformLocation(shaderProgram, "color");

  glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, projection);
  glUniformMatrix4fv(modelViewLoc, 1, GL_FALSE, modelView);
  glUniform4f(colorLoc, r, g, b, a);

  // Set up vertex attribute
  GLint positionLoc = glGetAttribLocation(shaderProgram, "position");
  glEnableVertexAttribArray(positionLoc);
  // Ensure client array pointer is used (no VBO bound)
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glVertexAttribPointer(positionLoc, 2, GL_FLOAT, GL_FALSE, 0, vertices);

  // Draw the rectangle
  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

  // Disable vertex attribute
  glDisableVertexAttribArray(positionLoc);
}

static int is_gui_active() {
  extern callbacks guiCallbacks; // declared in other compilation units
  return current_callback == &guiCallbacks;
}

static void draw_android_overlay() {
  if (!scr_w || !scr_h) return;
  if (is_gui_active()) return; // hide controls in menus
  draw_rect(btn_left[0], btn_left[1], btn_left[2], btn_left[3], 1,1,1,0.35f);
  draw_rect(btn_right[0], btn_right[1], btn_right[2], btn_right[3], 1,1,1,0.35f);
  draw_rect(btn_pause[0], btn_pause[1], btn_pause[2], btn_pause[3], 1,1,1,0.30f);
}

void gltron_frame(void) {
  if (!initialized) gltron_init();
  // Continuous input while buttons are active
  if (active_left) {
    keyGame('a', 0, 0);
  } else if (active_right) {
    keyGame('s', 0, 0);
  }
  // Simulate the usual idle/display cycle based on current callback
  if (current_callback && current_callback->idle) current_callback->idle();
  // Clear and render
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  if (current_callback && current_callback->display) current_callback->display();
  // Overlay controls (hidden in GUI)
  draw_android_overlay();
}

/*void gltron_on_key(int key, int action) {
  (void)action; // not used currently
  // Forward to existing keyboard handler via ASCII mapping
  switch (key) {
    case 27: // ESC
    case ' ':
    case 'A': case 'D': case 'S':
    case 'k': case 'K': case 'l': case 'L': case '5': case '6':
      keyGame((unsigned char)key, 0, 0);
      break;
    case 'a':
      keyGame('a', 0, 0);
      break;
    case 'd':
      keyGame('d', 0, 0);
      break;
    case 's':
      keyGame('s', 0, 0);
      break;
    case GLUT_KEY_LEFT:
      keyGame('a', 0, 0);
      break;
    case GLUT_KEY_RIGHT:
      keyGame('s', 0, 0);
      break;
    case GLUT_KEY_UP:
      keyGame('k', 0, 0);
      break;
    case GLUT_KEY_DOWN:
      keyGame('l', 0, 0);
      break;
    default:
      keyGame((unsigned char)key, 0, 0);
      break;
  }
}*/

void gltron_on_touch(float x, float y, int action) {
  // If paused and not game finished, unpause on touch-up
  if (game && (game->pauseflag != 0)) {
    // AMOTION_EVENT_ACTION_UP == 1 (mask already applied by caller)
    if ((action & 0xFF) == 1 /* ACTION_UP */ && !(game->pauseflag & PAUSE_GAME_FINISHED)) {
      switchCallbacks(&gameCallbacks);
      game->pauseflag = 0;
      return;
    }
  }
  // Otherwise, route to GUI or handle virtual buttons
  int ix = (int)x;
  int iy = (int)y;
  // Check virtual buttons first
  int actionMasked = (action & 0xFF);
  // If GUI (menu) is active, route all touches to menu handlers
  if (is_gui_active()) {
    motionGui(ix, iy);
    if (actionMasked == 1 /* UP */) {
      mouseGui(0, 0, ix, iy);
    }
    return;
  }
  // Manage active button states for in-game overlay
  if (actionMasked == 0 /* ACTION_DOWN */ || actionMasked == 2 /* ACTION_MOVE */) {
    active_left = hit_btn(ix, iy, btn_left);
    active_right = hit_btn(ix, iy, btn_right);
    active_pause = hit_btn(ix, iy, btn_pause);
    if (active_pause && actionMasked == 0) {
      keyGame(' ', 0, 0);
      return;
    }
    if (active_left || active_right) {
      // Immediate response on down/move
      if (active_left) keyGame('a', 0, 0);
      if (active_right) keyGame('s', 0, 0);
      return;
    }
  } else if (actionMasked == 1 /* ACTION_UP */) {
    active_left = active_right = active_pause = 0;
  }
  // Otherwise no overlay hit; ignore or add game-world touch handling if desired
}

#endif // ANDROID
