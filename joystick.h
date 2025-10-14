#ifndef JOYSTICK_H
#define JOYSTICK_H

/* Joystick/Gamepad support for GLTron */
/* Supports Xbox, PlayStation, and Steam Deck controllers */

/* Button mappings for standard gamepad layout */
typedef enum {
    JOY_BUTTON_A = 0,        /* Cross on PS, A on Xbox/Steam */
    JOY_BUTTON_B = 1,        /* Circle on PS, B on Xbox/Steam */
    JOY_BUTTON_X = 2,        /* Square on PS, X on Xbox/Steam */
    JOY_BUTTON_Y = 3,        /* Triangle on PS, Y on Xbox/Steam */
    JOY_BUTTON_L1 = 4,       /* L1/LB */
    JOY_BUTTON_R1 = 5,       /* R1/RB */
    JOY_BUTTON_SELECT = 6,   /* Select/Back/View */
    JOY_BUTTON_START = 7,    /* Start/Menu */
    JOY_BUTTON_L3 = 8,       /* Left stick click */
    JOY_BUTTON_R3 = 9,       /* Right stick click */
    JOY_BUTTON_HOME = 10,    /* Home/Guide/Steam button */
    JOY_BUTTON_DPAD_UP = 11,
    JOY_BUTTON_DPAD_DOWN = 12,
    JOY_BUTTON_DPAD_LEFT = 13,
    JOY_BUTTON_DPAD_RIGHT = 14,
    JOY_BUTTON_MAX = 15
} JoyButton;

/* Axis mappings */
typedef enum {
    JOY_AXIS_LEFT_X = 0,     /* Left stick horizontal */
    JOY_AXIS_LEFT_Y = 1,     /* Left stick vertical */
    JOY_AXIS_RIGHT_X = 2,    /* Right stick horizontal */
    JOY_AXIS_RIGHT_Y = 3,    /* Right stick vertical */
    JOY_AXIS_L2 = 4,         /* L2/LT trigger */
    JOY_AXIS_R2 = 5,         /* R2/RT trigger */
    JOY_AXIS_MAX = 6
} JoyAxis;

/* Joystick state */
typedef struct {
    int connected;
    int num_buttons;
    int num_axes;
    int buttons[JOY_BUTTON_MAX];
    float axes[JOY_AXIS_MAX];
    float deadzone;
    int joystick_id;
    char name[256];
} JoystickState;

/* Global joystick state */
extern JoystickState joystick;

/* Function prototypes */
int initJoystick(void);
void updateJoystick(void);
void closeJoystick(void);
void processJoystickGame(void);
void processJoystickMenu(void);
void processJoystickPause(void);
int isJoystickConnected(void);
void setJoystickDeadzone(float deadzone);

/* Helper functions */
int getJoystickButton(JoyButton button);
float getJoystickAxis(JoyAxis axis);
int isJoystickButtonPressed(JoyButton button);
int isJoystickButtonReleased(JoyButton button);

#endif /* JOYSTICK_H */
