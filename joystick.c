#include "gltron.h"
#include "joystick.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <GL/freeglut.h>

/* Global joystick state */
JoystickState joystick;
static JoystickState prev_joystick;  /* For detecting button press/release */

/* External function declarations */
extern int isSteamDeckController(const char* name);
extern void configureSteamDeckControls(void);

/* Initialize joystick system */
int initJoystick(void) {
    memset(&joystick, 0, sizeof(JoystickState));
    memset(&prev_joystick, 0, sizeof(JoystickState));
    
    joystick.deadzone = 0.15f;  /* 15% deadzone by default */
    joystick.joystick_id = 0;  /* Use first joystick (index 0) */
    
    /* Safety check - make sure GLUT is initialized */
    if (!glutGet(GLUT_INIT_STATE)) {
        printf("Warning: GLUT not initialized, cannot check for joystick\n");
        return 0;
    }
    
    /* Check if we have a window - if not, that's OK, we'll check joystick anyway */
    int current_window = glutGetWindow();
    if (current_window <= 0) {
        printf("Note: No GLUT window active during joystick check\n");
    }
    
    /* Check if joystick is available using FreeGLUT API */
    /* FreeGLUT uses joystick index, not GLUT_JOYSTICK_1 constant */
    int num_buttons = 0;
    int num_axes = 0;
    
    /* Try to get joystick info with error checking */
    printf("Checking for joystick at index %d...\n", joystick.joystick_id);
    
    /* First, try to detect if joystick functions are available */
    /* by checking if we can get device info without crashing */
    
    /* Use glutDeviceGet to check if joystick is present */
    int has_joystick = glutDeviceGet(GLUT_HAS_JOYSTICK);
    if (!has_joystick) {
        printf("GLUT reports no joystick support\n");
        return 0;
    }
    
    num_buttons = glutJoystickGetNumButtons(joystick.joystick_id);
    printf("Joystick buttons query returned: %d\n", num_buttons);
    
    if (num_buttons <= 0) {
        printf("No joystick detected at index %d (buttons=%d)\n", joystick.joystick_id, num_buttons);
        return 0;
    }
    
    num_axes = glutJoystickGetNumAxes(joystick.joystick_id);
    printf("Joystick axes query returned: %d\n", num_axes);
    
    if (num_axes > 0 && num_buttons > 0) {
        joystick.connected = 1;
        joystick.num_axes = (num_axes > JOY_AXIS_MAX) ? JOY_AXIS_MAX : num_axes;
        joystick.num_buttons = (num_buttons > JOY_BUTTON_MAX) ? JOY_BUTTON_MAX : num_buttons;
        
        printf("Joystick detected: %d axes, %d buttons\n", 
               joystick.num_axes, joystick.num_buttons);
        
        /* Try to detect controller type by button/axis count */
        if (num_axes >= 4 && num_buttons >= 11) {
            printf("Detected standard gamepad\n");
            if (num_buttons >= 16) {
                printf("Full featured controller detected (Xbox/PS/Steam Deck compatible)\n");
                /* Apply Steam Deck optimizations */
                joystick.deadzone = 0.20f;  /* Slightly higher deadzone for Steam Deck */
            }
        }
        
        /* Enable joystick polling */
        /* Just register the callback, don't try to access game structure yet */
        glutJoystickFunc(updateJoystick, 20);  /* Poll every 20ms */
        
        printf("Joystick initialized successfully\n");
        return 1;
    }
    
    printf("No joystick detected\n");
    return 0;
}

/* Update joystick state - called by GLUT callback */
void updateJoystick(unsigned int buttonMask, int x, int y, int z) {
    if (!joystick.connected) return;
    
    /* Save previous state for edge detection */
    memcpy(&prev_joystick, &joystick, sizeof(JoystickState));
    
    /* Update button states from button mask */
    for (int i = 0; i < joystick.num_buttons && i < 32; i++) {
        joystick.buttons[i] = (buttonMask & (1 << i)) ? 1 : 0;
    }
    
    /* Update axes - GLUT provides x, y, z in range -1000 to 1000 */
    /* Map to standard gamepad layout with -1.0 to 1.0 range */
    if (joystick.num_axes >= 2) {
        joystick.axes[JOY_AXIS_LEFT_X] = x / 1000.0f;
        joystick.axes[JOY_AXIS_LEFT_Y] = y / 1000.0f;
    }
    if (joystick.num_axes >= 3) {
        joystick.axes[JOY_AXIS_RIGHT_X] = z / 1000.0f;
    }
    
    /* Note: FreeGLUT only provides 3 axes (x, y, z) through the callback
       For more axes, we'd need to use a different API like SDL or read /dev/input directly */
    if (joystick.num_axes >= 4) {
        /* Use z axis for right stick X, and simulate others */
        joystick.axes[JOY_AXIS_RIGHT_Y] = 0.0f;  /* Not available in GLUT */
        joystick.axes[JOY_AXIS_L2] = 0.0f;       /* Not available in GLUT */
        joystick.axes[JOY_AXIS_R2] = 0.0f;       /* Not available in GLUT */
    }
    
    /* Apply deadzone */
    for (int i = 0; i < JOY_AXIS_MAX; i++) {
        if (fabs(joystick.axes[i]) < joystick.deadzone) {
            joystick.axes[i] = 0.0f;
        }
    }
}

/* Process joystick input during gameplay */
void processJoystickGame(void) {
    if (!joystick.connected || !game) return;
    
    /* Only process if joystick mode is active or we're in any input mode */
    if (game->settings->input_mode != 3 && game->settings->input_mode != 0) return;
    
    Data *data = game->player[0].data;
    if (!data || data->speed <= 0) return;
    
    /* Left stick or D-pad for turning */
    float stick_x = joystick.axes[JOY_AXIS_LEFT_X];
    
    /* Turn left/right based on stick */
    if (stick_x < -0.5f || joystick.buttons[JOY_BUTTON_DPAD_LEFT]) {
        turn(data, 3);  /* Turn left */
    } else if (stick_x > 0.5f || joystick.buttons[JOY_BUTTON_DPAD_RIGHT]) {
        turn(data, 1);  /* Turn right */
    }
    
    /* L1/R1 for quick turns */
    if (isJoystickButtonPressed(JOY_BUTTON_L1)) {
        turn(data, 3);  /* Turn left */
    } else if (isJoystickButtonPressed(JOY_BUTTON_R1)) {
        turn(data, 1);  /* Turn right */
    }
    
    /* Start button for pause */
    if (isJoystickButtonPressed(JOY_BUTTON_START)) {
        game->pauseflag = 1;
        switchCallbacks(&pauseCallbacks);
    }
    
    /* L3 (left stick click) to change camera */
    if (isJoystickButtonPressed(JOY_BUTTON_L3)) {
        game->settings->camType = (game->settings->camType + 1) % 3;
        printf("Camera mode: %d\n", game->settings->camType);
    }
    
    /* Right stick for camera rotation (if in appropriate camera mode) */
    if (game->settings->camType == 0) {  /* Circle cam */
        float right_x = joystick.axes[JOY_AXIS_RIGHT_X];
        if (fabs(right_x) > joystick.deadzone) {
            extern float camAngle;
            camAngle += right_x * 0.05f;  /* Adjust rotation speed */
        }
    }
}

/* Process joystick input in menus */
void processJoystickMenu(void) {
    if (!joystick.connected || !pCurrent) return;
    
    /* D-pad or left stick for navigation */
    if (isJoystickButtonPressed(JOY_BUTTON_DPAD_UP) || 
        (joystick.axes[JOY_AXIS_LEFT_Y] < -0.7f && prev_joystick.axes[JOY_AXIS_LEFT_Y] >= -0.7f)) {
        pCurrent->iHighlight = (pCurrent->iHighlight - 1) % pCurrent->nEntries;
        if (pCurrent->iHighlight < 0)
            pCurrent->iHighlight = pCurrent->nEntries - 1;
        playHighlightSound();
    }
    
    if (isJoystickButtonPressed(JOY_BUTTON_DPAD_DOWN) || 
        (joystick.axes[JOY_AXIS_LEFT_Y] > 0.7f && prev_joystick.axes[JOY_AXIS_LEFT_Y] <= 0.7f)) {
        pCurrent->iHighlight = (pCurrent->iHighlight + 1) % pCurrent->nEntries;
        playHighlightSound();
    }
    
    /* A button to select */
    if (isJoystickButtonPressed(JOY_BUTTON_A)) {
        menuAction(*(pCurrent->pEntries + pCurrent->iHighlight));
    }
    
    /* B button to go back */
    if (isJoystickButtonPressed(JOY_BUTTON_B)) {
        if (pCurrent->parent == NULL)
            restoreCallbacks();
        else
            pCurrent = pCurrent->parent;
    }
}

/* Process joystick input during pause */
void processJoystickPause(void) {
    if (!joystick.connected) return;
    
    /* A button or Start to resume */
    if (isJoystickButtonPressed(JOY_BUTTON_A) || 
        isJoystickButtonPressed(JOY_BUTTON_START)) {
        if (game->pauseflag & PAUSE_GAME_FINISHED)
            initData();
        lasttime = getElapsedTime();
        switchCallbacks(&gameCallbacks);
    }
    
    /* B button to return to menu */
    if (isJoystickButtonPressed(JOY_BUTTON_B)) {
        switchCallbacks(&guiCallbacks);
    }
}

/* Helper functions */
int isJoystickConnected(void) {
    return joystick.connected;
}

void setJoystickDeadzone(float deadzone) {
    joystick.deadzone = deadzone;
}

int getJoystickButton(JoyButton button) {
    if (button >= 0 && button < JOY_BUTTON_MAX)
        return joystick.buttons[button];
    return 0;
}

float getJoystickAxis(JoyAxis axis) {
    if (axis >= 0 && axis < JOY_AXIS_MAX)
        return joystick.axes[axis];
    return 0.0f;
}

int isJoystickButtonPressed(JoyButton button) {
    if (button >= 0 && button < JOY_BUTTON_MAX)
        return joystick.buttons[button] && !prev_joystick.buttons[button];
    return 0;
}

int isJoystickButtonReleased(JoyButton button) {
    if (button >= 0 && button < JOY_BUTTON_MAX)
        return !joystick.buttons[button] && prev_joystick.buttons[button];
    return 0;
}

void closeJoystick(void) {
    if (joystick.connected) {
        glutJoystickFunc(NULL, 0);
        joystick.connected = 0;
    }
}
