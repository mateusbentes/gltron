#include "configuration/settings.h"
#include "configuration/platform_settings.h"

void platform_InitSettings(void) {
#if defined(ANDROID) || defined(__ANDROID__)
    // Set fullscreen mode for Android
    setSettingi("windowMode", 0);  // 0 = fullscreen
    
    // Get the screen resolution and set it as the default
    int width, height;
    nebu_Video_GetScreenSize(&width, &height);
    if(width > 0 && height > 0) {
        setSettingi("width", width);
        setSettingi("height", height);
    } else {
        // Fallback to a reasonable default for mobile
        setSettingi("width", 1280);
        setSettingi("height", 720);
    }
    
    // Other Android-specific settings
    setSettingi("mouse_warp", 0);  // No mouse warping on touch devices
    setSettingi("touch_mode", 0);  // Default touch mode (swipe)
#endif
}