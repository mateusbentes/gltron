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

#define N 20.0
  checkGLError("gui.c displayGui - before clear");
  glClearColor(0.0, 0.0, 1.0, 0.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

#ifdef ANDROID
  // Ensure 2D projection and bind shader for Android GUI
  rasonly(game->screen);
  { GLuint sp = shader_get_basic(); if (sp) useShaderProgram(sp); }
  // Draw a simple background (textured if available, else solid)
  {
    GLuint sp = shader_get_basic();
    if (sp) {
      // Fullscreen quad in pixel coords
      GLfloat vx = 0.f, vy = 0.f;
      GLfloat vw = (GLfloat)game->screen->vp_w;
      GLfloat vh = (GLfloat)game->screen->vp_h;
      GLfloat verts[8] = { vx, vy,  vx+vw, vy,  vx+vw, vy+vh,  vx, vy+vh };
      // Try textured background using texGui
      int textured = (game && game->screen && game->screen->texGui != 0);
      GLint a_pos = glGetAttribLocation(sp, "position");
      GLint a_uv  = glGetAttribLocation(sp, "texCoord");
      glEnableVertexAttribArray(a_pos);
      glVertexAttribPointer(a_pos, 2, GL_FLOAT, GL_FALSE, 0, verts);
      if (textured) {
        GLfloat uvs[8] = { 0,0, 1,0, 1,1, 0,1 };
        glEnableVertexAttribArray(a_uv);
        glVertexAttribPointer(a_uv, 2, GL_FLOAT, GL_FALSE, 0, uvs);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, game->screen->texGui);
        setTexture(sp, 0);
        setColor(sp, 1,1,1,1);
      } else {
        // Solid subtle blue background
        setColor(sp, 0.1f, 0.15f, 0.25f, 1.0f);
      }
      glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
      if (textured) glDisableVertexAttribArray(a_uv);
      glDisableVertexAttribArray(a_pos);
    }
  }
#endif

  guiProjection(game->screen->vp_w, game->screen->vp_h);

#ifndef ANDROID
  // Set texture parameters and draw background using fixed-function on desktop
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // Bind texture once before the loop
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, game->screen->texGui);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

  // Use vertex arrays instead of immediate mode
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

  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_COLOR_ARRAY);
  glVertexPointer(2, GL_FLOAT, 0, vertices);
  glColorPointer(3, GL_FLOAT, 0, colors);
  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
  glDisableClientState(GL_COLOR_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);
#else
  // Android: skip legacy background to avoid ES1 usage
#endif

#ifndef ANDROID
  for(y1 = -1; y1 < 1; y1 += 2 / N) {
    y2 = y1 + 2 / N;
    for(x = -1; x < 1; x += 2 / N) {
      c1 = (x + 1) / 2;
      c2 = (x + 2 / N + 1) / 2;

      c1 = c1 / 2 + 0.25;
      c2 = c2 / 2 + 0.25;

      // Use vertex arrays instead of immediate mode
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
#endif

  x = bgs.posx;
  y = bgs.posy;
  w = 1;
  h = w/4;

  alpha = (sin(bgs.d - M_PI / 2) + 1) / 2;

#ifndef ANDROID
  // Use vertex arrays instead of immediate mode
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

  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glVertexPointer(2, GL_FLOAT, 0, vertices);
  glTexCoordPointer(2, GL_FLOAT, 0, texCoords);
  glColor4f(1.0f, 1.0f, 0.0f, alpha);
  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);
#endif

  glDisable(GL_TEXTURE_2D);

  #ifdef ANDROID
  // OpenGL ES has no fixed-function glColor*. Menu rendering on Android sets color
  // via setColor(...) in menu.c; no color state is set here.
#else
  glColor3f(1.0, 0.0, 1.0);
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
#ifndef ANDROID
  glDisable(GL_LIGHTING);
  glDisable(GL_DEPTH_TEST);
#endif
}

callbacks guiCallbacks = {
  displayGui, idleGui, keyboardGui, specialGui, initGui, initGLGui
};
