#ifndef INPUT_CONSTANTS_H
#define INPUT_CONSTANTS_H

// Input state constants
#define NEBU_INPUT_KEYSTATE_DOWN 1
#define NEBU_INPUT_KEYSTATE_UP 0

// System key constants
#define SYSTEM_KEY_ESCAPE 27
#define SYSTEM_KEY_ENTER 13
#define SYSTEM_KEY_SPACE 32

// OpenGL function declarations for compatibility
void glColor4f(float r, float g, float b, float a);
void nebu_Font_RenderToBox(void* font, const char* text, int flags, void* box, int align);
void nebu_System_SwapBuffers(void);
void nebu_System_PostRedisplay(void);
void nebu_debug_memory_CheckLeaksOnExit(void);

#endif /* INPUT_CONSTANTS_H */

