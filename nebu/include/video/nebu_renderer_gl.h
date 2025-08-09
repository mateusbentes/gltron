#ifndef NEBU_RENDERER_GL_H
#define NEBU_RENDERER_GL_H

// For SDL2 + OpenGL ES 2.0+ only
#ifdef __ANDROID__
  #include <GLES2/gl2.h>
#else
  #include <GLES2/gl2.h>
#endif

#include <stdio.h>

// Renderer type flags (for future use)
#define RENDERER_TYPE_ALL           255
#define RENDERER_TYPE_COLOR         1
#define RENDERER_TYPE_NORMAL        2
#define RENDERER_TYPE_TEXTURE_COORD 4
#define RENDERER_TYPE_TEXTURE       8
#define RENDERER_TYPE_TEXTURE_MODE  16

// Renderer state for OpenGL ES 2.0+ (no fixed-function pipeline)
typedef struct GLstate {
  int tex_id;           // Currently bound texture ID (if any)
  int type_mask;        // Which modes to change (for future use)
  int binds;            // Texture bind changes (for statistics)
  int mod_changes;      // Mode changes (for statistics)
} GLstate;

typedef struct Renderer {
  int ext_filter_anisotropic; // Anisotropic filtering support (for statistics/capabilities)
} Renderer;

extern void initRenderer(void);
extern void printRendererInfo(void);
extern void clearState(void);

extern Renderer renderer;
extern GLstate *state;

#endif // NEBU_RENDERER_GL_H
