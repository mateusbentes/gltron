#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "sgi_texture.h"
#include "gltron.h"
#include "gui_mouse.h"

#ifdef ANDROID
#include <GLES2/gl2.h>
#include "shaders.h"
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

sgi_texture *tex;

typedef struct {
float d;
float posx;
float posy;
long lt; 
} background_states;

background_states bgs;

void guiProjection(int x, int y) {
  checkGLError("gui.c guiProj - start");
#ifndef ANDROID
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  /*glOrtho(0, 0, x, y, -1, 1); */
  checkGLError("gui.c guiProj - proj");
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
#endif
  glViewport(0, 0, x, y);
  checkGLError("gui.c guiProj - end");
}

#define GUI_BLUE 0.3
void displayGui() {
  float x, y, w, h;
  float y1, y2;
  float a, b1, b2, c1, c2;
  float alpha;
#ifdef ANDROID
  static int logged = 0;
  if (!logged) {
    __android_log_print(ANDROID_LOG_INFO, "gltron", "displayGui: drawing GUI");
    logged = 1;
  }
#endif
  #define N 20.0
  checkGLError("gui.c displayGui - before clear");

  // Clear the screen
  glClearColor(0.0, 0.0, 1.0, 0.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

#ifdef ANDROID
  // Set up 2D projection and shader for Android
  rasonly(game->screen);
  GLuint shaderProgram = shader_get_basic();
  if (shaderProgram) {
    useShaderProgram(shaderProgram);
  }
#else
  // Set up 2D projection for non-Android
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
#endif

  // Draw background
  GLfloat vertices[8];
  GLfloat colors[12];

  // Background quad
  vertices[0] = -1.0f; vertices[1] = -1.0f;
  vertices[2] = 1.0f; vertices[3] = -1.0f;
  vertices[4] = 1.0f; vertices[5] = 1.0f;
  vertices[6] = -1.0f; vertices[7] = 1.0f;

  c1 = 0.25f; c2 = 0.75f;
  colors[0] = c1; colors[1] = c1; colors[2] = GUI_BLUE * c1;
  colors[3] = c2; colors[4] = c2; colors[5] = GUI_BLUE * c2;
  colors[6] = c2; colors[7] = c2; colors[8] = GUI_BLUE * c2;
  colors[9] = c1; colors[10] = c1; colors[11] = GUI_BLUE * c1;

  // Draw background
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_COLOR_ARRAY);
  glVertexPointer(2, GL_FLOAT, 0, vertices);
  glColorPointer(3, GL_FLOAT, 0, colors);
  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
  glDisableClientState(GL_COLOR_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);

  // Draw animated grid
  for(y1 = -1; y1 < 1; y1 += 2 / N) {
    y2 = y1 + 2 / N;
    for(x = -1; x < 1; x += 2 / N) {
      c1 = (x + 1) / 2;
      c2 = (x + 2 / N + 1) / 2;
      c1 = c1 / 2 + 0.25;
      c2 = c2 / 2 + 0.25;

      a = x + sin(bgs.d) / 10;
      b1 = x + 2 / N;
      b2 = b1 + cos(bgs.d) / 10;

      vertices[0] = a; vertices[1] = y1;
      vertices[2] = b1; vertices[3] = y1;
      vertices[4] = b2; vertices[5] = y2;
      vertices[6] = a; vertices[7] = y2;

      colors[0] = c1; colors[1] = c1; colors[2] = GUI_BLUE * c1;
      colors[3] = c2; colors[4] = c2; colors[5] = GUI_BLUE * c2;
      colors[6] = c2; colors[7] = c2; colors[8] = GUI_BLUE * c2;
      colors[9] = c1; colors[10] = c1; colors[11] = GUI_BLUE * c1;

      glEnableClientState(GL_VERTEX_ARRAY);
      glEnableClientState(GL_COLOR_ARRAY);
      glVertexPointer(2, GL_FLOAT, 0, vertices);
      glColorPointer(3, GL_FLOAT, 0, colors);
      glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
      glDisableClientState(GL_COLOR_ARRAY);
      glDisableClientState(GL_VERTEX_ARRAY);
    }
  }

  // Draw logo
  x = bgs.posx;
  y = bgs.posy;
  w = 1;
  h = w/4;
  alpha = (sin(bgs.d - M_PI / 2) + 1) / 2;

  vertices[0] = x - w / 2; vertices[1] = y - h / 2;
  vertices[2] = x + w / 2; vertices[3] = y - h / 2;
  vertices[4] = x + w / 2; vertices[5] = y + h / 2;
  vertices[6] = x - w / 2; vertices[7] = y + h / 2;

  GLfloat texCoords[8] = {
    0.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f
  };

#ifdef ANDROID
  // Set up shader for logo on Android
  if (shaderProgram) {
    setColor(shaderProgram, 1.0f, 1.0f, 0.0f, alpha);
    setTexture(shaderProgram, 0); // Assuming texture unit 0
  }
#else
  // Set up color for logo on non-Android
  glColor4f(1.0f, 1.0f, 0.0f, alpha);
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, game->screen->texGui);
#endif

  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glVertexPointer(2, GL_FLOAT, 0, vertices);
  glTexCoordPointer(2, GL_FLOAT, 0, texCoords);
  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);

#ifndef ANDROID
  glDisable(GL_TEXTURE_2D);
#endif

  // Draw menu
#ifdef ANDROID
  if (shaderProgram) {
    setColor(shaderProgram, 1.0f, 0.0f, 1.0f, 1.0f); // Set color for menu
  }
#else
  glColor3f(1.0f, 0.0f, 1.0f); // Set color for menu
#endif
  drawMenu(game->screen);

  if(game->settings->mouse_warp)
    mouseWarp();

#ifndef ANDROID
  glutSwapBuffers();
#endif
}

void idleGui() {
  float delta;
  long now;

#ifdef SOUND
  soundIdle();
#endif

  now = getElapsedTime();
  delta = now - bgs.lt;
  bgs.lt = now;
  delta /= 1000.0;
  bgs.d += delta;
  /* printf("%.5f\n", delta); */
  
  if(bgs.d > 2 * M_PI) { 
    bgs.d -= 2 * M_PI;
    bgs.posx = 1.0 * (float)rand() / (float)RAND_MAX - 1;
    bgs.posy = 1.5 * (float)rand() / (float)RAND_MAX - 1;
  }

  applyDisplaySettingsDeferred();
  forceViewportResetIfNeededForGui();
#ifndef ANDROID
  #ifndef ANDROID
  glutPostRedisplay();
#endif
#else
  /* On Android, redisplay should be driven by the app render loop */
#endif
}

void keyboardGui(unsigned char key, int x, int y) {
  int i;
  switch(key) {
  case 27:
    /* ESC Back: apply any pending display changes */
    requestDisplayApply();
    if(pCurrent->parent == NULL)
      restoreCallbacks();
    else
      pCurrent = pCurrent->parent;
    break;
  case 13: case ' ':
    menuAction(*(pCurrent->pEntries + pCurrent->iHighlight));
    break;
  case 'q': exit(0); break;
  case 'l':
    printf("%d entries:\n", pCurrent->nEntries);
    for(i = 0; i < pCurrent->nEntries; i++)
      printf("printing '%s' - %d entries\n",
	     ((Menu*)*(pCurrent->pEntries + i))->szName,
	     ((Menu*)*(pCurrent->pEntries + i))->nEntries);
    break;
  default: printf("got key %d\n", key);
  }
}

void  specialGui(int key, int x, int y) {
  switch(key) {
  case GLUT_KEY_DOWN:
    pCurrent->iHighlight = (pCurrent->iHighlight + 1) % pCurrent->nEntries;
    break;
  case GLUT_KEY_UP:
    pCurrent->iHighlight = (pCurrent->iHighlight - 1) % pCurrent->nEntries;
    if(pCurrent->iHighlight < 0)
      pCurrent->iHighlight = pCurrent->nEntries - 1;
    break;
  }
}

void initGui() {
  /* init states */
  bgs.d = 0;
  bgs.posx = -1;
  bgs.posy = -1;
  bgs.lt = getElapsedTime();

  pCurrent = *pMenuList; /* erstes Menu ist RootMenu - Default pCurrent */
  pCurrent->iHighlight = 0;

  /* rasonly(game->screen); */
}

void initGLGui() {
#ifndef ANDROID
  glShadeModel(GL_SMOOTH);
#endif
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  // Ensure GUI overlays render on Android/GLES
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
}

callbacks guiCallbacks = {
  displayGui, idleGui, keyboardGui, specialGui, initGui, initGLGui
};
