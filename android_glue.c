#include "android_glue.h"
#include "globals.h"

#ifdef ANDROID

static int initialized = 0;
static char s_base_path[256] = "/data/data/gltron/files"; // default fallback

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
  // Initialize core game structures and data
  initMainGameSettings("settings.txt");
  initGameStructures();
  resetScores();
  initData();
  initialized = 1;
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
static int scr_w = 0, scr_h = 0;
static int btn_left[4];   // x, y, w, h
static int btn_right[4];  // x, y, w, h
static int btn_pause[4];  // x, y, w, h
static int active_left = 0;
static int active_right = 0;
static int active_pause = 0;

static void update_buttons_layout() {
  if (!scr_w || !scr_h) return;
  int bw = scr_w * 0.2f;
  int bh = scr_h * 0.18f;
  // Left: bottom-left
  btn_left[0] = (int)(scr_w * 0.05f);
  btn_left[1] = (int)(scr_h * 0.05f);
  btn_left[2] = bw;
  btn_left[3] = bh;
  // Right: bottom-right
  btn_right[2] = bw;
  btn_right[3] = bh;
  btn_right[0] = scr_w - btn_right[2] - (int)(scr_w * 0.05f);
  btn_right[1] = (int)(scr_h * 0.05f);
  // Pause: top-right small square
  int pw = scr_w * 0.10f;
  int ph = pw;
  btn_pause[2] = pw;
  btn_pause[3] = ph;
  btn_pause[0] = scr_w - pw - (int)(scr_w * 0.03f);
  btn_pause[1] = scr_h - ph - (int)(scr_h * 0.03f);
}

static int hit_btn(int x, int y, int* btn) {
  return (x >= btn[0] && x <= btn[0]+btn[2] && y >= btn[1] && y <= btn[1]+btn[3]);
}

static void draw_rect(int x, int y, int w, int h, float r, float g, float b, float a) {
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_TEXTURE_2D);
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrthof(0, scr_w, 0, scr_h, -1, 1);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  glColor4f(r,g,b,a);
  glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x+w, y);
    glVertex2f(x+w, y+h);
    glVertex2f(x, y+h);
  glEnd();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
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
  // Simulate the usual idle/display cycle
  idleGame();
  // Render
  displayGame();
  // Overlay controls
  draw_android_overlay();
}

void gltron_on_key(int key, int action) {
  (void)action; // not used currently
  // Forward to existing keyboard/special handlers
  // For simplicity route arrows as special keys, others as ASCII
  switch (key) {
    case 27: // ESC
    case ' ':
    case 'a': case 'A': case 'd': case 'D': case 's': case 'S':
    case 'k': case 'K': case 'l': case 'L': case '5': case '6':
      keyGame((unsigned char)key, 0, 0);
      break;
    case GLUT_KEY_LEFT:
    case GLUT_KEY_RIGHT:
    case GLUT_KEY_UP:
    case GLUT_KEY_DOWN:
      specialGame(key, 0, 0);
      break;
    default:
      keyGame((unsigned char)key, 0, 0);
      break;
  }
}

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
