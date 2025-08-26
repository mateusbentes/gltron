#include "android_glue.h"
#include "shaders.h"
#include "globals.h"
#include "gltron.h"
#include <string.h>
#ifdef ANDROID
#include <android/log.h>
#include <android/asset_manager.h>
#include <sys/stat.h>
#endif

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
#ifdef ANDROID
  if (!s_base_path[0]) return;
  char settingsPathBuf[PATH_MAX];
  snprintf(settingsPathBuf, sizeof(settingsPathBuf), "%s/%s", s_base_path, "settings.txt");
  // Ensure base directory exists
  mkdir(s_base_path, 0700);
  // If file does not exist, try to seed from asset; else create defaults
  FILE* f = fopen(settingsPathBuf, "rb");
  if (!f) {
    __android_log_print(ANDROID_LOG_INFO, "gltron", "settings.txt not found; seeding from assets or defaults at %s", settingsPathBuf);
    if (g_android_asset_mgr) {
      AAsset* asset = AAssetManager_open(g_android_asset_mgr, "settings.txt", AASSET_MODE_STREAMING);
      if (asset) {
        FILE* out = fopen(settingsPathBuf, "wb");
        if (out) {
          char buf[4096];
          int r;
          while ((r = AAsset_read(asset, buf, sizeof(buf))) > 0) fwrite(buf, 1, (size_t)r, out);
          fclose(out);
          __android_log_print(ANDROID_LOG_INFO, "gltron", "Seeded settings.txt from APK asset");
        }
        AAsset_close(asset);
      }
    }
    // If still missing, write minimal defaults
    f = fopen(settingsPathBuf, "rb");
    if (!f) {
      FILE* out = fopen(settingsPathBuf, "wb");
      if (out) {
        const char* defaults =
          "2\n"               /* number of sections */
          "i 28\n"           /* 28 integer setting names */
          "show_help\n"
          "show_fps\n"
          "show_wall\n"
          "show_glow\n"
          "show_2d\n"
          "show_alpha\n"
          "show_floor_texture\n"
          "line_spacing\n"
          "erase_crashed\n"
          "fast_finish\n"
          "fov\n"
          "width\n"
          "height\n"
          "show_ai_status\n"
          "camType\n"
          "display_type\n"
          "playSound\n"
          "show_model\n"
          "ai_player1\n"
          "ai_player2\n"
          "ai_player3\n"
          "ai_player4\n"
          "show_crash_texture\n"
          "turn_cycle\n"
          "mouse_warp\n"
          "sound_driver\n"
          "input_mode\n"
          "fullscreen\n"
          "f 1\n"           /* 1 float setting name */
          "speed\n";
        fwrite(defaults, 1, strlen(defaults), out);
        fclose(out);
        __android_log_print(ANDROID_LOG_INFO, "gltron", "Wrote default settings.txt (formatted)");
      }
    } else {
      fclose(f);
    }
    // Let settings module initialize from this path now
    initMainGameSettings(settingsPathBuf);
    return;
  } else {
    fclose(f);
    initMainGameSettings(settingsPathBuf);
    return;
  }
#endif
}

void gltron_init(void) {
  if (initialized) return;
#ifdef ANDROID
  __android_log_print(ANDROID_LOG_INFO, "gltron", "gltron_init: base_path='%s' mgr=%p", s_base_path, g_android_asset_mgr);
  init_settings_android();
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
#endif
  }
#ifdef ANDROID
  // Diagnostics and fallback menu on Android
  if (pMenuList && pMenuList[0]) {
    __android_log_print(ANDROID_LOG_INFO, "gltron", "menu loaded: first=%p name='%s' entries=%d",
      pMenuList[0], pMenuList[0]->szName, pMenuList[0]->nEntries);
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
#endif
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
