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

  /* Tunables to better match Android text rendering */
  const float ANDROID_GUI_HIT_Y_OFFSET = 0.10f;         /* move hit rect slightly upward */
  const float ANDROID_GUI_HIT_HEIGHT_SCALE = 1.25f;     /* enlarge height a bit */
  const float ANDROID_GUI_HIT_WIDTH_PER_CHAR = 0.70f;   /* width per character */
  const int   ANDROID_GUI_HIT_XPAD = 3;                 /* extra size units as padding */

  int x_start = vw / 6; /* text x in viewport-local pixels */
  int size = vw / 32;
  int lineheight = size * 2;
  int y_start = 2 * vh / 3; /* top line baseline (viewport-local) */

  if (lineheight <= 0) return -1;

  /* Convert window coords to viewport-local coords with origin at bottom-left (viewport-local) */
  int x_local = x_win - vx;
  int y_local = vh - (y_win - vy); /* use viewport height consistently */

  /* Back button hit-test FIRST to avoid overlap with items */
  {
    const char* back = "Back";
    int len = (int)strlen(back);
    int bx = vw - (int)(size * 4.5f);   /* move a bit further from edge */
    int by = (int)(size * 1.2f);
    int rect_x0 = bx - (int)(0.5f * size); /* expand left padding */
    int rect_y0 = by - (int)(0.5f * size); /* expand vertically */
    int rect_w = (int)(size * ANDROID_GUI_HIT_WIDTH_PER_CHAR * len) + size * (ANDROID_GUI_HIT_XPAD + 1);
    int rect_h = (int)(size * 2.0f * ANDROID_GUI_HIT_HEIGHT_SCALE);
    if (x_local >= rect_x0 && x_local <= rect_x0 + rect_w &&
        y_local >= rect_y0 && y_local <= rect_y0 + rect_h) {
      return -2; /* special code for back */
    }
  }

  /* Determine which line rectangle contains the point */
  for (int i = 0; i < pCurrent->nEntries; i++) {
    int item_y_base = y_start - i * lineheight;
    int rect_x0 = x_start;
    int rect_y0 = item_y_base - (int)((1.3f + ANDROID_GUI_HIT_Y_OFFSET) * size);

    /* Estimate text width with Android-tuned factor */
    const char* txt = ((Menu*)*(pCurrent->pEntries + i))->display.szCaption;
    int len = (int)strlen(txt);
    int rect_w = (int)(size * ANDROID_GUI_HIT_WIDTH_PER_CHAR * len) + size * ANDROID_GUI_HIT_XPAD;
    int rect_h = (int)(lineheight * ANDROID_GUI_HIT_HEIGHT_SCALE);

    if (x_local >= rect_x0 && x_local <= rect_x0 + rect_w &&
        y_local >= rect_y0 && y_local <= rect_y0 + rect_h) {
      return i;
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

      const char* cur = pCurrent ? pCurrent->szName : "(null)";
      const char* par = (pCurrent && pCurrent->parent) ? pCurrent->parent->szName : "(null)";

      requestDisplayApply();
      if (pCurrent == NULL || pCurrent->parent == NULL) {
#ifdef ANDROID
        __android_log_print(ANDROID_LOG_INFO, "gltron", "mouseGui: BACK at top-level -> restoreCallbacks + request finish");
#endif
        restoreCallbacks();
        /* Mark for finish in android_main via global flag */
        extern int g_finish_requested; g_finish_requested = 1;
      } else {
        pCurrent = pCurrent->parent;
        pCurrent->iHighlight = -1;
        /* Ensure GUI callbacks are active */
        switchCallbacks(&guiCallbacks);
        /* Force immediate display apply to avoid stale screen */
        requestDisplayApply();
        applyDisplaySettingsDeferred();
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
