#ifndef INPUT_H
#define INPUT_H

#include <SDL2/SDL.h> // For SDL_Renderer and SDL_Event

// Function prototypes
void initInput(void);
void shutdownInput(void);
void nebu_Input_GetMousePosition(int* x, int* y);
int nebu_Input_GetMouseButtonState(int button);
void nebu_Input_HandleEvents(void);

typedef struct Input {
  int mouse1;
  int mouse2;
} Input;

#define MOUSE_ORIG_X 100
#define MOUSE_ORIG_Y 100
#define MOUSE_CX 0.003f
#define MOUSE_CY 0.003f

// Key constants
#define SYSTEM_KEYPRESS 1
#define SYSTEM_KEYRELEASE 0

#define SYSTEM_KEY_UP 273
#define SYSTEM_KEY_DOWN 274
#define SYSTEM_KEY_LEFT 276
#define SYSTEM_KEY_RIGHT 275
#define SYSTEM_KEY_SPACE 32
#define SYSTEM_KEY_RETURN 13

#define SYSTEM_MOUSEPRESSED 1
#define SYSTEM_MOUSERELEASED 0
#define SYSTEM_MOUSEBUTTON_LEFT 1
#define SYSTEM_MOUSEBUTTON_RIGHT 3

typedef enum ReservedKeys {
  eEscape = 0,
  eSpace,
  eF1,
  eF2,
  eF3,
  eF4,
  eF5,
  eF6,
  eF7,
  eF8,
  eF9,
  eF10,
  eF11,
  eF12,
  eAltLeft,
  eReservedKeys,
} ReservedKeys;

extern int ReservedKeyCodes[eReservedKeys];

// Game input functions
void keyGame(SDL_KeyboardEvent *event);
void gameMouse(SDL_MouseButtonEvent *event);

// Menu input functions (modern SDL2 event-based)
void keyMenu(SDL_KeyboardEvent *event);
void mouseMenu(SDL_MouseButtonEvent *event);

// Touch input functions (modernized for SDL2)
void inputTouchGame(int state, int x, int y, int screenWidth, int screenHeight);

// Modernized overlay drawing function using SDL_Renderer
void inputDrawTouchControls(SDL_Renderer *renderer, int screenWidth, int screenHeight);

// Modernized SDL2 event handler for input
void inputHandleSDLEvent(const SDL_Event *event, int screenWidth, int screenHeight);

// Optional: Touch region and state types for external use
typedef enum {
    TOUCH_REGION_NONE = 0,
    TOUCH_REGION_LEFT,
    TOUCH_REGION_RIGHT,
    TOUCH_REGION_UP,
    TOUCH_REGION_DOWN,
    TOUCH_REGION_BOOST
} TouchRegion;

typedef struct {
    int active;
    int x, y;
    TouchRegion region;
} TouchState;

void Input_Idle();

extern int joy_threshold;
extern Input gInput;

#endif
