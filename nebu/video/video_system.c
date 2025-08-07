#include "video/nebu_video_system.h"  // Include this header first to get the SYSTEM_* constants
#include "video/nebu_renderer_gl.h"
#include "base/nebu_system.h"
#include "base/nebu_assert.h"
#include "base/nebu_debug_memory.h"
#include "../../src/include/base/sdl_compat.h"

/* Define constants directly in this file to avoid include issues */
#ifndef SYSTEM_DOUBLE
#define SYSTEM_DOUBLE     0x0001  /* Double buffering */
#endif

#ifndef SYSTEM_32_BIT
#define SYSTEM_32_BIT     0x0002  /* 32-bit color depth */
#endif

#ifndef SYSTEM_ALPHA
#define SYSTEM_ALPHA      0x0004  /* Alpha channel */
#endif

#ifndef SYSTEM_DEPTH
#define SYSTEM_DEPTH      0x0008  /* Depth buffer */
#endif

#ifndef SYSTEM_STENCIL
#define SYSTEM_STENCIL    0x0010  /* Stencil buffer */
#endif

#ifndef SYSTEM_FULLSCREEN
#define SYSTEM_FULLSCREEN 0x0020  /* Fullscreen mode */
#endif

/* Include SDL2 header */
#include <SDL2/SDL.h>

static SDL_Surface *gScreen = NULL;
static int width = 0;
static int height = 0;
static int bitdepth = 0;
static int flags = 0;
static int video_initialized = 0;
static int window_id = 0;

void nebu_Video_Init(void) {
  if(SDL_Init(SDL_INIT_VIDEO) < 0 ) {
    fprintf(stderr, "Couldn't initialize SDL video: %s\n", SDL_GetError());
    nebu_assert(0); exit(1); /* OK: critical, no visual */
  } else
		video_initialized = 1;
}

void nebu_Video_SetWindowMode(int x, int y, int w, int h) {
  fprintf(stderr, "ignoring (%d,%d) initial window position - feature not implemented\n", x, y);
  width = w;
  height = h;

  /* Use SDL_SetVideoMode_Compat instead of direct SDL functions */
  gScreen = SDL_SetVideoMode_Compat(w, h, 0, SDL_WINDOW_OPENGL);
  if(!gScreen) {
    fprintf(stderr, "Video initialization failed: %s\n", SDL_GetError());
    nebu_assert(0); exit(1); /* OK: critical, no visual */
  }
}

void nebu_Video_SetDisplayMode(int f) {
  int zdepth;

  flags = f;
  if(!video_initialized) {
    if(SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) {
      fprintf(stderr, "[system] can't initialize Video: %s\n", SDL_GetError());
      nebu_assert(0); exit(1); /* OK: critical, no visual */
    }
  }
  if(flags & SYSTEM_DOUBLE)
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1);

  if(flags & SYSTEM_32_BIT) {
    zdepth = 24;
    bitdepth = 32;
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
  } else {
    zdepth = 16;
    bitdepth = 0;
  }
  if(flags & SYSTEM_ALPHA)
	  SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 8 );
  if(flags & SYSTEM_DEPTH)
    SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, zdepth);
  if(flags & SYSTEM_STENCIL)
     SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 8);
  else
     SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 0);
  video_initialized = 1;
}

void printOpenGLDebugInfo(void)
{
	int r, g, b, a;

	fprintf(stderr, "GL vendor: %s\n", glGetString(GL_VENDOR));
	fprintf(stderr, "GL renderer: %s\n", glGetString(GL_RENDERER));
	fprintf(stderr, "GL version: %s\n", glGetString(GL_VERSION));

	fprintf(stderr, "Bitdepth:\n");
	nebu_Video_GetDisplayDepth(&r, &g, &b, &a);

	fprintf(stderr, "  Red: %d\n", r);
	fprintf(stderr, "  Green: %d\n", g);
	fprintf(stderr, "  Blue: %d\n", b);
	fprintf(stderr, "  Alpha: %d\n", a);
}

void SystemSetGamma(float red, float green, float blue) {
  //SDL_SetGamma(red, green, blue);
}

void createWindow(const char *name)
{
  /* Use compatibility layer instead of direct SDL functions */
  Uint32 sdl_flags = SDL_WINDOW_OPENGL;
  if(flags & SYSTEM_FULLSCREEN)
    sdl_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;

  gScreen = SDL_SetVideoMode_Compat(width, height, bitdepth, sdl_flags);
  if(!gScreen) {
    fprintf(stderr, "[system] Couldn't set GL mode: %s\n", SDL_GetError());
    nebu_assert(0); exit(1); /* OK: critical, no visual */
  }

  window_id = 1;
  SDL_GL_SetSwapInterval(1);
}

void nebu_Video_GetDisplayDepth(int *r, int *g, int *b, int *a)
{
	SDL_GL_GetAttribute(SDL_GL_RED_SIZE, r);
	SDL_GL_GetAttribute(SDL_GL_GREEN_SIZE, g);
	SDL_GL_GetAttribute(SDL_GL_BLUE_SIZE, b);
	SDL_GL_GetAttribute(SDL_GL_ALPHA_SIZE, a);
}

int nebu_Video_Create(char *name) {
	nebu_assert (window_id == 0);  // only single window allowed for now
	nebu_assert (width != 0 && height != 0);

	createWindow(name);
	glewInit();

	if(!GLEW_ARB_multitexture)
	{
		printOpenGLDebugInfo();
		nebu_Video_Destroy(window_id);
		nebu_Video_Init();
		nebu_Video_SetDisplayMode(flags);
		// try without alpha
		fprintf(stderr, "trying without destination alpha\n");
		SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 0);
		createWindow(name);
		glewInit();
		if(!GLEW_ARB_multitexture)
		{
			printOpenGLDebugInfo();
			fprintf(stderr, "multitexturing is not available\n");
			nebu_assert(0); exit(1);
		}
	}
	printOpenGLDebugInfo();

	glClearColor(0,0,0,0);
	glClear(GL_COLOR_BUFFER_BIT);
	nebu_System_SwapBuffers();
	return window_id;
}

void nebu_Video_Destroy(int id) {
	nebu_assert(id == window_id);
	window_id = 0;
	SDL_DestroyWindow_Compat();
	video_initialized = 0;
}

void SystemReshapeFunc(void(*reshape)(int w, int h)) {
	fprintf(stderr, "can't set reshape function (%p) - feature not supported\n", reshape);
}

void nebu_Video_WarpPointer(int x, int y) {
  if (g_window) {
    SDL_WarpMouseInWindow(g_window, x, y);
  }
}

void nebu_Video_CheckErrors(const char *where) {
	GLenum error;
	while((error = glGetError()) != GL_NO_ERROR)
	{
		fprintf(stderr, "[system] GL error near %s: %d\n", where, (int)error);
	}
}

void nebu_Video_SwapBuffers(void) {
	SDL_GL_SwapWindow_Compat();
}

void nebu_Video_GetDimension(int *w, int *h) {
	if (w) *w = width;
	if (h) *h = height;
}

void nebu_Video_SetDimension(int w, int h) {
	width = w;
	height = h;
}

int nebu_Video_GetWidth(void) {
	return width;
}

int nebu_Video_GetHeight(void) {
	return height;
}

void nebu_Video_GetScreenSize(int *w, int *h) {
	// For desktop, return current window size
	if (w) *w = width;
	if (h) *h = height;
}
