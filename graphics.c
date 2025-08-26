#include "gltron.h"
#include <string.h>

#ifdef ANDROID
#include <GLES2/gl2.h>
#include "shaders.h"
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

// Global shader program for Android is managed centrally via shaders.c

void checkGLError(char *where) {
  int error = glGetError();
  if(error != GL_NO_ERROR)
    printf("[glError: %s] - %d\n", where, error);
}

void rasonly(gDisplay *d) {
  /* do rasterising only (in local display d) */
#ifdef ANDROID
  // For Android, use orthographic projection with GLES
  glViewport(d->vp_x, d->vp_y, d->vp_w, d->vp_h);

  float left = 0.0f;
  float right = (GLfloat) d->vp_w;
  float bottom = 0.0f;
  float top = (GLfloat) d->vp_h;
  float near = 0.0f;
  float far = 1.0f;

  float projectionMatrix[16] = {
    2.0f / (right - left), 0.0f, 0.0f, 0.0f,
    0.0f, 2.0f / (top - bottom), 0.0f, 0.0f,
    0.0f, 0.0f, -2.0f / (far - near), 0.0f,
    -(right + left) / (right - left), -(top + bottom) / (top - bottom), -(far + near) / (far - near), 1.0f
  };

  GLuint prog = shader_get_basic();
  if (prog) {
    useShaderProgram(prog);
    setProjectionMatrix(prog, projectionMatrix);
  }
#else
  // For desktop OpenGL
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0.0f, (GLfloat) d->vp_w, 0.0f, (GLfloat) d->vp_h, 0.0f, 1.0f);
  checkGLError("rasonly");
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glViewport(d->vp_x, d->vp_y, d->vp_w, d->vp_h);
#endif
}

void drawFPS(gDisplay *d) {
#define FPS_HSIZE 20
  /* draws FPS in upper left corner of Display d */
  static int fps_h[FPS_HSIZE];
  static int pos = -FPS_HSIZE;
  static int fps_min = 0;
  static int fps_avg = 0;

  char tmp[20];
  int diff;

  rasonly(d);
  diff = (dt > 0) ? dt : 1;

  if(pos < 0) {
    fps_avg = 1000 / diff;
    fps_min = 1000 / diff;
    fps_h[pos + FPS_HSIZE] = 1000 / diff;
    pos++;
  } else {
    fps_h[pos] = 1000 / diff;
    pos = (pos + 1) % FPS_HSIZE;
    if(pos % 10 == 0) {
      int i;
      int sum = 0;
      int min = 1000;
      for(i = 0; i < FPS_HSIZE; i++) {
        sum += fps_h[i];
        if(fps_h[i] < min)
          min = fps_h[i];
      }
      fps_min = min;
      fps_avg = sum / FPS_HSIZE;
    }
  }

  sprintf(tmp, "average FPS: %d", fps_avg);
#ifdef ANDROID
  { GLuint sp = shader_get_basic(); if (sp) { useShaderProgram(sp); setColor(sp, 1.0f, 0.4f, 0.2f, 1.0f); } }
  drawText(d->vp_w - 180, d->vp_h - 20, 10, tmp);
#else
  glColor4f(1.0, 0.4, 0.2, 1.0);
  drawText(d->vp_w - 180, d->vp_h - 20, 10, tmp);
#endif

  sprintf(tmp, "minimum FPS: %d", fps_min);
#ifdef ANDROID
  drawText(d->vp_w - 180, d->vp_h - 35, 10, tmp);
#else
  drawText(d->vp_w - 180, d->vp_h - 35, 10, tmp);
#endif
}

void drawText(int x, int y, int size, char *text) {
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
#ifndef ANDROID
  glEnable(GL_TEXTURE_2D);
  glPushMatrix();
  glTranslatef(x, y, 0);
  glScalef(size, size, size);
  ftxRenderString(ftx, text, strlen(text));
  glPopMatrix();
  glDisable(GL_TEXTURE_2D);
#else
  // Android: render text at pixel position (x,y) with uniform scale 'size'.
  if (!text) return;
  glDisable(GL_TEXTURE_2D); // no fixed-function textures in ES2

  // Bind shader and fully set render state for text
  GLuint prog = shader_get_basic();
  if (prog && game && game->screen) {
    useShaderProgram(prog);
    // Ortho projection in screen pixels
    GLfloat left = 0.0f, right = (GLfloat)game->screen->vp_w;
    GLfloat bottom = 0.0f, top = (GLfloat)game->screen->vp_h;
    GLfloat znear = 0.0f, zfar = 1.0f;
    GLfloat proj[16] = {
      2.0f/(right-left), 0, 0, 0,
      0, 2.0f/(top-bottom), 0, 0,
      0, 0, -2.0f/(zfar-znear), 0,
      -(right+left)/(right-left), -(top+bottom)/(top-bottom), -(zfar+znear)/(zfar-znear), 1
    };
    setProjectionMatrix(prog, proj);
    // View identity
    GLfloat view[16] = {1,0,0,0,  0,1,0,0,  0,0,1,0,  0,0,0,1};
    setViewMatrix(prog, view);
    // Model: translate to (x,y), scale by 'size'
    GLfloat model[16] = {
      (GLfloat)size, 0, 0, 0,
      0, (GLfloat)size, 0, 0,
      0, 0, 1, 0,
      (GLfloat)x, (GLfloat)y, 0, 1
    };
    setModelMatrix(prog, model);

    // Bind font texture if available
    if (game->screen->texFont) {
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, game->screen->texFont);
      setTexture(prog, 0);
    }
  }

  // If we have a font texture, render string; else draw a placeholder block
  if (game && game->screen && game->screen->texFont) {
    ftxRenderString(ftx, text, strlen(text));
  } else if (prog) {
    // Debug placeholder quad at (x,y)
    GLint a_pos = glGetAttribLocation(prog, "position");
    GLfloat verts[8] = { (GLfloat)x, (GLfloat)y, (GLfloat)(x+size), (GLfloat)y,
                         (GLfloat)(x+size), (GLfloat)(y+size), (GLfloat)x, (GLfloat)(y+size) };
    glEnableVertexAttribArray(a_pos);
    glVertexAttribPointer(a_pos, 2, GL_FLOAT, GL_FALSE, 0, verts);
    setColor(prog, 1.f, 0.f, 0.f, 0.8f);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glDisableVertexAttribArray(a_pos);
  }
#endif
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  polycount += strlen(text);
}

int hsv2rgb(float h, float s, float v, float *r, float *g, float *b) {
  float j, f, m, n, k;
  int i;

  if(s == 0)
    if(h == -1)
      *r = *g = *b = v;
    else return 1;
  else {
    if(h >= 360) h = 0;
    i = h / 60; /* integer */
    j = h / 60;

    f = j - i;
    m = v * (1 - s);
    n = v * (1 - s * f);
    k = v * (1 - s * (1 - f));
    switch(i) {
    case 0: *r = v; *g = k; *b = m; break;
    case 1: *r = n; *g = v; *b = m; break;
    case 2: *r = m; *g = v; *b = k; break;
    case 3: *r = m; *g = n; *b = v; break;
    case 4: *r = k; *g = m; *b = v; break;
    case 5: *r = v; *g = m; *b = n; break;
    }
  }
  return 0;
}

void colorDisc() {
  int h;
  float r, g, b;
#ifdef ANDROID
  // For Android, use GLES with centralized shader
  GLuint prog = shader_get_basic();
  if (!prog) return;
  useShaderProgram(prog);

  GLfloat vertices[362*3]; // 360 degrees + center point
  vertices[0] = 0.0f; vertices[1] = 0.0f; vertices[2] = 0.0f; // Center point
  for(h = 0; h <= 360; h += 10) {
    int index = (h/10 + 1) * 3;
    hsv2rgb(h, 1, 1, &r, &g, &b);
    vertices[index] = cos(h * 2 * M_PI / 360);
    vertices[index+1] = sin(h * 2 * M_PI / 360);
    vertices[index+2] = 0.0f;
  }

  GLint positionLoc = glGetAttribLocation(prog, "position");
  GLint colorLoc = glGetUniformLocation(prog, "color");

  GLuint vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glEnableVertexAttribArray(positionLoc);
  glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);

  for(h = 0; h <= 360; h += 10) {
    hsv2rgb(h, 1, 1, &r, &g, &b);
    glUniform4f(colorLoc, r, g, b, 1.0f);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 362/3);
  }

  glDisableVertexAttribArray(positionLoc);
  glDeleteBuffers(1, &vbo);
  /* keep program bound; drawCam controls unbinding */
#else
  // For desktop OpenGL
  glShadeModel(GL_SMOOTH);
  glBegin(GL_TRIANGLE_FAN);
  glColor3f(1.0, 1.0, 1.0);
  glVertex3f(0, 0, 0);
  for(h = 0; h <= 360; h += 10) {
    hsv2rgb(h, 1, 1, &r, &g, &b);
    glColor3f(r, g, b);
    glVertex3f(cos(h * 2 * M_PI / 360), sin(h * 2 * M_PI / 360), 0);
  }
  glEnd();
#endif
}

#ifdef ANDROID
void initShaderProgram() {
  // Centralized shader init
  init_shaders_android();
}
#endif
