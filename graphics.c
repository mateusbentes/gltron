#include "gltron.h"
#include <string.h>

#ifdef ANDROID
#include <GLES2/gl2.h>
#include "shaders.h"
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

// Global shader program for Android is declared in shaders.h as extern.

void checkGLError(char *where) {
  int error;
  error = glGetError();
  if(error != GL_NO_ERROR)
    printf("[glError: %s] - %d\n", where, error);
}

void rasonly(gDisplay *d) {
  /* do rasterising only (in local display d) */
#ifdef ANDROID
  // For Android, use orthographic projection with GLES
  glViewport(d->vp_x, d->vp_y, d->vp_w, d->vp_h);

  // Set up orthographic projection matrix
  float left = 0.0f;
  float right = (GLfloat) d->vp_w;
  float bottom = 0.0f;
  float top = (GLfloat) d->vp_h;
  float near = 0.0f;
  float far = 1.0f;

  // Create orthographic projection matrix
  float projectionMatrix[16] = {
    2.0f / (right - left), 0.0f, 0.0f, 0.0f,
    0.0f, 2.0f / (top - bottom), 0.0f, 0.0f,
    0.0f, 0.0f, -2.0f / (far - near), 0.0f,
    -(right + left) / (right - left), -(top + bottom) / (top - bottom), -(far + near) / (far - near), 1.0f
  };

  // Use shader program and set projection matrix
  useShaderProgram(shaderProgram);
  setProjectionMatrix(shaderProgram, projectionMatrix);
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
      // printf("minimum FPS: %d - average FPS: %d\n", min, sum / FPS_HSIZE);
    }
  }

  sprintf(tmp, "average FPS: %d", fps_avg);
#ifdef ANDROID
  // For Android, we'll need to implement a text rendering function
  // This is a placeholder - you'll need to implement proper text rendering
  // for Android using shaders and textures
  // drawTextAndroid(d->vp_w - 180, d->vp_h - 20, 10, tmp);
#else
  glColor4f(1.0, 0.4, 0.2, 1.0);
  drawText(d->vp_w - 180, d->vp_h - 20, 10, tmp);
#endif

  sprintf(tmp, "minimum FPS: %d", fps_min);
#ifdef ANDROID
  // drawTextAndroid(d->vp_w - 180, d->vp_h - 35, 10, tmp);
#else
  drawText(d->vp_w - 180, d->vp_h - 35, 10, tmp);
#endif
}

void drawText(int x, int y, int size, char *text) {
  /* int i; */

  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_TEXTURE_2D);
  /* txfBindFontTexture(txf); */

#ifdef ANDROID
  // For Android, we'll need to implement a text rendering function
  // This is a placeholder - you'll need to implement proper text rendering
  // for Android using shaders and textures
  // drawTextAndroid(x, y, size, text);
#else
  glPushMatrix();

  glTranslatef(x, y, 0);
  glScalef(size, size, size);
  ftxRenderString(ftx, text, strlen(text));

  glPopMatrix();
#endif

  glDisable(GL_TEXTURE_2D);
  /* 
  glRasterPos2f(x, y);
  for(i = 0; *(text + i) != 0; i++)
    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *(text + i));
  */
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
  // For Android, use GLES functions with shaders
  GLuint shaderProgram = createShaderProgram();
  useShaderProgram(shaderProgram);

  // Set up vertex data
  GLfloat vertices[362*3]; // 360 degrees + center point
  vertices[0] = 0.0f; vertices[1] = 0.0f; vertices[2] = 0.0f; // Center point

  for(h = 0; h <= 360; h += 10) {
    int index = (h/10 + 1) * 3;
    hsv2rgb(h, 1, 1, &r, &g, &b);
    vertices[index] = cos(h * 2 * M_PI / 360);
    vertices[index+1] = sin(h * 2 * M_PI / 360);
    vertices[index+2] = 0.0f;
  }

  // Get attribute and uniform locations
  GLint positionLoc = glGetAttribLocation(shaderProgram, "position");
  GLint colorLoc = glGetUniformLocation(shaderProgram, "color");

  // Set up vertex buffer
  GLuint vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  // Draw the triangle fan
  glEnableVertexAttribArray(positionLoc);
  glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);

  // Draw the disc with smooth shading
  for(h = 0; h <= 360; h += 10) {
    hsv2rgb(h, 1, 1, &r, &g, &b);
    glUniform4f(colorLoc, r, g, b, 1.0f);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 362/3);
  }

  // Clean up
  glDisableVertexAttribArray(positionLoc);
  glDeleteBuffers(1, &vbo);
  glUseProgram(0);
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

// Initialize shader program for Android
#ifdef ANDROID
void initShaderProgram() {
  shaderProgram = createShaderProgram();
  if (shaderProgram == 0) {
    printf("Failed to create shader program\n");
  }
}
#endif
