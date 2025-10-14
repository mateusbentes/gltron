/* Steam Deck specific controller configuration */
#include "joystick.h"
#include <string.h>
#include <stdio.h>

/* Steam Deck controller mapping
 * The Steam Deck controller appears as a standard gamepad in Linux
 * 
 * Buttons:
 * A (Cross) - Confirm/Accelerate
 * B (Circle) - Cancel/Back
 * X (Square) - Action
 * Y (Triangle) - Menu
 * L1 - Turn Left
 * R1 - Turn Right
 * L2 (Analog) - Brake/Slow
 * R2 (Analog) - Boost/Speed
 * Left Stick - Movement/Navigation
 * Right Stick - Camera (if implemented)
 * D-Pad - Menu Navigation / Quick Turn
 * Start - Pause
 * Select - Menu
 * Steam Button - (System reserved)
 * L3 (Left Stick Click) - Change Camera
 * R3 (Right Stick Click) - Reset View
 * L4/L5 (Back paddles) - Custom actions
 * R4/R5 (Back paddles) - Custom actions
 */

/* Check if this is a Steam Deck controller */
int isSteamDeckController(const char* name) {
    if (!name) return 0;
    
    /* Steam Deck controller identifiers */
    if (strstr(name, "Steam") != NULL) return 1;
    if (strstr(name, "Valve") != NULL) return 1;
    if (strstr(name, "Steam Deck") != NULL) return 1;
    if (strstr(name, "SteamDeck") != NULL) return 1;
    
    return 0;
}

/* Configure controls specifically for Steam Deck */
void configureSteamDeckControls(void) {
    printf("Steam Deck controller detected - applying optimal configuration\n");
    
    /* Set a slightly larger deadzone for Steam Deck's sticks */
    joystick.deadzone = 0.20f;
    
    /* Steam Deck has all standard gamepad controls plus extras */
    /* The mapping is handled in processJoystickGame but we can set flags here */
    
    printf("Steam Deck configuration applied:\n");
    printf("  Left Stick / D-Pad: Turn\n");
    printf("  L1/R1: Quick Turn Left/Right\n");
    printf("  A: Confirm\n");
    printf("  B: Back\n");
    printf("  Start: Pause\n");
    printf("  L3: Change Camera View\n");
}
