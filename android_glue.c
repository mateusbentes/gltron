#include "android_glue.h"
#include "sound_backend.h"
#include "shaders.h"
#include "globals.h"
#include "gltron.h"
#include <string.h>
#include <errno.h>
#ifdef ANDROID
#include <android/log.h>
#include <android/asset_manager.h>
#include <sys/stat.h>
#endif

/* extern flag from android_main.c to request Activity.finish() */
extern int g_finish_requested;

/* Local helper: detect BACK hit in GUI using same viewport math as gui_mouse.c */
static int hit_test_back_in_viewport(int x_win, int y_win) {
  if (!game || !game->screen) return 0;
  int vx = game->screen->vp_x;
  int vy = game->screen->vp_y;
  int vw = game->screen->vp_w;
  int vh = game->screen->vp_h;
  int x_local = x_win - vx;
  int y_local = vh - (y_win - vy);
  int size = vw / 32;
  const float ANDROID_GUI_HIT_HEIGHT_SCALE = 1.25f;
  const float ANDROID_GUI_HIT_WIDTH_PER_CHAR = 0.70f;
  const int   ANDROID_GUI_HIT_XPAD = 3;
  int len = 4; /* "Back" */
  int bx = vw - (int)(size * 4.5f);
  int by = (int)(size * 1.2f);
  int rect_x0 = bx - (int)(0.5f * size);
  int rect_y0 = by - (int)(0.5f * size);
  int rect_w = (int)(size * ANDROID_GUI_HIT_WIDTH_PER_CHAR * len) + size * (ANDROID_GUI_HIT_XPAD + 1);
  int rect_h = (int)(size * 2.0f * ANDROID_GUI_HIT_HEIGHT_SCALE);
  return (x_local >= rect_x0 && x_local <= rect_x0 + rect_w &&
          y_local >= rect_y0 && y_local <= rect_y0 + rect_h);
}

/* Local helper: detect menu item index at window coords; returns >=0 for item, -2 for Back, -1 for none */
static int hit_test_menu_item_in_viewport(int x_win, int y_win) {
  if (!game || !game->screen || !pCurrent) return -1;
  int vx = game->screen->vp_x;
  int vy = game->screen->vp_y;
  int vw = game->screen->vp_w;
  int vh = game->screen->vp_h;
  int x_local = x_win - vx;
  int y_local = vh - (y_win - vy);
  int size = vw / 32;
  int lineheight = size * 2;
  int x_start = vw / 6;
  int y_start = 2 * vh / 3;
  const float ANDROID_GUI_HIT_Y_OFFSET = 0.10f;
  const float ANDROID_GUI_HIT_HEIGHT_SCALE = 1.25f;
  const float ANDROID_GUI_HIT_WIDTH_PER_CHAR = 0.70f;
  const int   ANDROID_GUI_HIT_XPAD = 3;
  /* Back first */
  if (hit_test_back_in_viewport(x_win, y_win)) return -2;
  if (lineheight <= 0) return -1;
  for (int i = 0; i < pCurrent->nEntries; ++i) {
    int item_y_base = y_start - i * lineheight;
    int rect_x0 = x_start;
    int rect_y0 = item_y_base - (int)((1.3f + ANDROID_GUI_HIT_Y_OFFSET) * size);
    Menu* m = (Menu*)*(pCurrent->pEntries + i);
    const char* txt = m ? m->display.szCaption : "";
    int len = txt ? (int)strlen(txt) : 0;
    int rect_w = (int)(size * ANDROID_GUI_HIT_WIDTH_PER_CHAR * len) + size * ANDROID_GUI_HIT_XPAD;
    int rect_h = (int)(lineheight * ANDROID_GUI_HIT_HEIGHT_SCALE);
    if (x_local >= rect_x0 && x_local <= rect_x0 + rect_w &&
        y_local >= rect_y0 && y_local <= rect_y0 + rect_h) {
      return i;
    }
  }
  return -1;
}

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

static void init_settings_android() {
  // Load settings.txt similar to desktop gltron.c
  char *path = getFullPath("settings.txt");
  if (path != NULL) {
    initMainGameSettings(path); /* reads defaults from ~/.gltronrc */
    free(path);
  } else {
    printf("fatal: could not find settings.txt, exiting...\n");
    exit(1);
  }
}

void gltron_init(void) {
  if (initialized) return;
  __android_log_print(ANDROID_LOG_INFO, "gltron", "gltron_init: base_path='%s' mgr=%p", s_base_path, g_android_asset_mgr);
  init_settings_android();
  // Ensure touch input is enabled on Android
  if (game && game->settings) {
    game->settings->input_mode = 1; // enable touch/mouse input
  }
  // Load menu.txt similar to desktop gltron.c
  char *path = getFullPath("menu.txt");
  if (path) {
    pMenuList = loadMenuFile(path);
    __android_log_print(ANDROID_LOG_INFO, "gltron", "menu.txt loaded from %s", path);
    free(path);
  } else {
    __android_log_print(ANDROID_LOG_WARN, "gltron", "getFullPath failed for menu.txt, trying direct asset stream");
    if (g_android_asset_mgr) {
      AAsset* asset = AAssetManager_open(g_android_asset_mgr, "menu.txt", AASSET_MODE_STREAMING);
      if (asset) {
        off_t len = AAsset_getLength(asset);
        char* buf = (char*)malloc((size_t)len + 1);
        if (buf) {
          int total = 0;
          while (total < len) {
            int r = AAsset_read(asset, buf + total, (size_t)(len - total));
            if (r <= 0) break;
            total += r;
          }
          buf[total] = '\0';
          if (total > 0) {
            pMenuList = loadMenuFromBuffer(buf);
            if (!pMenuList) {
              __android_log_print(ANDROID_LOG_ERROR, "gltron", "loadMenuFromBuffer failed for menu.txt");
            }
          }
          free(buf);
        }
        AAsset_close(asset);
      } else {
        __android_log_print(ANDROID_LOG_ERROR, "gltron", "AAssetManager_open(menu.txt) failed");
      }
    }
    if (!pMenuList) {
      __android_log_print(ANDROID_LOG_ERROR, "gltron", "Failed to load menu.txt via all fallbacks");
    }
  }
  // Diagnostics and fallback menu on Android
  if (pMenuList && pMenuList[0]) {
    // Ensure current menu pointer is set
    pCurrent = pMenuList[0];
  } else {
    __android_log_print(ANDROID_LOG_ERROR, "gltron", "menu load failed; creating minimal fallback menu");
    // Create a minimal menu hierarchy with 1 top-level menu and a few entries
    Menu* root = (Menu*)calloc(1, sizeof(Menu));
    strcpy(root->szName, "mRoot");
    strcpy(root->szCapFormat, "GLTron");
    root->nEntries = 2;
    root->pEntries = (Menu**)calloc(root->nEntries, sizeof(Menu*));
    for (int i=0; i<root->nEntries; ++i) root->pEntries[i] = (Menu*)calloc(1, sizeof(Menu));
    strcpy(root->pEntries[0]->szName, "cGame");
    strcpy(root->pEntries[0]->szCapFormat, "Start");
    strcpy(root->pEntries[1]->szName, "pq");
    strcpy(root->pEntries[1]->szCapFormat, "Quit");
    pMenuList = (Menu**)calloc(1, sizeof(Menu*));
    pMenuList[0] = root;
    pCurrent = root;
  }

  initGameStructures();
  resetScores();
  initData();
  // Ensure unpaused when starting a forced game later
  if (game) game->pauseflag = 0;

  // Initialize sound backend and try to load/play music on Android
#ifdef ANDROID
  if (sb_init()) {
    __android_log_print(ANDROID_LOG_INFO, "gltron", "sound: sb_init OK, loading music");
    if (!sb_load_music("gltron.it")) {
      __android_log_print(ANDROID_LOG_WARN, "gltron", "sound: sb_load_music(gltron.it) failed");
    } else {
      sb_play_music();
      __android_log_print(ANDROID_LOG_INFO, "gltron", "sound: music started");
    }
  } else {
    __android_log_print(ANDROID_LOG_ERROR, "gltron", "sound: sb_init failed");
  }
#endif

  // Initialize shaders and font system early on Android
  init_shaders_android();
  // initialize font texture/renderer
  if (ftx == NULL) {
    initFonts();
  }
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
  // Always draw a small debug banner at top to prove overlay works
  draw_rect(8, 8, scr_w/3, scr_h/20, 1.f, 1.f, 1.f, 0.15f);
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
#ifdef ANDROID
  static int logged_cb = 0;
  if (!logged_cb) {
    __android_log_print(ANDROID_LOG_INFO, "gltron", "frame: current_callback=%p display=%p idle=%p", current_callback, current_callback ? current_callback->display : NULL, current_callback ? current_callback->idle : NULL);
    logged_cb = 1;
  }
#endif
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
  // Normalize to pixel coords and fix Y origin mismatch if needed
  int ix = (int)(x + 0.5f);
  int iy = (int)(y + 0.5f);
  if (ix < 0) ix = 0; if (iy < 0) iy = 0;
  if (scr_w > 0 && ix >= scr_w) ix = scr_w - 1;
  if (scr_h > 0 && iy >= scr_h) iy = scr_h - 1;

  // Our UI math (update_buttons_layout/draw_rect) assumes origin at top-left.
  // On some Android setups touch Y is bottom-left; flip if a build-flag is set.
#ifndef ANDROID_TOUCH_TOPLEFT
#define ANDROID_TOUCH_TOPLEFT 1
#endif
#if ANDROID_TOUCH_TOPLEFT == 0
  if (scr_h > 0) iy = scr_h - 1 - iy;
#endif

  // If paused and not game finished, unpause on touch-up
  if (game && (game->pauseflag != 0)) {
    if ((action & 0xFF) == 1 /* ACTION_UP */ && !(game->pauseflag & PAUSE_GAME_FINISHED)) {
      switchCallbacks(&gameCallbacks);
      game->pauseflag = 0;
      return;
    }
  }

  int actionMasked = (action & 0xFF);

  // If GUI (menu) is active, route all touches to menu handlers
  if (is_gui_active()) {
#ifdef ANDROID
    if (actionMasked == 0) __android_log_print(ANDROID_LOG_INFO, "gltron", "touch->GUI: down %d,%d", ix, iy);
    if (actionMasked == 2) __android_log_print(ANDROID_LOG_INFO, "gltron", "touch->GUI: move %d,%d", ix, iy);
    if (actionMasked == 1) __android_log_print(ANDROID_LOG_INFO, "gltron", "touch->GUI: up %d,%d (click)", ix, iy);
#endif
    // Tap tolerance for BACK: remember DOWN position and whether it started on BACK
    static int s_gui_down_on_back = 0; static int s_down_x = 0, s_down_y = 0; const int tol = 24;
    if (actionMasked == 0) {
      s_down_x = ix; s_down_y = iy;
      s_gui_down_on_back = hit_test_back_in_viewport(ix, iy);
      motionGui(ix, iy);
    } else if (actionMasked == 2) {
      motionGui(ix, iy);
    } else if (actionMasked == 1) {
      if (s_gui_down_on_back) {
        int dx = ix - s_down_x; if (dx < 0) dx = -dx;
        int dy = iy - s_down_y; if (dy < 0) dy = -dy;
        if (dx <= tol && dy <= tol) {
#ifdef ANDROID
          __android_log_print(ANDROID_LOG_INFO, "gltron", "touch->GUI: BACK via tap-tolerance");
#endif
          requestDisplayApply();
          if (!pCurrent || pCurrent->parent == NULL) {
            restoreCallbacks();
            g_finish_requested = 1;
          } else {
            pCurrent = pCurrent->parent;
            pCurrent->iHighlight = -1;
            switchCallbacks(&guiCallbacks);
            requestDisplayApply();
            applyDisplaySettingsDeferred();
          }
          s_gui_down_on_back = 0;
          return;
        }
      }
      // Direct dispatch based on a fresh hit-test at release
      {
        int idx = hit_test_menu_item_in_viewport(ix, iy);
        if (idx == -2) {
          // BACK
          requestDisplayApply();
          if (!pCurrent || pCurrent->parent == NULL) {
            restoreCallbacks();
            g_finish_requested = 1;
          } else {
            pCurrent = pCurrent->parent;
            pCurrent->iHighlight = -1;
            switchCallbacks(&guiCallbacks);
            requestDisplayApply();
            applyDisplaySettingsDeferred();
          }
        } else if (idx >= 0) {
          pCurrent->iHighlight = idx;
          // If selecting 'Start Game' from minimal menu, ensure unpaused and force viewport reset
      if (*(pCurrent->pEntries + pCurrent->iHighlight) && strcmp((*(pCurrent->pEntries + pCurrent->iHighlight))->szCapFormat, "Start") == 0) {
        if (game) game->pauseflag = 0;
        // Force a viewport/projection reset safely via public API
        requestDisplayApply();
        applyDisplaySettingsDeferred();
      }
      menuAction(*(pCurrent->pEntries + pCurrent->iHighlight));
        }
      }
      s_gui_down_on_back = 0;
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
    // Not over overlay buttons: forward to mouse-based swipe/tap logic
    {
      int y_bl = game->screen ? (game->screen->h - 1 - iy) : iy;
      if (actionMasked == 0) {
        mouseGame(0 /* left button */, 0 /* down */, ix, y_bl);
      } else {
        motionGame(ix, y_bl);
      }
    }
  } else if (actionMasked == 1 /* ACTION_UP */) {
    // End overlay states
    int y_bl = game->screen ? (game->screen->h - 1 - iy) : iy;
    active_left = active_right = active_pause = 0;
    // Forward to mouse-based tap/swipe finish
    mouseGame(0 /* left button */, 1 /* up */, ix, y_bl);
  }
  // Otherwise no overlay hit; ignore or add game-world touch handling if desired
}

#endif // ANDROID
