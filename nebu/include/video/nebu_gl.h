#ifndef NEBU_GL_H
#define NEBU_GL_H

/* Include the appropriate OpenGL headers based on platform */
#ifdef __APPLE__
  #include <OpenGL/gl.h>
  #include <OpenGL/glu.h>
#elif defined(_WIN32)
  #include <GL/gl.h>
  #include <GL/glu.h>
#else
  /* Linux/Unix systems */
  #include <GL/gl.h>
  #include <GL/glu.h>
#endif

/* Fallback to SDL's OpenGL headers if needed */
#ifndef GL_VERSION_1_1
  #include "SDL_opengl.h"
#endif

#endif /* NEBU_GL_H */
