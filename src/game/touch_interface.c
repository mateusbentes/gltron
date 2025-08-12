/* Touch input interface for Lua */
#include "game/gltron.h"
#include "input/nebu_input_system.h"
#include "video/nebu_video_system.h"
#include "scripting/nebu_scripting.h"

/* Set touch mode from Lua */
int c_set_touch_mode(lua_State *L) {
    int mode;
    
    if(lua_gettop(L) != 1 || !lua_isnumber(L, 1)) {
        /* Use nebu_assert instead of scripting_Error */
        fprintf(stderr, "invalid parameters to c_set_touch_mode\n");
        return 0;
    }
    
    /* Use lua_tonumber instead of lua_tonumberx for Lua 5 compatibility */
    mode = (int)lua_tonumber(L, 1);
    /* Call the function if it exists, otherwise just return */
#ifdef __ANDROID__
    nebu_Input_SetTouchMode(mode);
#endif
    
    return 0;
}

/* Set touch swipe threshold from Lua */
int c_set_touch_swipe_threshold(lua_State *L) {
    int threshold;
    
    if(lua_gettop(L) != 1 || !lua_isnumber(L, 1)) {
        /* Use nebu_assert instead of scripting_Error */
        fprintf(stderr, "invalid parameters to c_set_touch_swipe_threshold\n");
        return 0;
    }
    
    /* Use lua_tonumber instead of lua_tonumberx for Lua 5 compatibility */
    threshold = (int)lua_tonumber(L, 1);
    /* Call the function if it exists, otherwise just return */
#if defined(__ANDROID__)
    nebu_Input_SetTouchSwipeThreshold(threshold);
#endif
    
    return 0;
}

/* Get screen dimensions for Lua */
int c_get_screen_dimensions(lua_State *L) {
    int width, height;
    
    nebu_Video_GetDimension(&width, &height);
    
    /* Use lua_pushnumber instead of lua_pushinteger for Lua 5 compatibility */
    lua_pushnumber(L, width);
    lua_pushnumber(L, height);
    
    return 2;
}

/* Check if we're on Android */
int c_is_android(lua_State *L) {
#if defined(__ANDROID__)
    lua_pushboolean(L, 1);
#else
    lua_pushboolean(L, 0);
#endif
    return 1;
}

/* Register touch input functions with Lua */
void touch_interface_register(void) {
    scripting_Register("c_set_touch_mode", c_set_touch_mode);
    scripting_Register("c_set_touch_swipe_threshold", c_set_touch_swipe_threshold);
    scripting_Register("c_get_screen_dimensions", c_get_screen_dimensions);
    scripting_Register("c_is_android", c_is_android);
}
