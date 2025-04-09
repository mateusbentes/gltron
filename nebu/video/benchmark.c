
#include "video/nebu_gl.h"  // Include the OpenGL headers

float vertices[] = { 
  -.5, -.5, 0, 1,
  .5, -.5, 0, 1,
  .5, .5, 0, 1,
  -.5, .5, 0, 1
};

float tex_coords[] = {
  0, 0,
  1, 0,
  1, 1,
  0, 1
};

float colors[] = {
  1, 1, 1, 1,
  1, 0, 0, 1,
  0, 1, 0, 1,
  0, 0, 1, 1
};

int indices[] = { 0, 1, 2, 0, 2, 3 };

/* fill a quarter of the screen */

void fill_plain() {
  glDepthMask(GL_TRUE);
  glEnable(GL_DEPTH_TEST);

  glDisableClientState(GL_COLOR_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glEnableClientState(GL_VERTEX_ARRAY);

  glColor4f(1, 1, 1, 1.0f);
  glDrawElements(GL_TRIANGLES, 2, GL_UNSIGNED_INT, indices);
  glDisableClientState(GL_VERTEX_ARRAY);
}

void fill_textured() {
  glDepthMask(GL_TRUE);
  glEnable(GL_DEPTH_TEST);
}

void setupGL() {
  unsigned char pixels[] = { 255, 0, 0, 255 };
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glVertexPointer(4, GL_FLOAT, 0, vertices);
  glTexCoordPointer(2, GL_FLOAT, 0, tex_coords);  // Fixed: glTexturePointer -> glTexCoordPointer
  glColorPointer(4, GL_FLOAT, 0, colors);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 2, 2, 0, GL_LUMINANCE,
	       GL_UNSIGNED_BYTE, pixels);

  // Fixed: glTexParameterf needs parameters
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}
