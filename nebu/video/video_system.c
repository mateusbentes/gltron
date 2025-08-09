#include "video/nebu_video_system.h"
#include "video/nebu_renderer_gl.h"
#include "base/nebu_system.h"
#include "base/nebu_assert.h"
#include "base/nebu_debug_memory.h"
#include "../../src/include/base/sdl_compat.h"

#ifndef SYSTEM_DOUBLE
#define SYSTEM_DOUBLE     0x0001
#endif
#ifndef SYSTEM_32_BIT
#define SYSTEM_32_BIT     0x0002
#endif
#ifndef SYSTEM_ALPHA
#define SYSTEM_ALPHA      0x0004
#endif
#ifndef SYSTEM_DEPTH
#define SYSTEM_DEPTH      0x0008
#endif
#ifndef SYSTEM_STENCIL
#define SYSTEM_STENCIL    0x0010
#endif
#ifndef SYSTEM_FULLSCREEN
#define SYSTEM_FULLSCREEN 0x0020
#endif

#include <SDL2/SDL.h>
#ifdef __ANDROID__
  #include <GLES2/gl2.h>
#else
  #include <GL/gl.h>
#endif

static SDL_Surface *gScreen = NULL;
static int width = 0;
static int height = 0;
static int bitdepth = 0;
static int flags = 0;
static int video_initialized = 0;
static int window_id = 0;
static SDL_GLContext g_glcontext = NULL;
SDL_Window *g_window = NULL;

void nebu_Video_Init(void) {
    if(SDL_Init(SDL_INIT_VIDEO) < 0 ) {
        fprintf(stderr, "Couldn't initialize SDL video: %s\n", SDL_GetError());
        nebu_assert(0); exit(1);
    } else {
        video_initialized = 1;
    }
}

void nebu_Video_SetWindowMode(int x, int y, int w, int h) {
    fprintf(stderr, "ignoring (%d,%d) initial window position - feature not implemented\n", x, y);
    width = w;
    height = h;
    // No direct SDL_SetVideoMode_Compat, use SDL_CreateWindow
    // Window will be created in createWindow()
}

void nebu_Video_SetDisplayMode(int f) {
    int zdepth;
    flags = f;
    if(!video_initialized) {
        if(SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) {
            fprintf(stderr, "[system] can't initialize Video: %s\n", SDL_GetError());
            nebu_assert(0); exit(1);
        }
    }
    if(flags & SYSTEM_DOUBLE)
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

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
        SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    if(flags & SYSTEM_DEPTH)
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, zdepth);
    if(flags & SYSTEM_STENCIL)
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    else
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 0);

    // For OpenGL ES 2.0 context
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);

    video_initialized = 1;
}

void printOpenGLDebugInfo(void) {
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
    // SDL_SetGamma is deprecated, use SDL_SetWindowGammaRamp if needed
    // Not implemented for OpenGL ES
}

void createWindow(const char *name) {
    Uint32 sdl_flags = SDL_WINDOW_OPENGL;
    if(flags & SYSTEM_FULLSCREEN)
        sdl_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;

    g_window = SDL_CreateWindow(
        name,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        width, height,
        sdl_flags
    );
    if(!g_window) {
        fprintf(stderr, "[system] Couldn't create SDL window: %s\n", SDL_GetError());
        nebu_assert(0); exit(1);
    }

    g_glcontext = SDL_GL_CreateContext(g_window);
    if(!g_glcontext) {
        fprintf(stderr, "[system] Couldn't create GL context: %s\n", SDL_GetError());
        nebu_assert(0); exit(1);
    }

    SDL_GL_SetSwapInterval(1); // Enable vsync

    window_id = 1;
}

void nebu_Video_GetDisplayDepth(int *r, int *g, int *b, int *a) {
    SDL_GL_GetAttribute(SDL_GL_RED_SIZE, r);
    SDL_GL_GetAttribute(SDL_GL_GREEN_SIZE, g);
    SDL_GL_GetAttribute(SDL_GL_BLUE_SIZE, b);
    SDL_GL_GetAttribute(SDL_GL_ALPHA_SIZE, a);
}

int nebu_Video_Create(char *name) {
    nebu_assert(window_id == 0);
    nebu_assert(width != 0 && height != 0);

    createWindow(name);

    // No GLEW: OpenGL ES 2.0+ functions are available by default
    printOpenGLDebugInfo();

    glClearColor(0,0,0,0);
    glClear(GL_COLOR_BUFFER_BIT);
    nebu_System_SwapBuffers();
    return window_id;
}

void nebu_Video_Destroy(int id) {
    nebu_assert(id == window_id);
    window_id = 0;
    if (g_glcontext) {
        SDL_GL_DeleteContext(g_glcontext);
        g_glcontext = NULL;
    }
    if (g_window) {
        SDL_DestroyWindow(g_window);
        g_window = NULL;
    }
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
    while((error = glGetError()) != GL_NO_ERROR) {
        fprintf(stderr, "[system] GL error near %s: %d\n", where, (int)error);
    }
}

void nebu_Video_SwapBuffers(void) {
    SDL_GL_SwapWindow(g_window);
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
    if (w) *w = width;
    if (h) *h = height;
}
