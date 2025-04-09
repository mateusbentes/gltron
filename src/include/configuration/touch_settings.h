#ifndef TOUCH_SETTINGS_H
#define TOUCH_SETTINGS_H

/* Initialize touch settings from Lua configuration */
void initTouchSettings(void);

/* Save touch settings to Lua configuration */
void saveTouchSettings(void);

/* Get the name of the current touch mode */
const char* getTouchModeName(int mode);

/* Cycle to the next touch mode */
void cycleTouchMode(void);

#endif