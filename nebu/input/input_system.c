/* 
 * Input system implementation
 * Handles keyboard, mouse, joystick, and touch input
 */

/* Include base/nebu_system.h first to ensure its declaration is seen first */
#include "base/nebu_system.h"

/* Then include other headers */
#include "input/nebu_input_system.h"
#include "input/nebu_system_keynames.h"
#include "video/nebu_video_system.h"
#include "scripting/nebu_scripting.h"

/* Include the KeySet structure definition */
#include "input/nebu_system_keynames.h"

#include <SDL2/SDL.h>
#include <errno.h>

#include "base/nebu_debug_memory.h"

/* Define the input handler structure */
typedef struct {
    void (*keyboard)(int state, int key, int x, int y);
    void (*mouse)(int buttons, int state, int x, int y);
    void (*mouseMotion)(int x, int y);
} InputHandler;

/* Define the current input handler */
static InputHandler *current = NULL;

static float joystick_threshold = 0;
static int mouse_x = -1;
static int mouse_y = -1;
static int mouse_rel_x = 0;
static int mouse_rel_y = 0;

/* Touch input variables */
static int touch_active = 0;
static int touch_start_x = 0;
static int touch_start_y = 0;
static int touch_current_x = 0;
static int touch_current_y = 0;
static int touch_swipe_threshold = 20; /* Minimum pixels for swipe detection */
static int touch_tap_threshold = 10;   /* Maximum movement for tap detection */
static Uint32 touch_start_time = 0;    /* For timing taps */
static Uint32 touch_tap_time = 300;    /* Maximum ms for tap detection */

/* Touch control mode */
typedef enum {
    TOUCH_MODE_SWIPE_ENUM = 0,    /* Swipe to change direction */
    TOUCH_MODE_VIRTUAL_DPAD_ENUM, /* Virtual directional pad */
    TOUCH_MODE_SCREEN_REGIONS_ENUM /* Touch different screen regions for directions */
} TouchControlMode;

static TouchControlMode touch_mode = TOUCH_MODE_SWIPE_ENUM;

enum { eMaxKeyState = 1024 };
static int keyState[eMaxKeyState];

static void setKeyState(int key, int state)
{
	if(key < eMaxKeyState)
		keyState[key] = state;
}

void nebu_Input_Init(void) {
	int i;

	/* keyboard */
	//SDL_EnableKeyRepeat(0, 0); /* turn keyrepeat off */
  
	/* joystick */
	if(SDL_Init(SDL_INIT_JOYSTICK) >= 0) {
		SDL_Joystick *joy;
		int joysticks = SDL_NumJoysticks();
		
		/* FIXME: why only two joysticks? */
		/* joystick, currently at most 2 */
		int max_joy=2; /* default... override by setting NEBU_MAX_JOY */
		char *NEBU_MAX_JOY=getenv("NEBU_MAX_JOY");
		
		if(NEBU_MAX_JOY)
		{
			int n;
			char *endptr;
			errno=0;
			n=strtol(NEBU_MAX_JOY, &endptr, 10);
			if(n<0)
				n=0;
			if(n>4)
				n=4; /* this is the max we can handle! */
			if(!*endptr && !errno)
				max_joy=n;
		}
		
		if(joysticks > max_joy)
			joysticks = max_joy;
		
		for(i = 0; i < joysticks; i++) {
			joy = SDL_JoystickOpen(i);
		}
		if(i)
			SDL_JoystickEventState(SDL_ENABLE);
	} else {
		const char *s = SDL_GetError();
		fprintf(stderr, "[init] couldn't initialize joysticks: %s\n", s);
	}
	for(i = 0; i < eMaxKeyState; i++)
	{
		keyState[i] = NEBU_INPUT_KEYSTATE_UP;
	}
    
    /* Initialize touch input */
    touch_active = 0;
    
    /* Enable touch events */
    SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "1");
}

void nebu_Input_Grab(void) {
	//SDL_WM_GrabInput(SDL_GRAB_ON);
}

void nebu_Input_Ungrab(void) {
	//SDL_WM_GrabInput(SDL_GRAB_OFF);
}

void nebu_Input_HidePointer(void) {
	SDL_ShowCursor(SDL_DISABLE);
}

void nebu_Input_UnhidePointer(void) {
	SDL_ShowCursor(SDL_ENABLE);
}

/* Function to set the current input handler */
void nebu_Input_SetHandler(void *handler) {
    current = (InputHandler*)handler;
}

/* Function to create a new input handler */
void* nebu_Input_CreateHandler(
    void (*keyboard)(int state, int key, int x, int y),
    void (*mouse)(int buttons, int state, int x, int y),
    void (*mouseMotion)(int x, int y)
) {
    InputHandler *handler = (InputHandler*)malloc(sizeof(InputHandler));
    if (!handler) {
        fprintf(stderr, "[input] Failed to allocate memory for input handler\n");
        return NULL;
    }
    
    handler->keyboard = keyboard;
    handler->mouse = mouse;
    handler->mouseMotion = mouseMotion;
    
    return handler;
}

/* Function to destroy an input handler */
void nebu_Input_DestroyHandler(void *handler) {
    if (handler) {
        free(handler);
    }
}

void SystemMouse(int buttons, int state, int x, int y) {
	if(current && current->mouse)
		current->mouse(buttons, state, x, y);
}

int nebu_Input_GetKeyState(int key)
{
	if(key > eMaxKeyState)
		return NEBU_INPUT_KEYSTATE_UP;
	else
		return keyState[key];
}

void nebu_Input_Mouse_GetDelta(int *x, int *y)
{
	*x = mouse_rel_x;
	*y = mouse_rel_y;
}

void nebu_Input_Mouse_WarpToOrigin(void)
{
	mouse_rel_x = 0;
	mouse_rel_y = 0;
}

void SystemMouseMotion(int x, int y) {
	// save mouse position
	// printf("[input] mouse motion to %d, %d\n", x, y);
	mouse_x = x;
	mouse_y = y;
	if(current && current->mouseMotion)
		current->mouseMotion(x, y);
}

const char* nebu_Input_GetKeyname(int key) {
	if(key < SYSTEM_CUSTOM_KEYS)
		return SDL_GetKeyName(key);
	else {
		int i;
		
		for(i = 0; i < CUSTOM_KEY_COUNT; i++) {
			if(custom_keys.key[i].key == key)
				return custom_keys.key[i].name;
		}
		return "unknown custom key";
	}
}

/* Include the external declaration for custom_keys */
extern custom_keynames custom_keys;

/* Process touch input based on the current touch mode */
static void processTouchInput(int x, int y, int is_down) {
    int screen_width, screen_height;
    int dx, dy;
    int direction_key = 0;
    
    nebu_Video_GetDimension(&screen_width, &screen_height);
    
    if (is_down) {
        /* Touch started */
        touch_active = 1;
        touch_start_x = x;
        touch_start_y = y;
        touch_current_x = x;
        touch_current_y = y;
        touch_start_time = SDL_GetTicks();
    } else {
        /* Touch ended */
        if (!touch_active) return;
        
        touch_active = 0;
        touch_current_x = x;
        touch_current_y = y;
        
        dx = touch_current_x - touch_start_x;
        dy = touch_current_y - touch_start_y;
        
        /* Check if this was a tap (quick touch with minimal movement) */
        if (SDL_GetTicks() - touch_start_time < touch_tap_time &&
            abs(dx) < touch_tap_threshold && abs(dy) < touch_tap_threshold) {
            /* Handle tap based on screen region */
            if (touch_mode == TOUCH_MODE_SCREEN_REGIONS_ENUM) {
                /* Divide screen into regions and determine direction */
                if (x < screen_width / 3) {
                    /* Left third of screen - turn left */
                    direction_key = SDLK_LEFT;
                } else if (x > 2 * screen_width / 3) {
                    /* Right third of screen - turn right */
                    direction_key = SDLK_RIGHT;
                } else if (y < screen_height / 2) {
                    /* Top middle - accelerate */
                    direction_key = SDLK_UP;
                } else {
                    /* Bottom middle - brake/slow down */
                    direction_key = SDLK_DOWN;
                }
                
                /* Send key press and release events */
                if (direction_key) {
                    if (current && current->keyboard) {
                        current->keyboard(NEBU_INPUT_KEYSTATE_DOWN, direction_key, 0, 0);
                        current->keyboard(NEBU_INPUT_KEYSTATE_UP, direction_key, 0, 0);
                    }
                }
            }
            return;
        }
        
        /* Process swipe if movement exceeds threshold */
        if (touch_mode == TOUCH_MODE_SWIPE_ENUM && 
            (abs(dx) > touch_swipe_threshold || abs(dy) > touch_swipe_threshold)) {
            
            /* Determine primary direction of swipe */
            if (abs(dx) > abs(dy)) {
                /* Horizontal swipe */
                direction_key = (dx > 0) ? SDLK_RIGHT : SDLK_LEFT;
            } else {
                /* Vertical swipe */
                direction_key = (dy > 0) ? SDLK_DOWN : SDLK_UP;
            }
            
            /* Send key press and release events */
            if (current && current->keyboard) {
                current->keyboard(NEBU_INPUT_KEYSTATE_DOWN, direction_key, 0, 0);
                current->keyboard(NEBU_INPUT_KEYSTATE_UP, direction_key, 0, 0);
            }
        }
    }
}

/* Process touch motion for virtual d-pad mode */
static void processTouchMotion(int x, int y) {
    if (!touch_active) return;
    
    touch_current_x = x;
    touch_current_y = y;
    
    if (touch_mode == TOUCH_MODE_VIRTUAL_DPAD_ENUM) {
        int dx = touch_current_x - touch_start_x;
        int dy = touch_current_y - touch_start_y;
        int direction_key = 0;
        
        /* Clear all direction keys first */
        setKeyState(SDLK_LEFT, NEBU_INPUT_KEYSTATE_UP);
        setKeyState(SDLK_RIGHT, NEBU_INPUT_KEYSTATE_UP);
        setKeyState(SDLK_UP, NEBU_INPUT_KEYSTATE_UP);
        setKeyState(SDLK_DOWN, NEBU_INPUT_KEYSTATE_UP);
        
        /* Determine primary direction based on distance from start point */
        if (abs(dx) > touch_swipe_threshold || abs(dy) > touch_swipe_threshold) {
            if (abs(dx) > abs(dy)) {
                /* Horizontal movement */
                direction_key = (dx > 0) ? SDLK_RIGHT : SDLK_LEFT;
            } else {
                /* Vertical movement */
                direction_key = (dy > 0) ? SDLK_DOWN : SDLK_UP;
            }
            
            /* Set the key state */
            setKeyState(direction_key, NEBU_INPUT_KEYSTATE_DOWN);
            
            /* Send key press event */
            if (current && current->keyboard) {
                current->keyboard(NEBU_INPUT_KEYSTATE_DOWN, direction_key, 0, 0);
            }
        }
    }
}

/* Implementation of the function declared in nebu_system.h */
void nebu_Intern_HandleInput(SDL_Event *event) {
	const char *keyname;
	int key, state;
	// int skip_axis_event = 0;
	static int joy_axis_state[2] = { 0, 0 };
	static int joy_lastaxis[2] = { 0, 0 };

	switch(event->type) {
	case SDL_KEYDOWN:
	case SDL_KEYUP:
		if(event->type == SDL_KEYDOWN) {
			state = NEBU_INPUT_KEYSTATE_DOWN;
		} else {
			state = NEBU_INPUT_KEYSTATE_UP;
		}
 
		keyname = SDL_GetKeyName(event->key.keysym.sym);
		key = 0;
		switch(event->key.keysym.sym) {
		case SDLK_SPACE: key = ' '; break;
		case SDLK_ESCAPE: key = 27; break;
		case SDLK_RETURN: key = 13; break;
		default:
			if(keyname[1] == 0) key = keyname[0];
			break;
		}
		/* check: is that translation necessary? */
		setKeyState(key, state);
		if(current && current->keyboard)
			current->keyboard(state, key ? key : event->key.keysym.sym, 0, 0);
		break;
	case SDL_JOYAXISMOTION:
		if( abs(event->jaxis.value) <= joystick_threshold * SYSTEM_JOY_AXIS_MAX) {
			// axis returned to origin, only generate event if it was set before
			if(joy_axis_state[event->jaxis.which] & (1 << event->jaxis.axis)) {
				joy_axis_state[event->jaxis.which] &= ~ 
					(1 << event->jaxis.axis); // clear axis
				key = SYSTEM_JOY_LEFT + event->jaxis.which * SYSTEM_JOY_OFFSET;
				if(event->jaxis.axis == 1) {
					key += 2;
				}
				if(joy_lastaxis[event->jaxis.which] & (1 << event->jaxis.axis)) {
					key++;
				}
				setKeyState(key, NEBU_INPUT_KEYSTATE_UP);
				if(current && current->keyboard)
					current->keyboard(NEBU_INPUT_KEYSTATE_UP, key, 0, 0);
			} else {
				// do nothing
			}
		} else {
			// axis set, only generate event if it wasn't set before
			if(! (joy_axis_state[event->jaxis.which] & (1 << event->jaxis.axis)) ) {
				joy_axis_state[event->jaxis.which] |= (1 << event->jaxis.axis);
				key = SYSTEM_JOY_LEFT + event->jaxis.which * SYSTEM_JOY_OFFSET;
				if(event->jaxis.axis == 1) {
					key += 2;
				}
				if(event->jaxis.value > 0) {
					key++;
					joy_lastaxis[event->jaxis.which] |= (1 << event->jaxis.axis);
				} else {
					joy_lastaxis[event->jaxis.which] &= ~(1 << event->jaxis.axis);
				}
				setKeyState(key, NEBU_INPUT_KEYSTATE_DOWN);
				if(current && current->keyboard)
					current->keyboard(NEBU_INPUT_KEYSTATE_DOWN, key, 0, 0);
			} else {
				// do nothing
			}
		}
		break;
				 
#if 0
		if (abs(event->jaxis.value) <= joystick_threshold * SYSTEM_JOY_AXIS_MAX) {
			skip_axis_event &= ~(1 << event->jaxis.axis);
			break;
		}
		if(skip_axis_event & (1 << event->jaxis.axis))
			break;
		skip_axis_event |= 1 << event->jaxis.axis;
		key = SYSTEM_JOY_LEFT + event->jaxis.which * SYSTEM_JOY_OFFSET;
		if(event->jaxis.axis == 1)
			key += 2;
		if(event->jaxis.value > 0)
			key++;
		setKeyState(key, NEBU_INPUT_KEYSTATE_DOWN);
		if(current && current->keyboard)
			current->keyboard(NEBU_INPUT_KEYSTATE_DOWN, key, 0, 0);
		break;
#endif
	case SDL_JOYBUTTONDOWN:
	case SDL_JOYBUTTONUP:
		if(event->type == SDL_JOYBUTTONDOWN)
			state = NEBU_INPUT_KEYSTATE_DOWN;
		else
			state = NEBU_INPUT_KEYSTATE_UP;
		
		key = SYSTEM_JOY_BUTTON_0 + event->jbutton.button +
			SYSTEM_JOY_OFFSET * event->jbutton.which;
		setKeyState(key, state);
		if(current && current->keyboard)
			current->keyboard(state, key, 0, 0);
		break;
	case SDL_MOUSEBUTTONDOWN:
	case SDL_MOUSEBUTTONUP:
		/* Handle as touch input on all platforms */
		processTouchInput(event->button.x, event->button.y, 
                         event->type == SDL_MOUSEBUTTONDOWN);
		
		SystemMouse(event->button.button, event->button.state, 
			event->button.x, event->button.y);
		break;
	case SDL_MOUSEMOTION:
		/* Handle as touch motion on all platforms */
		processTouchMotion(event->motion.x, event->motion.y);
		
		SystemMouseMotion(event->motion.x, event->motion.y);
		mouse_rel_x += event->motion.xrel;
		mouse_rel_y += event->motion.yrel;
		break;
		
	/* Handle direct touch events */
	case SDL_FINGERDOWN:
		{
			int x, y;
			int screen_width, screen_height;
			
			nebu_Video_GetDimension(&screen_width, &screen_height);
			x = (int)(event->tfinger.x * screen_width);
			y = (int)(event->tfinger.y * screen_height);
			
			processTouchInput(x, y, 1);
		}
		break;
	case SDL_FINGERUP:
		{
			int x, y;
			int screen_width, screen_height;
			
			nebu_Video_GetDimension(&screen_width, &screen_height);
			x = (int)(event->tfinger.x * screen_width);
			y = (int)(event->tfinger.y * screen_height);
			
			processTouchInput(x, y, 0);
		}
		break;
	case SDL_FINGERMOTION:
		{
			int x, y;
			int screen_width, screen_height;
			
			nebu_Video_GetDimension(&screen_width, &screen_height);
			x = (int)(event->tfinger.x * screen_width);
			y = (int)(event->tfinger.y * screen_height);
			
			processTouchMotion(x, y);
		}
		break;
	}
}

void SystemSetJoyThreshold(float f) { 
	joystick_threshold = f;
}

/* Set the touch control mode */
void nebu_Input_SetTouchMode(int mode) {
    if (mode >= 0 && mode <= 2) {
        touch_mode = (TouchControlMode)mode;
    }
}

/* Get the current touch control mode */
int nebu_Input_GetTouchMode(void) {
    return (int)touch_mode;
}

/* Set the touch swipe threshold */
void nebu_Input_SetTouchSwipeThreshold(int threshold) {
    if (threshold > 0) {
        touch_swipe_threshold = threshold;
    }
}

/* Quit the input system */
void nebu_Input_Quit(void) {
    /* Free the current input handler if it exists */
    if (current) {
        free(current);
        current = NULL;
    }
    
    /* Close all joysticks */
    SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
}
