#include "configuration/touch_settings.h"
#include "configuration/settings.h"
#include "input/nebu_input_system.h"
#include "scripting/nebu_scripting.h"

/* Initialize touch settings from Lua configuration */
void initTouchSettings(void) {
    int touch_mode = 0;
    int swipe_threshold = 20;
    
    /* Try to load settings from Lua */
    if (!scripting_GetGlobal("settings", "touch_mode", NULL)) {
        scripting_GetIntegerResult(&touch_mode);
    }
    
    if (!scripting_GetGlobal("settings", "touch_swipe_threshold", NULL)) {
        scripting_GetIntegerResult(&swipe_threshold);
    }
    
    /* Apply settings */
    nebu_Input_SetTouchMode(touch_mode);
    nebu_Input_SetTouchSwipeThreshold(swipe_threshold);
}

/* Save touch settings to Lua configuration */
void saveTouchSettings(void) {
    int touch_mode = nebu_Input_GetTouchMode();
    
    /* Save settings to Lua */
    scripting_SetInteger(touch_mode, "touch_mode", "settings", NULL);
    
    /* Save swipe threshold - we don't have a getter for this, so we'll use a default value */
    scripting_SetInteger(20, "touch_swipe_threshold", "settings", NULL);
}

/* Get the name of the current touch mode */
const char* getTouchModeName(int mode) {
    switch(mode) {
        case TOUCH_MODE_SWIPE:
            return "Swipe";
        case TOUCH_MODE_VIRTUAL_DPAD:
            return "Virtual D-Pad";
        case TOUCH_MODE_SCREEN_REGIONS:
            return "Screen Regions";
        default:
            return "Unknown";
    }
}

/* Cycle to the next touch mode */
void cycleTouchMode(void) {
    int current_mode = nebu_Input_GetTouchMode();
    int next_mode = (current_mode + 1) % 3; /* 3 modes total */
    
    nebu_Input_SetTouchMode(next_mode);
    
    /* Save the new setting */
    saveTouchSettings();
}