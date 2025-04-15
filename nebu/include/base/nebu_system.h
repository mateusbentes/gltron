#ifndef NEBU_SYSTEM_H
#define NEBU_SYSTEM_H

#include <SDL2/SDL.h>

/* System initialization and shutdown */
void nebu_Init(void);
void nebu_System_Exit(void);

/* Callback registration functions */
void nebu_System_SetCallback_Display(void (*callback)(void));
void nebu_System_SetCallback_Idle(void (*callback)(void));
void nebu_System_SetCallback_Key(void (*callback)(int state, int key, int x, int y));
void nebu_System_SetCallback_Mouse(void (*callback)(int button, int state, int x, int y));
void nebu_System_SetCallback_MouseMotion(void (*callback)(int x, int y));
void nebu_System_SetCallback_SystemEvent(void (*callback)(void *event));

/* Callback invocation functions */
void nebu_System_Display(void);
void nebu_System_Idle(void);
void nebu_System_Key(int state, int key, int x, int y);
void nebu_System_Mouse(int button, int state, int x, int y);
void nebu_System_MouseMotion(int x, int y);
void nebu_System_SystemEvent(void *event);

/* Main loop control */
int nebu_System_MainLoop(void);
void nebu_System_ExitLoop(int return_code);
void nebu_System_PostRedisplay(void);

/* Time functions */
unsigned int nebu_Time_GetElapsed(void);
unsigned int nebu_Time_GetElapsedSinceLastFrame(void);
void nebu_Time_SetCurrentFrameTime(unsigned t);
int nebu_Time_GetTimeForLastFrame(void);
void nebu_Time_FrameDelay(unsigned int delay);

/* System functions */
void nebu_System_SwapBuffers(void);
void nebu_System_Sleep(int ms);

/* Internal functions, not for public use */
void nebu_Intern_HandleInput(SDL_Event *event);

#endif /* NEBU_SYSTEM_H */
