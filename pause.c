#include "gltron.h"

static int p_is_down = 0;
static int p_down_x = 0, p_down_y = 0;
static long p_down_t = 0;

static int p_is_tap(int up_x, int up_y, long up_t) {
  int vw = game->screen->vp_w;
  float max_move = vw * 0.02f; /* 2% of width */
  long dt_ms = up_t - p_down_t;
  int mdx = abs(up_x - p_down_x);
  int mdy = abs(up_y - p_down_y);
  return (mdx <= max_move && mdy <= max_move && dt_ms <= 300);
}

void motionPause(int x, int y) {
  (void)x; (void)y;
}

void mousePause(int button, int state, int x, int y) {
  if (game->settings->input_mode == 0) return; /* keyboard only */
#ifdef ANDROID
  if (button == 0) { // GLUT_LEFT_BUTTON
    if (state == 0) { // GLUT_DOWN
#else
  if (button == GLUT_LEFT_BUTTON) {
    if (state == GLUT_DOWN) {
#endif
      p_is_down = 1;
      p_down_x = x; p_down_y = y; p_down_t = getElapsedTime();
#ifdef ANDROID
    } else if (state == 1) { // GLUT_UP
#else
    } else if (state == GLUT_UP) {
#endif
      if (p_is_down) {
        long up_t = getElapsedTime();
        if (p_is_tap(x, y, up_t)) {
          if(game->pauseflag & PAUSE_GAME_FINISHED)
            initData();
          lasttime = getElapsedTime();
          switchCallbacks(&gameCallbacks);
        }
      }
      p_is_down = 0;
    }
  }
}

/* very brief - just the pause mode */

void idlePause() {
#ifdef SOUND
  soundIdle();
#endif
  if(getElapsedTime() - lasttime < 10) return;
  timediff();
#ifndef ANDROID
  glutPostRedisplay();
#endif
}

void displayPause() {
  drawGame();
  drawPause(game->screen);

  if(game->settings->mouse_warp)
    mouseWarp();
#ifndef ANDROID
  glutSwapBuffers();
#endif
}

void keyboardPause(unsigned char key, int x, int y) {
  switch(key) {
  case 27:
    switchCallbacks(&guiCallbacks);
    break;
  case ' ':
    if(game->pauseflag & PAUSE_GAME_FINISHED)
      initData();
    lasttime = getElapsedTime();
    switchCallbacks(&gameCallbacks);
    break;
  case 'q':
    exit(1);
    break;
  }
}

void specialPause(int key, int x, int y) {
  int i;

  switch(key) {
#ifdef ANDROID
  case 102: // GLUT_KEY_F1
#else
  case GLUT_KEY_F1:
#endif
    defaultDisplay(0);
    break;
#ifdef ANDROID
  case 103: // GLUT_KEY_F2
#else
  case GLUT_KEY_F2:
#endif
    defaultDisplay(1);
    break;
#ifdef ANDROID
  case 104: // GLUT_KEY_F3
#else
  case GLUT_KEY_F3:
#endif
    defaultDisplay(2);
    break;
#ifdef ANDROID
  case 109: // GLUT_KEY_F10
#else
  case GLUT_KEY_F10:
#endif
    game->settings->camType = (game->settings->camType + 1) % CAM_COUNT;
    for(i = 0; i < game->players; i++)
      game->player[i].camera->camType = game->settings->camType;
    break;
#ifdef ANDROID
  case 114: // GLUT_KEY_F5
#else
  case GLUT_KEY_F5:
#endif
    saveSettings();
    break;
  }
}

void initPause() {
}

void initPauseGL() {
  initGLGame();
}

callbacks pauseCallbacks = {
  displayPause, idlePause, keyboardPause, specialPause, initPause, initPauseGL
};
