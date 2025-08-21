#include "gltron.h"
#include <math.h>
#ifdef ANDROID
// Define GLUT mouse constants for Android path
#define GLUT_LEFT_BUTTON 0
#define GLUT_DOWN 0
#define GLUT_UP 1
#endif

static int down_x = -1, down_y = -1;
static int last_x = -1, last_y = -1;
static int is_down = 0;
static long down_t = 0;

static void handle_swipe(int dx, int dy) {
  /* Only horizontal swipes control steering */
  int vw = game->screen->vp_w;
  float min_dist = vw * 0.05f; /* 5% of width */
  if (abs(dx) < min_dist) return;
  if (abs(dx) < abs(dy)) return; /* require mostly horizontal */

  if (dx > 0) {
    /* right */
    turn(game->player[0].data, 1);
  } else {
    /* left */
    turn(game->player[0].data, 3);
  }
}

static int is_tap(int up_x, int up_y, long up_t) {
  int vw = game->screen->vp_w;
  float max_move = vw * 0.02f; /* 2% of width */
  long dt_ms = up_t - down_t;
  int mdx = abs(up_x - down_x);
  int mdy = abs(up_y - down_y);
  if (mdx <= max_move && mdy <= max_move && dt_ms <= 300) return 1;
  return 0;
}

void motionGame(int x, int y) {
  if (game->settings->input_mode == 0) return;
  if (!is_down) return;
  last_x = x; last_y = y;
}

void mouseGame(int button, int state, int x, int y) {
  if (game->settings->input_mode == 0) return; /* keyboard only */

  if (button == GLUT_LEFT_BUTTON) {
    if (state == GLUT_DOWN) {
      is_down = 1;
      down_x = last_x = x;
      down_y = last_y = y;
      down_t = getElapsedTime();
    } else if (state == GLUT_UP) {
      if (is_down) {
        int dx = x - down_x;
        int dy = y - down_y;
        long up_t = getElapsedTime();
        if (is_tap(x, y, up_t)) {
          /* tap toggles pause */
          switchCallbacks(&pauseCallbacks);
        } else {
          handle_swipe(dx, dy);
        }
      }
      is_down = 0;
    }
  }
}
