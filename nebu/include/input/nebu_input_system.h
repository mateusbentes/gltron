
#ifndef NEBU_INPUT_SYSTEM_H
#define NEBU_INPUT_SYSTEM_H

#include "base/nebu_system.h"
#include <SDL2/SDL.h>

/* Key definitions */
#define SYSTEM_KEY_DOWN SDLK_DOWN
#define SYSTEM_KEY_UP SDLK_UP
#define SYSTEM_KEY_LEFT SDLK_LEFT
#define SYSTEM_KEY_RIGHT SDLK_RIGHT
#define SYSTEM_KEY_SHIFT_LEFT SDLK_LSHIFT
#define SYSTEM_KEY_SHIFT_RIGHT SDLK_RSHIFT
#define SYSTEM_KEY_CTRL_LEFT SDLK_LCTRL
#define SYSTEM_KEY_CTRL_RIGHT SDLK_RCTRL

#define SYSTEM_KEY_F1 SDLK_F1
#define SYSTEM_KEY_F2 SDLK_F2
#define SYSTEM_KEY_F3 SDLK_F3
#define SYSTEM_KEY_F4 SDLK_F4
#define SYSTEM_KEY_F5 SDLK_F5
#define SYSTEM_KEY_F6 SDLK_F6
#define SYSTEM_KEY_F7 SDLK_F7
#define SYSTEM_KEY_F8 SDLK_F8
#define SYSTEM_KEY_F9 SDLK_F9
#define SYSTEM_KEY_F10 SDLK_F10
#define SYSTEM_KEY_F11 SDLK_F11
#define SYSTEM_KEY_F12 SDLK_F12
#define SYSTEM_KEY_ALT_LEFT SDLK_LALT
#define SYSTEM_KEY_ALT_RIGHT SDLK_RALT
#define SYSTEM_KEY_CAPS_LOCK SDLK_CAPSLOCK

#define SYSTEM_KEY_ESCAPE 27
#define SYSTEM_KEY_ENTER SDLK_ENTER
#define SYSTEM_KEY_RETURN SDLK_RETURN
#define SYSTEM_KEY_TAB SDLK_TAB

/* Mouse definitions */
#define SYSTEM_MOUSEUP SDL_MOUSEBUTTONUP
#define SYSTEM_MOUSEDOWN SDL_MOUSEBUTTONDOWN

#define SYSTEM_MOUSEPRESSED SDL_PRESSED
#define SYSTEM_MOUSERELEASED SDL_RELEASED

#define SYSTEM_MOUSEBUTTON_LEFT SDL_BUTTON_LEFT
#define SYSTEM_MOUSEBUTTON_RIGHT SDL_BUTTON_RIGHT

/* Joystick definitions */
#define SYSTEM_JOY_AXIS_MAX 32767

/* Joystick constants */
#define SYSTEM_JOY_OFFSET 100
#define SYSTEM_CUSTOM_KEYS 512

#define SYSTEM_JOY_LEFT 300
#define SYSTEM_JOY_RIGHT 301
#define SYSTEM_JOY_UP 302
#define SYSTEM_JOY_DOWN 303
#define SYSTEM_JOY_BUTTON_0 310
#define SYSTEM_JOY_BUTTON_1 311
#define SYSTEM_JOY_BUTTON_2 312
#define SYSTEM_JOY_BUTTON_3 313
#define SYSTEM_JOY_BUTTON_4 314
#define SYSTEM_JOY_BUTTON_5 315
#define SYSTEM_JOY_BUTTON_6 316
#define SYSTEM_JOY_BUTTON_7 317
#define SYSTEM_JOY_BUTTON_8 318
#define SYSTEM_JOY_BUTTON_9 319
#define SYSTEM_JOY_BUTTON_10 320
#define SYSTEM_JOY_BUTTON_11 321
#define SYSTEM_JOY_BUTTON_12 322
#define SYSTEM_JOY_BUTTON_13 323
#define SYSTEM_JOY_BUTTON_14 324
#define SYSTEM_JOY_BUTTON_15 325
#define SYSTEM_JOY_BUTTON_16 326
#define SYSTEM_JOY_BUTTON_17 327
#define SYSTEM_JOY_BUTTON_18 328
#define SYSTEM_JOY_BUTTON_19 329

/* Input states */
#define NEBU_INPUT_KEYSTATE_DOWN 1
#define NEBU_INPUT_KEYSTATE_UP 0

/* Touch control modes */
#define TOUCH_MODE_SWIPE 0
#define TOUCH_MODE_VIRTUAL_DPAD 1
#define TOUCH_MODE_SCREEN_REGIONS 2

/* Input functions */
void nebu_Input_Init(void);
void nebu_Input_Grab(void);
void nebu_Input_Ungrab(void);
void nebu_Input_HidePointer(void);
void nebu_Input_UnhidePointer(void);
int nebu_Input_GetKeyState(int key);
void nebu_Input_Mouse_GetDelta(int *x, int *y);
void nebu_Input_Mouse_WarpToOrigin(void);
const char* nebu_Input_GetKeyname(int key);

/* Mouse functions */
void SystemMouse(int buttons, int state, int x, int y);
void SystemMouseMotion(int x, int y);

/* Joystick functions */
void SystemSetJoyThreshold(float f);

/* Touch input functions */
void nebu_Input_SetTouchMode(int mode);
int nebu_Input_GetTouchMode(void);
void nebu_Input_SetTouchSwipeThreshold(int threshold);

#endif
