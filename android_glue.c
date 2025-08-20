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
  // Let the game react to display changes
  requestDisplayApply();
  applyDisplaySettingsDeferred();
}

void gltron_frame(void) {
  if (!initialized) gltron_init();
  // Simulate the usual idle/display cycle
  // Update game state
  idleGame();
  // Render
  displayGame();
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
  (void)action;
  // Map touch to GUI interactions if GUI is active
  // Basic example: update highlight and trigger action on tap
  int ix = (int)x;
  int iy = (int)y;
  motionGui(ix, iy);
  mouseGui(0, 0, ix, iy);
}

#endif // ANDROID
