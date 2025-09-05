#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

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
  glClearColor(0.0, 0.0, 0.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

#ifdef ANDROID
  // Android: prepare shader but avoid forcing projection/view/model here
  GLuint shaderProgram = shader_get_basic();
  if (shaderProgram) {
    useShaderProgram(shaderProgram);
    setColor(shaderProgram, 1.0f, 1.0f, 1.0f, 1.0f);
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

#ifdef ANDROID
  // Android/GLES2: draw background in clip-space with identity matrices
  // Bind white texture to avoid black modulation
  glActiveTexture(GL_TEXTURE0);
  static GLuint s_white_tex_bg = 0;
  if (s_white_tex_bg == 0) {
    GLubyte px[4] = {255,255,255,255};
    glGenTextures(1, &s_white_tex_bg);
    glBindTexture(GL_TEXTURE_2D, s_white_tex_bg);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, px);
  }
  glBindTexture(GL_TEXTURE_2D, s_white_tex_bg);
  setTexture(shaderProgram, 0);

  // Use identity matrices locally for clip-space verts
  const GLfloat I[16] = {
    1,0,0,0,
    0,1,0,0,
    0,0,1,0,
    0,0,0,1
  };
  setProjectionMatrix(shaderProgram, (float*)I);
  setViewMatrix(shaderProgram, (float*)I);
  setModelMatrix(shaderProgram, (float*)I);

  // Background uses a solid color for simplicity
  setColor(shaderProgram, 0.2f, 0.2f, 0.5f, 1.0f);

  GLfloat bgVerts[8] = {
    -1.0f,-1.0f,
     1.0f,-1.0f,
     1.0f, 1.0f,
    -1.0f, 1.0f
  };
  GLint a_pos_bg = glGetAttribLocation(shaderProgram, "position");
  glEnableVertexAttribArray(a_pos_bg);
  glVertexAttribPointer(a_pos_bg, 2, GL_FLOAT, GL_FALSE, 0, bgVerts);
  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
  glDisableVertexAttribArray(a_pos_bg);
#else
  // Draw background using immediate mode for non-Android
  glBegin(GL_TRIANGLE_FAN);
  for (int i = 0; i < 4; i++) {
    glColor3f(colors[i*3], colors[i*3+1], colors[i*3+2]);
    glVertex2f(vertices[i*2], vertices[i*2+1]);
  }
  glEnd();
#endif

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

#ifdef ANDROID
      // Android/GLES2: draw grid tile in clip-space with identity
      glActiveTexture(GL_TEXTURE0);
      static GLuint s_white_tex_grid = 0;
      if (s_white_tex_grid == 0) {
        GLubyte px[4] = {255,255,255,255};
        glGenTextures(1, &s_white_tex_grid);
        glBindTexture(GL_TEXTURE_2D, s_white_tex_grid);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, px);
      }
      glBindTexture(GL_TEXTURE_2D, s_white_tex_grid);
      setTexture(shaderProgram, 0);

      const GLfloat Igrid[16] = {
        1,0,0,0,
        0,1,0,0,
        0,0,1,0,
        0,0,0,1
      };
      setProjectionMatrix(shaderProgram, (float*)Igrid);
      setViewMatrix(shaderProgram, (float*)Igrid);
      setModelMatrix(shaderProgram, (float*)Igrid);

      GLint a_pos_grid = glGetAttribLocation(shaderProgram, "position");
      glEnableVertexAttribArray(a_pos_grid);
      glVertexAttribPointer(a_pos_grid, 2, GL_FLOAT, GL_FALSE, 0, vertices);
      // use left edge color as uniform approximation
      setColor(shaderProgram, colors[0], colors[1], colors[2], 1.0f);
      glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
      glDisableVertexAttribArray(a_pos_grid);
#else
      // Draw grid using immediate mode for non-Android
      glBegin(GL_TRIANGLE_FAN);
      for (int i = 0; i < 4; i++) {
        glColor3f(colors[i*3], colors[i*3+1], colors[i*3+2]);
        glVertex2f(vertices[i*2], vertices[i*2+1]);
      }
      glEnd();
#endif
    }
  }

  // Draw logo
  // Compute logo in pixel space for consistent sizing
  alpha = (sin(bgs.d - M_PI / 2) + 1) / 2;
#ifdef ANDROID
  float vpw = (float)game->screen->vp_w;
  float vph = (float)game->screen->vp_h;
  // Logo size: half of viewport width, 4:1 aspect
  w = vpw * 0.5f;
  h = w * 0.25f;
  // Map bgs.posx,posy [-1,1] to pixel center
  x = (bgs.posx * 0.5f + 0.5f) * vpw;
  y = (bgs.posy * 0.5f + 0.5f) * vph;
  // Build pixel-space quad
  GLfloat logoVerts[8];
  logoVerts[0] = x - w * 0.5f; logoVerts[1] = y - h * 0.5f;
  logoVerts[2] = x + w * 0.5f; logoVerts[3] = y - h * 0.5f;
  logoVerts[4] = x + w * 0.5f; logoVerts[5] = y + h * 0.5f;
  logoVerts[6] = x - w * 0.5f; logoVerts[7] = y + h * 0.5f;
#else
  x = bgs.posx;
  y = bgs.posy;
  w = 1;
  h = w/4;
  vertices[0] = x - w / 2; vertices[1] = y - h / 2;
  vertices[2] = x + w / 2; vertices[3] = y - h / 2;
  vertices[4] = x + w / 2; vertices[5] = y + h / 2;
  vertices[6] = x - w / 2; vertices[7] = y + h / 2;
#endif

  GLfloat texCoords[8] = {
    0.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f
  };

#ifdef ANDROID
  // Android/GLES2: before logo/text rendering, set 2D pixel-space projection
  float wvp = (float)game->screen->vp_w;
  float hvp = (float)game->screen->vp_h;
  GLfloat proj2D[16] = {
    2.0f / wvp, 0.0f,       0.0f, 0.0f,
    0.0f,       2.0f / hvp, 0.0f, 0.0f,
    0.0f,       0.0f,       1.0f, 0.0f,
    -1.0f,     -1.0f,       0.0f, 1.0f
  };
  const GLfloat Ilogo[16] = {
    1,0,0,0,
    0,1,0,0,
    0,0,1,0,
    0,0,0,1
  };
  setProjectionMatrix(shaderProgram, (float*)proj2D);
  setViewMatrix(shaderProgram, (float*)Ilogo);
  setModelMatrix(shaderProgram, (float*)Ilogo);

  // Draw the logo with pixel-space vertices
  GLint a_pos_logo = glGetAttribLocation(shaderProgram, "position");
  GLint a_uv_logo  = glGetAttribLocation(shaderProgram, "texCoord");
  glEnableVertexAttribArray(a_pos_logo);
  glEnableVertexAttribArray(a_uv_logo);
  glVertexAttribPointer(a_pos_logo, 2, GL_FLOAT, GL_FALSE, 0, logoVerts);
  glVertexAttribPointer(a_uv_logo, 2, GL_FLOAT, GL_FALSE, 0, texCoords);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, game->screen->texGui);
  setTexture(shaderProgram, 0);
  setColor(shaderProgram, 1.0f, 1.0f, 1.0f, alpha);

  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

  glDisableVertexAttribArray(a_pos_logo);
  glDisableVertexAttribArray(a_uv_logo);
#else
  // Set up color for logo on non-Android
  glColor4f(1.0f, 1.0f, 0.0f, alpha);
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, game->screen->texGui);

  // Draw logo using immediate mode for non-Android
  glBegin(GL_TRIANGLE_FAN);
  for (int i = 0; i < 4; i++) {
    glTexCoord2f(texCoords[i*2], texCoords[i*2+1]);
    glVertex2f(vertices[i*2], vertices[i*2+1]);
  }
  glEnd();

  glDisable(GL_TEXTURE_2D);
#endif

  // Draw menu
#ifdef ANDROID
  if (shaderProgram) {
    // Re-bind shader and re-apply 2D pixel projection before text drawing
    useShaderProgram(shaderProgram);
    float wvp = (float)game->screen->vp_w;
    float hvp = (float)game->screen->vp_h;
    GLfloat proj2D[16] = {
      2.0f / wvp, 0.0f,       0.0f, 0.0f,
      0.0f,       2.0f / hvp, 0.0f, 0.0f,
      0.0f,       0.0f,       1.0f, 0.0f,
      -1.0f,     -1.0f,       0.0f, 1.0f
    };
    const GLfloat I2D[16] = {
      1,0,0,0,
      0,1,0,0,
      0,0,1,0,
      0,0,0,1
    };
    setProjectionMatrix(shaderProgram, (float*)proj2D);
    setViewMatrix(shaderProgram, (float*)I2D);
    setModelMatrix(shaderProgram, (float*)I2D);
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
  glutPostRedisplay();
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
#ifdef ANDROID
  init_shaders_android();
  GLuint prog = shader_get_basic();
#endif

#ifdef ANDROID
  init_shaders_android();
#endif

#ifndef ANDROID
  glShadeModel(GL_SMOOTH);
#endif
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  // Ensure GUI overlays render on Android/GLES
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
#ifdef ANDROID
  // Initialize a 1x1 white texture for non-textured GUI draws
  static GLuint s_white_tex = 0;
  if (s_white_tex == 0) {
    GLubyte pixel[4] = {255,255,255,255};
    glGenTextures(1, &s_white_tex);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, s_white_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
  }
#endif
}

callbacks guiCallbacks = {
  displayGui, idleGui, keyboardGui, specialGui, initGui, initGLGui
};
