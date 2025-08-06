-- Android touch input configuration
print("[lua] Loading Android touch configuration")

-- Touch control modes
TOUCH_MODE_SWIPE = 0         -- Swipe to change direction
TOUCH_MODE_VIRTUAL_DPAD = 1  -- Virtual directional pad
TOUCH_MODE_SCREEN_REGIONS = 2 -- Touch different screen regions for directions

-- Default touch settings
touch_mode = TOUCH_MODE_SWIPE
touch_swipe_threshold = 20

-- Function to set touch mode
function set_touch_mode(mode)
    if mode >= 0 and mode <= 2 then
        touch_mode = mode
        -- Call C function to set the mode
        if c_set_touch_mode then
            c_set_touch_mode(mode)
        end
        print("[lua] Touch mode set to: " .. mode)
    else
        print("[lua] Invalid touch mode: " .. mode)
    end
end

-- Function to set swipe threshold
function set_touch_swipe_threshold(threshold)
    if threshold > 0 then
        touch_swipe_threshold = threshold
        -- Call C function to set the threshold
        if c_set_touch_swipe_threshold then
            c_set_touch_swipe_threshold(threshold)
        end
        print("[lua] Touch swipe threshold set to: " .. threshold)
    else
        print("[lua] Invalid touch swipe threshold: " .. threshold)
    end
end

-- Initialize touch settings based on device
function init_touch_settings()
    -- Check if we're on Android
    if is_android and is_android() then
        -- Use swipe mode by default on Android
        set_touch_mode(TOUCH_MODE_SWIPE)
        
        -- Set appropriate swipe threshold based on screen size
        local width, height = get_screen_dimensions()
        if width and height then
            local min_dimension = math.min(width, height)
            -- Set threshold to about 5% of the smaller screen dimension
            local threshold = math.floor(min_dimension * 0.05)
            set_touch_swipe_threshold(threshold)
        else
            -- Default threshold if we can't get screen dimensions
            set_touch_swipe_threshold(20)
        end
        
        print("[lua] Android touch input configured")
    end
end

-- Function to get screen dimensions
function get_screen_dimensions()
    -- This should be implemented in C and exposed to Lua
    if c_get_screen_dimensions then
        return c_get_screen_dimensions()
    end
    return nil, nil
end

-- Function to check if we're on Android
function is_android()
    -- This should be implemented in C and exposed to Lua
    if c_is_android then
        return c_is_android()
    end
    return false
end

-- Initialize touch settings
init_touch_settings()

print("[lua] Android touch configuration loaded")