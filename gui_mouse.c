#include "gltron.h"
#include "gui_mouse.h"
#include <string.h>
#ifdef ANDROID
// On Android we don't have GLUT; guard calls and constants
#define GLUT_LEFT_BUTTON 0
#define GLUT_UP 1
#endif

/* Helper to map window coords to menu item index */
static int gui_hit_test(int x_win, int y_win) {
  /* Mirror layout from drawMenu */
  int vx = game->screen->vp_x;
  int vy = game->screen->vp_y;
  int vw = game->screen->vp_w;
  int vh = game->screen->vp_h;

  int x_start = vw / 6; /* text x in viewport-local pixels */
  int size = vw / 32;
  int lineheight = size * 2;
  int y_start = 2 * vh / 3; /* top line baseline (viewport-local) */

  if (lineheight <= 0) return -1;

  /* Convert window coords to viewport-local coords with origin at bottom-left (viewport-local) */
  int x_local = x_win - vx;
  int y_local = vh - (y_win - vy); /* use viewport height consistently */

  /* Determine which line rectangle contains the point */
  for (int i = 0; i < pCurrent->nEntries; i++) {
    int item_y_base = y_start - i * lineheight;
    int rect_x0 = x_start;
    int rect_y0 = item_y_base - (int)(1.3f * size); /* a bit taller than font */

    /* Estimate text width: roughly 0.6 * size pixels per character */
    const char* txt = ((Menu*)*(pCurrent->pEntries + i))->display.szCaption;
    int len = (int)strlen(txt);
    int rect_w = (int)(size * 0.65f * len) + size * 3; /* more forgiving width */
    int rect_h = (int)(lineheight * 1.1f); /* slightly larger hit height */

    if (x_local >= rect_x0 && x_local <= rect_x0 + rect_w &&
        y_local >= rect_y0 && y_local <= rect_y0 + rect_h) {
      return i;
    }
  }
  /* Back button hit-test: bottom-right */
  {
    const char* back = "Back";
    int len = (int)strlen(back);
    int bx = vw - (int)(size * 4);
    int by = (int)(size * 1.2f);
    int rect_x0 = bx;
    int rect_y0 = by - (int)(0.2f * size);
    int rect_w = (int)(size * 0.6f * len) + size; /* small padding */
    int rect_h = (int)(1.6f * size);
    if (x_local >= rect_x0 && x_local <= rect_x0 + rect_w &&
        y_local >= rect_y0 && y_local <= rect_y0 + rect_h) {
      return -2; /* special code for back */
    }
  }
  return -1;
}

void motionGui(int x, int y) {
  if (game->settings->input_mode == 0) return; /* keyboard only */
  int idx = gui_hit_test(x, y);
  if (idx >= 0) {
    pCurrent->iHighlight = idx;
#ifndef ANDROID
    glutPostRedisplay();
#endif
  }
}

void mouseGui(int button, int state, int x, int y) {
  if (game->settings->input_mode == 0) return; /* keyboard only */
  if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
    int idx = gui_hit_test(x, y);
    if (idx == -2) {
      /* Back button */
      /* On Back, request applying any pending display changes */
      requestDisplayApply();
      if (pCurrent->parent == NULL) {
        restoreCallbacks();
      } else {
        pCurrent = pCurrent->parent;
      }
#ifndef ANDROID
      glutPostRedisplay();
#endif
    } else if (idx >= 0) {
      pCurrent->iHighlight = idx;
      menuAction(*(pCurrent->pEntries + pCurrent->iHighlight));
    }
  }
}
