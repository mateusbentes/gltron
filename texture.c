#include "gltron.h"

#include "sgi_texture.h"

void deleteTextures(gDisplay *d) {
  glDeleteTextures(1, &(d->texFloor));
  glDeleteTextures(1, &(d->texWall));
  glDeleteTextures(1, &(d->texGui));
  glDeleteTextures(1, &(d->texCrash));
}

void loadTexture(char *filename, int format) {
  char *path;
  sgi_texture *tex;

  path = getFullPath(filename);
  if(path != 0)
    tex = load_sgi_texture(path);
  else {
    fprintf(stderr, "fatal: could not load %s, exiting...\n", filename);
    exit(1);
  }
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexImage2D(GL_TEXTURE_2D, 0, format, tex->width, tex->height, 0,
	       GL_RGBA, GL_UNSIGNED_BYTE, tex->data);
  free(tex->data);
  free(tex);
}

void initTexture(gDisplay *d) {
  checkGLError("texture.c initTexture - start");

  /* floor texture */
  glGenTextures(1, &(d->texFloor));
  glBindTexture(GL_TEXTURE_2D, d->texFloor);
  loadTexture("gltron_floor.sgi", GL_RGB16);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  checkGLError("texture.c initTextures - floor");
  /* menu icon */
  glGenTextures(1, &(d->texGui));
  glBindTexture(GL_TEXTURE_2D, d->texGui);
  loadTexture("gltron.sgi", GL_RGBA);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  checkGLError("texture.c initTextures - gui");

  /* wall texture */
  glGenTextures(1, &(d->texWall));
  glBindTexture(GL_TEXTURE_2D, d->texWall);
  loadTexture("gltron_wall.sgi", GL_RGBA);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  /* crash texture */
  glGenTextures(1, &(d->texCrash));
  glBindTexture(GL_TEXTURE_2D, d->texCrash);
  loadTexture("gltron_crash.sgi", GL_RGBA);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  checkGLError("texture.c initTextures - end");
}



