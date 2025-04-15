#ifndef INPUT_H
#define INPUT_H

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
void keyGame(int state, int key, int x, int y);
void gameMouse(int buttons, int state, int x, int y);

// Menu input functions
void keyMenu(int state, int key, int x, int y);
void mouseMenu(int button, int state, int x, int y);

// Touch input functions
void touchGame(int state, int x, int y, int screenWidth, int screenHeight);
void drawTouchControls(int screenWidth, int screenHeight);

void Input_Idle();

extern int joy_threshold;
extern Input gInput;
#endif
