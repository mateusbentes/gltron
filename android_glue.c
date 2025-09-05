#include "android_glue.h"
#include "gltron.h"
#include "sound_backend.h"
#include "shaders.h"
#include "globals.h"
#include "gltron.h"
#include "switchCallbacks.h"
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef ANDROID
#include <android/log.h>
#include <android/asset_manager.h>
#include <sys/stat.h>
#endif

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

// Forward declarations for GUI functions implemented in gui.c
void displayGui(void);
void initGui(void);
void initGLGui(void);
void idleGui(void);

// Forward declarations for game functions
void displayGame(void);
void idleGame(void);
void keyGame(unsigned char, int, int);
void specialGame(int, int, int);
void initGame(void);
void initGLGame(void);

// External callback structures defined in other files
extern callbacks guiCallbacks;
extern callbacks gameCallbacks;
extern callbacks pauseCallbacks;

// Mouse/motion handlers
void mouseGame(int button, int state, int x, int y);
void motionGame(int x, int y);
void motionGui(int x, int y);

// Display and menu functions
void requestDisplayApply(void);
void applyDisplaySettingsDeferred(void);
void forceViewportResetIfNeededForGame(void);
void menuAction(Menu *pMenu);

android_callbacks current_android_callbacks = {0};
android_callbacks last_android_callbacks = {0};
struct android_app* state;

struct android_app* state = NULL;

int g_finish_requested = 0;
int did_first_render = 0;

int gui_callbacks_initialized = 0;
int game_callbacks_initialized = 0;
int pause_callbacks_initialized = 0;

// Reset game callbacks initialization when starting a new game
void reset_game_callbacks_init() {
  game_callbacks_initialized = 0;
  pause_callbacks_initialized = 0;
}

extern int g_finish_requested;

extern int scr_w, scr_h;
void update_buttons_layout();
extern callbacks *current_callback;

// base path buffer exposed for file.c
#ifndef PATH_MAX
#define PATH_MAX 1024
#endif
char s_base_path[PATH_MAX] = "/data/data/org.gltron.game/files"; // default fallback

int initialized = 0;
// base path defined above (non-static) and declared in header

#ifdef ANDROID

// Allow host (NativeActivity) to pass in the AAssetManager
void gltron_set_asset_manager(void* mgr) {
  g_android_asset_mgr = (AAssetManager*)mgr;
}

void gltron_set_base_path(const char* base_path) {
  if (!base_path) {
    return;
  }
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
    exit(1);
  }
}

/* Local helper: detect BACK hit in GUI using same viewport math as gui_mouse.c */
int hit_test_back_in_viewport(int x_win, int y_win) {
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
int hit_test_menu_item_in_viewport(int x_win, int y_win) {
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

// Function to validate callback pointers
static int validate_callbacks(const android_callbacks* cb) {
  if (!cb) {
    __android_log_print(ANDROID_LOG_ERROR, "gltron", "validate_callbacks: NULL callback structure");
    return 0;
  }
  
  // Check for obviously corrupted pointers
  if (cb->display && cb->display == (void*)0xdeadbeef) {
    __android_log_print(ANDROID_LOG_ERROR, "gltron", "validate_callbacks: corrupted display pointer");
    return 0;
  }
  if (cb->idle && cb->idle == (void*)0xdeadbeef) {
    __android_log_print(ANDROID_LOG_ERROR, "gltron", "validate_callbacks: corrupted idle pointer");
    return 0;
  }
  if (cb->init && cb->init == (void*)0xdeadbeef) {
    __android_log_print(ANDROID_LOG_ERROR, "gltron", "validate_callbacks: corrupted init pointer");
    return 0;
  }
  if (cb->initGL && cb->initGL == (void*)0xdeadbeef) {
    __android_log_print(ANDROID_LOG_ERROR, "gltron", "validate_callbacks: corrupted initGL pointer");
    return 0;
  }
  
  return 1; // callbacks are valid
}

// Initialize safe callbacks structure
static void init_safe_callbacks(android_callbacks* cb, 
                               void (*display)(void), 
                               void (*idle)(void),
                               void (*keyboard)(unsigned char, int, int),
                               void (*special)(int, int, int),
                               void (*init)(void),
                               void (*initGL)(void)) {
  if (!cb) return;
  
  memset(cb, 0, sizeof(android_callbacks));
  cb->display = display;
  cb->idle = idle;
  cb->keyboard = keyboard;
  cb->special = special;
  cb->init = init;
  cb->initGL = initGL;
}

// Android-specific callback switching implementations
void android_switchCallbacks(callbacks *new) {
  if (!new) {
    __android_log_print(ANDROID_LOG_ERROR, "gltron", "android_switchCallbacks: NULL callback passed");
    return;
  }
  
  extern callbacks guiCallbacks;
  extern callbacks gameCallbacks;
  extern callbacks pauseCallbacks;
  
  // Log which callback we're switching to
  const char* callback_name = "unknown";
  if (new == &guiCallbacks) callback_name = "guiCallbacks";
  else if (new == &gameCallbacks) callback_name = "gameCallbacks";
  else if (new == &pauseCallbacks) callback_name = "pauseCallbacks";
  
  __android_log_print(ANDROID_LOG_INFO, "gltron", "android_switchCallbacks: switching to %s (%p)", callback_name, new);
  
  // Don't switch if already on the same callback
  if (new == current_callback) {
    __android_log_print(ANDROID_LOG_INFO, "gltron", "android_switchCallbacks: already on %s, skipping", callback_name);
    return;
  }

  // Update current callback structure
  current_android_callbacks.display = new->display;
  current_android_callbacks.idle = new->idle;
  current_android_callbacks.keyboard = new->keyboard;
  current_android_callbacks.special = new->special;
  current_android_callbacks.init = new->init;
  current_android_callbacks.initGL = new->initGL;

  // Update the global pointer for compatibility
  extern callbacks *last_callback;
  extern callbacks *current_callback;
  last_callback = current_callback;
  current_callback = new;

  // Get elapsed time for timing
  extern int getElapsedTime(void);
  extern int lasttime;
  lasttime = getElapsedTime();

  // Determine if we need to initialize based on which callback set this is
  int needs_init = 0;
  int needs_initGL = 0;
  
  if (new == &guiCallbacks) {
    if (!gui_callbacks_initialized) {
      needs_init = 1;
      needs_initGL = 1;
      gui_callbacks_initialized = 1;
    }
  } else if (new == &gameCallbacks) {
    if (!game_callbacks_initialized) {
      needs_init = 1;
      needs_initGL = 1;
      game_callbacks_initialized = 1;
      // Ensure display is set up for game
      if (game && game->screen) {
        __android_log_print(ANDROID_LOG_INFO, "gltron", "Setting up display for game");
        setupDisplay(game->screen);
        // Force viewport update for game
        game->screen->vp_x = 0;
        game->screen->vp_y = 0;
        game->screen->vp_w = game->screen->w;
        game->screen->vp_h = game->screen->h;
      }
      // Reset timing to avoid huge jumps
      extern int lasttime;
      lasttime = getElapsedTime();
    }
  } else if (new == &pauseCallbacks) {
    if (!pause_callbacks_initialized) {
      needs_init = 1;
      needs_initGL = 1;
      pause_callbacks_initialized = 1;
    }
  } else {
    // Unknown callback set, initialize it anyway
    needs_init = 1;
    needs_initGL = 1;
  }

  // Initialize the new callbacks only if needed
  if (needs_init && new->init && new->init != (void*)0xdeadbeef) {
    __android_log_print(ANDROID_LOG_INFO, "gltron", "Calling init() for %s", callback_name);
    (new->init)();
  }

  if (needs_initGL && new->initGL && new->initGL != (void*)0xdeadbeef) {
    // Clear any pending GL errors before calling initGL
    while (glGetError() != GL_NO_ERROR) {
      // Clear error queue
    }
    
    __android_log_print(ANDROID_LOG_INFO, "gltron", "Calling initGL() for %s", callback_name);
    // Call initGL directly without recursive fallback
    (new->initGL)();
    
    // Check for errors after initGL
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
      __android_log_print(ANDROID_LOG_WARN, "gltron", "GL error after initGL: 0x%x (non-fatal)", error);
      // Don't fallback recursively, just log the error
    }
  }

  __android_log_print(ANDROID_LOG_INFO, "gltron", "Callback switching completed successfully");
  __android_log_print(ANDROID_LOG_INFO, "gltron", "Current callbacks: display=%p, idle=%p, keyboard=%p, special=%p, init=%p, initGL=%p",
    current_android_callbacks.display, current_android_callbacks.idle, current_android_callbacks.keyboard,
    current_android_callbacks.special, current_android_callbacks.init, current_android_callbacks.initGL);
}

void android_updateCallbacks(void) {
  // Called when the window is recreated on Android
  // Just update the timing
  extern int getElapsedTime(void);
  extern int lasttime;
  lasttime = getElapsedTime();
}

void android_restoreCallbacks(void) {
  extern callbacks *last_callback;
  extern callbacks guiCallbacks;

  if (last_callback == 0 || last_callback == (void*)0xdeadbeef) {
    // Fallback to GUI instead of crashing
  android_switchCallbacks(&guiCallbacks);
    return;
  }

  if (!validate_callbacks(&last_android_callbacks)) {
    // Fallback to GUI
  android_switchCallbacks(&guiCallbacks);
    return;
  }

  // Restore the callbacks from last_android_callbacks
  current_android_callbacks = last_android_callbacks;

  // Update the global pointer for compatibility
  extern callbacks *current_callback;
  current_callback = last_callback;

  // Get elapsed time for timing
  extern int getElapsedTime(void);
  extern int lasttime;
  lasttime = getElapsedTime();

  __android_log_print(ANDROID_LOG_INFO, "gltron", "Callback restoration completed successfully");
}

void gltron_init(void) {
  if (initialized) {
    return;
  }

  init_settings_android();
  
  // Initialize game structures BEFORE setting up anything else
  initGameStructures();
  resetScores();
  
  // Ensure touch input is enabled on Android
  if (game && game->settings) {
    game->settings->input_mode = 1; // enable touch/mouse input
    // Also ensure game starts unpaused
    game->settings->fast_finish = 1; // Enable fast finish for better Android experience
  }
  
  // Load menu.txt similar to desktop gltron.c
  char *path = getFullPath("menu.txt");
  if (path) {
    pMenuList = loadMenuFile(path);
    free(path);
  } else {
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
            }
          }
          free(buf);
        }
        AAsset_close(asset);
      } else {
      }
    }
    if (!pMenuList) {
    }
  }
  
  // Diagnostics and fallback menu on Android
  if (pMenuList && pMenuList[0]) {
    // Ensure current menu pointer is set
    pCurrent = pMenuList[0];
  } else {
    // Create a minimal menu hierarchy with 1 top-level menu and a few entries
    Menu* root = (Menu*)calloc(1, sizeof(Menu));
    strcpy(root->szName, "mRoot");
    strcpy(root->szCapFormat, "GLTron");
    root->nEntries = 2;
    root->pEntries = (Menu**)calloc(root->nEntries, sizeof(Menu*));
    for (int i=0; i<root->nEntries; ++i) root->pEntries[i] = (Menu*)calloc(1, sizeof(Menu));
    strcpy(root->pEntries[0]->szName, "xreset");  // Use correct menu code for starting game
    strcpy(root->pEntries[0]->szCapFormat, "Start Game");
    strcpy(root->pEntries[1]->szName, "pq");
    strcpy(root->pEntries[1]->szCapFormat, "Quit");
    pMenuList = (Menu**)calloc(1, sizeof(Menu*));
    pMenuList[0] = root;
    pCurrent = root;
  }

  // Already initialized above, just call initData
  initData();
  
  // Start the game unpaused on Android
  if (game) {
    game->pauseflag = 0;
  }

  // Initialize sound backend and try to load/play music on Android
#ifdef ANDROID
  if (sb_init()) {
    if (!sb_load_music("gltron.it")) {
    } else {
      sb_play_music();
    }
  } else {
  }
#endif

  // Initialize shaders and font system early on Android
  init_shaders_android();
  GLenum error = glGetError();
  if (error != GL_NO_ERROR) {
    __android_log_print(ANDROID_LOG_WARN, "gltron", "GL error after shader init: 0x%x", error);
  }
  
  // Verify shader was created successfully
  GLuint prog = shader_get_basic();
  if (!prog) {
    __android_log_print(ANDROID_LOG_ERROR, "gltron", "Failed to create shader program!");
  } else {
    __android_log_print(ANDROID_LOG_INFO, "gltron", "Shader program created successfully: %u", prog);
  }
  
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
  
  // Set up initial Android callbacks safely
  extern callbacks guiCallbacks;
  android_switchCallbacks(&guiCallbacks);
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

int hit_btn(int x, int y, int* btn) {
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

int is_gui_active() {
  extern callbacks guiCallbacks; // declared in other compilation units
  return current_callback == &guiCallbacks;
}

void draw_android_overlay() {
  if (!scr_w || !scr_h) return;
  
#ifdef DEBUG_OVERLAY
  // Always draw a small debug banner at top to prove overlay works
  // Make it more visible to help diagnose black screen issues
  draw_rect(8, 8, scr_w/3, scr_h/20, 1.f, 1.f, 1.f, 0.25f);
  
  // Draw a small indicator in each corner to help diagnose rendering issues
  // This will be visible even if the main game rendering fails
  draw_rect(0, 0, 20, 20, 1.0f, 0.0f, 0.0f, 0.5f); // Top-left
  draw_rect(scr_w-20, 0, 20, 20, 0.0f, 1.0f, 0.0f, 0.5f); // Top-right
  draw_rect(0, scr_h-20, 20, 20, 0.0f, 0.0f, 1.0f, 0.5f); // Bottom-left
  draw_rect(scr_w-20, scr_h-20, 20, 20, 1.0f, 1.0f, 0.0f, 0.5f); // Bottom-right
#endif

  if (is_gui_active()) return;
  
  // Make controls more visible to help with black screen issues
  draw_rect(btn_left[0], btn_left[1], btn_left[2], btn_left[3], 1,1,1,0.45f);
  draw_rect(btn_right[0], btn_right[1], btn_right[2], btn_right[3], 1,1,1,0.45f);
  draw_rect(btn_pause[0], btn_pause[1], btn_pause[2], btn_pause[3], 1,1,1,0.40f);
}

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
  
  // If paused, unpause on touch-up
  if (game != NULL && game->pauseflag != 0) {
    if (action == 1 /* ACTION_UP */) {
      __android_log_print(ANDROID_LOG_INFO, "gltron", "Unpausing game on touch-up");
      
      // Check if game is in a valid state to unpause
      if (game->running <= 0) {
        __android_log_print(ANDROID_LOG_INFO, "gltron", "Game is finished, restarting");
        // If game is finished, restart it
        initData();
        game->pauseflag = 0;
      }
      
      extern callbacks gameCallbacks;
      // Reset timing to avoid huge time jumps
      extern int lasttime;
      lasttime = getElapsedTime();
      
      // Safely switch callbacks
      android_updateCallbacks();
      android_switchCallbacks(&gameCallbacks);
      
      // Apply display settings after switching to game callbacks
      requestDisplayApply();
      applyDisplaySettingsDeferred();
      
      // Clear pause flag
      game->pauseflag = 0;
      
      // Ensure viewport/proj are correct on first frame
      forceViewportResetIfNeededForGame();
      return;
    }
  }

  // If GUI (menu) is active, route all touches to menu handlers
  if (is_gui_active()) {
#ifdef ANDROID
    if (action == 0) __android_log_print(ANDROID_LOG_INFO, "gltron", "touch->GUI: down %d,%d", ix, iy);
    if (action == 2) __android_log_print(ANDROID_LOG_INFO, "gltron", "touch->GUI: move %d,%d", ix, iy);
    if (action == 1) __android_log_print(ANDROID_LOG_INFO, "gltron", "touch->GUI: up %d,%d (click)", ix, iy);
#endif
    // Tap tolerance for BACK: remember DOWN position and whether it started on BACK
    static int s_gui_down_on_back = 0; static int s_down_x = 0, s_down_y = 0; const int tol = 24;
    
    // Handle different touch actions
    if (action == 0) { // ACTION_DOWN
      // Store initial touch position
      s_down_x = ix; 
      s_down_y = iy;
      // Check if touch started on BACK button
      s_gui_down_on_back = hit_test_back_in_viewport(ix, iy);
      // Forward to GUI motion handler
      motionGui(ix, iy);
    } else if (action == 2) { // ACTION_MOVE
      // Forward to GUI motion handler
      motionGui(ix, iy);
    } else if (action == 1) { // ACTION_UP
      // Check if we had a tap on the BACK button
      if (s_gui_down_on_back) {
        // Calculate distance moved from initial touch
        int dx = ix - s_down_x; if (dx < 0) dx = -dx;
        int dy = iy - s_down_y; if (dy < 0) dy = -dy;
        
        // If within tolerance, consider it a tap on BACK
        if (dx <= tol && dy <= tol) {
#ifdef ANDROID
          __android_log_print(ANDROID_LOG_INFO, "gltron", "touch->GUI: BACK via tap-tolerance");
#endif
          // Apply any pending display changes
          requestDisplayApply();
          
          // Check if we're at the root menu
          if (!pCurrent || pCurrent->parent == NULL) {
            // Special case: if game is finished, restart instead of exiting
            if (game && (game->pauseflag & PAUSE_GAME_FINISHED)) {
              __android_log_print(ANDROID_LOG_INFO, "gltron", "Game finished, restarting on BACK");
              
              // Reset game state
              initData();
              game->pauseflag = 0;
              
              // Switch to game callbacks
              extern callbacks gameCallbacks;
              extern int lasttime;
              lasttime = getElapsedTime();
              android_switchCallbacks(&gameCallbacks);
              
              // Reset back button state
              s_gui_down_on_back = 0;
              return;
            }

            // At root menu, request app exit
            __android_log_print(ANDROID_LOG_INFO, "gltron", "At root menu, requesting exit");
            android_restoreCallbacks();
            g_finish_requested = 1;
          } else {
            // Go up one level in menu hierarchy
            __android_log_print(ANDROID_LOG_INFO, "gltron", "Going up one level in menu");
            if (pCurrent && pCurrent->parent) {
              pCurrent = pCurrent->parent;
              pCurrent->iHighlight = -1;
              
              // Switch to GUI callbacks
              extern callbacks guiCallbacks;
              android_switchCallbacks(&guiCallbacks);
              
              // Apply display settings
              requestDisplayApply();
              applyDisplaySettingsDeferred();
            }
          }
          
          // Reset back button state
          s_gui_down_on_back = 0;
          return;
        }
      }
      
      // If not a BACK tap, check for menu item taps
      {
        // Perform hit test at release position
        int idx = hit_test_menu_item_in_viewport(ix, iy);
        
        if (idx == -2) {
          // Hit on BACK button
          __android_log_print(ANDROID_LOG_INFO, "gltron", "Menu: BACK button hit");
          
          // Apply any pending display changes
          requestDisplayApply();
          
          // Check if we're at the root menu
          if (!pCurrent || pCurrent->parent == NULL) {
            // At root menu, request app exit
            android_restoreCallbacks();
            g_finish_requested = 1;
          } else {
            // Go up one level in menu hierarchy
            if (pCurrent && pCurrent->parent) {
              pCurrent = pCurrent->parent;
              pCurrent->iHighlight = -1;
              
              // Switch to GUI callbacks
              extern callbacks guiCallbacks;
              android_switchCallbacks(&guiCallbacks);
              
              // Apply display settings
              requestDisplayApply();
              applyDisplaySettingsDeferred();
            }
          }
        } else if (idx >= 0 && pCurrent && idx < pCurrent->nEntries) {
          // Hit on a menu item
          __android_log_print(ANDROID_LOG_INFO, "gltron", "Menu: item %d selected", idx);
          
          // Highlight the selected item
          pCurrent->iHighlight = idx;
          
          // Execute the menu action
          // If selecting 'Start Game' from menu, don't change pause flag here
          // Let the game start in pause mode as intended
          if (pCurrent->pEntries && *(pCurrent->pEntries + pCurrent->iHighlight)) {
            menuAction(*(pCurrent->pEntries + pCurrent->iHighlight));
          }
        }
      }
      
      // Reset back button state
      s_gui_down_on_back = 0;
    }
    return;
  }

  // Manage active button states for in-game overlay
  if (action == 0 /* ACTION_DOWN */ || action == 2 /* ACTION_MOVE */) {
    // Store previous button states
    int was_left = active_left;
    int was_right = active_right;
    
    // Check which buttons are being touched
    active_left = hit_btn(ix, iy, btn_left);
    active_right = hit_btn(ix, iy, btn_right);
    active_pause = hit_btn(ix, iy, btn_pause);
    
    // Handle pause button press
    if (active_pause && action == 0) {
      __android_log_print(ANDROID_LOG_INFO, "gltron", "Pause button pressed");
      
      // Check game state before pausing
      if (game && game->pauseflag == 0) {
        // Only pause if game is running
        keyGame(' ', 0, 0);
      }
      return;
    }
    
    // Only send turn commands on initial press, not continuous
    if (active_left && !was_left) {
      __android_log_print(ANDROID_LOG_INFO, "gltron", "Left button pressed");
      
      // Check if game is in a valid state for turning
      if (game && game->pauseflag == 0) {
        // Check if player is still alive before processing turn
        if (game->player[0].data && game->player[0].data->speed > 0) {
          // Use a try-catch approach to prevent crashes
          extern int lasttime;
          int old_time = lasttime;
          
          // Update timing before turn to avoid huge time jumps
          lasttime = getElapsedTime();
          
          // Process the turn
          keyGame('a', 0, 0);
          
          // If something went wrong, restore the old time
          if (lasttime - old_time > 1000) {
            lasttime = old_time + 20; // Add a small increment
          }
        }
      }
      return;
    }
    
    if (active_right && !was_right) {
      __android_log_print(ANDROID_LOG_INFO, "gltron", "Right button pressed");
      
      // Check if game is in a valid state for turning
      if (game && game->pauseflag == 0) {
        // Check if player is still alive before processing turn
        if (game->player[0].data && game->player[0].data->speed > 0) {
          // Use a try-catch approach to prevent crashes
          extern int lasttime;
          int old_time = lasttime;
          
          // Update timing before turn to avoid huge time jumps
          lasttime = getElapsedTime();
          
          // Process the turn
          keyGame('s', 0, 0);
          
          // If something went wrong, restore the old time
          if (lasttime - old_time > 1000) {
            lasttime = old_time + 20; // Add a small increment
          }
        }
      }
      return;
    }
    
    // Not over overlay buttons: forward to mouse-based swipe/tap logic
    if (game && game->screen) {
      int y_bl = (game->screen->h - 1 - iy);
      
      // Only forward if game is in a valid state
      if (game->pauseflag == 0) {
        if (action == 0) {
          mouseGame(0 /* left button */, 0 /* down */, ix, y_bl);
        } else {
          motionGame(ix, y_bl);
        }
      }
    }
  } else if (action == 1 /* ACTION_UP */) {
    // End overlay states
    active_left = active_right = active_pause = 0;
    
    // Forward to mouse-based tap/swipe finish only if game is in a valid state
    if (game != NULL && game->screen != NULL && game->pauseflag == 0) {
      int y_bl = (game->screen->h - 1 - iy);
      mouseGame(0 /* left button */, 1 /* up */, ix, y_bl);
    }
  }
  // Otherwise no overlay hit; ignore or add game-world touch handling if desired
}

#endif // ANDROID
