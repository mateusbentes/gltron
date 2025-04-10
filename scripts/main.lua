-- Debug version of main.lua with extensive logging
print("[lua] Starting main.lua with debug logging")

-- Define a function to safely execute code with error handling
function safe_execute(func, description)
    print("[lua] Executing: " .. description)
    local status, err = pcall(func)
    if not status then
        print("[lua] ERROR in " .. description .. ": " .. tostring(err))
        return false
    end
    print("[lua] Successfully executed: " .. description)
    return true
end

-- Define valid callbacks to check against
local valid_callbacks = {
    ["gui"] = true,
    ["game"] = true,
    ["pause"] = true,
    ["credits"] = true,
    ["configure"] = true,
    ["timedemo"] = true
}

-- Function to validate callback names
function validate_callback(cb_name)
    print("[lua] Validating callback: " .. tostring(cb_name))
    if type(cb_name) ~= "string" then
        print("[lua] WARNING: Callback is not a string: " .. tostring(cb_name))
        return "gui" -- Default to gui if not a string
    end
    
    if not valid_callbacks[cb_name] then
        print("[lua] WARNING: Unknown callback: " .. cb_name)
        return "gui" -- Default to gui if unknown
    end
    
    return cb_name
end

-- Initialize global variables
callback = "gui"
game_initialized = 0

-- Initialize return codes
if not EScriptingReturnCode then
    print("[lua] Initializing EScriptingReturnCode")
    EScriptingReturnCode = {
        eSRC_Quit = 0,
        eSRC_Game_End = 1,
        eSRC_Game_Pause = 2,
        eSRC_Game_Unpause = 3,
        eSRC_Game_Credits = 4,
        eSRC_Game_Escape = 5,
        eSRC_Timedemo = 7,
        eSRC_Timedemo_Abort = 8,
        eSRC_Credits = 9,
        eSRC_Game_Launch = 10,
        eSRC_GUI_Escape = 11,
        eSRC_GUI_Prompt = 12,
        eSRC_GUI_Prompt_Escape = 13,
        eSRC_Pause_Escape = 14,
        eSRC_32bitWarning_OK = 15,
    }
else
    print("[lua] EScriptingReturnCode already initialized")
end

-- Initialize callback mapping table
if not next_callback then
    print("[lua] Initializing next_callback")
    next_callback = {}
    next_callback[ EScriptingReturnCode.eSRC_Game_Launch ] = 
        function() 
            print("[lua] Game launch -> pause")
            game_initialized = 1;
            return "pause";
        end
    next_callback[ EScriptingReturnCode.eSRC_Game_End ] = function() 
        print("[lua] Game end -> pause")
        return "pause"; 
    end
    next_callback[ EScriptingReturnCode.eSRC_Game_Pause ] = function() 
        print("[lua] Game pause -> pause")
        return "pause"; 
    end
    next_callback[ EScriptingReturnCode.eSRC_Game_Unpause ] = function() 
        print("[lua] Game unpause -> game")
        return "game"; 
    end
    next_callback[ EScriptingReturnCode.eSRC_Credits ] = function() 
        print("[lua] Credits -> credits")
        return "credits"; 
    end
    next_callback[ EScriptingReturnCode.eSRC_Game_Escape ] = function() 
        print("[lua] Game escape -> gui")
        return "gui"; 
    end
    next_callback[ EScriptingReturnCode.eSRC_GUI_Escape ] =
        function() 
            if(game_initialized == 1) then
                print("[lua] GUI escape (game initialized) -> pause")
                return "pause"
            else
                print("[lua] GUI escape (game not initialized) -> gui")
                return "gui"
            end
        end
    next_callback[ EScriptingReturnCode.eSRC_GUI_Prompt_Escape ] = function() 
        print("[lua] GUI prompt escape -> gui")
        return "gui"; 
    end
    next_callback[ EScriptingReturnCode.eSRC_Pause_Escape ] = function() 
        print("[lua] Pause escape -> gui")
        return "gui"; 
    end
    next_callback[ EScriptingReturnCode.eSRC_GUI_Prompt ] = function() 
        print("[lua] GUI prompt -> configure")
        return "configure"; 
    end
else
    print("[lua] next_callback already initialized")
end

-- Check for timedemo mode
timedemo = nil
if(timedemo) then
    print("[lua] Running in timedemo mode")
    c_setCallback("timedemo")
    c_mainLoop()
    os.exit()
end

print("[lua] Starting main loop")
local iteration = 0
while true do
    iteration = iteration + 1
    print("[lua] Main loop iteration " .. iteration)
    
    -- Validate callback before using it
    callback = validate_callback(callback)
    
    print("[lua] Setting callback: " .. tostring(callback))
    
    -- Try to call c_setCallback with error handling
    local cb_status, cb_err = pcall(function()
        c_setCallback(callback)
    end)
    
    if not cb_status then
        print("[lua] ERROR setting callback: " .. tostring(cb_err))
        -- Try to set callback to "gui" as a fallback
        print("[lua] Trying to set callback to 'gui' as fallback")
        local fallback_status, fallback_err = pcall(function()
            c_setCallback("gui")
        end)
        if not fallback_status then
            print("[lua] ERROR setting fallback callback: " .. tostring(fallback_err))
            break
        end
        callback = "gui"
    end
    
    print("[lua] Running main loop")
    
    -- Try to call c_mainLoop with error handling
    local loop_status, status = pcall(function()
        return c_mainLoop()
    end)
    
    if not loop_status then
        print("[lua] ERROR in main loop: " .. tostring(status))
        break
    end
    
    print("[lua] System returned: " .. tostring(status))
    
    if status == nil then
        print("[lua] WARNING: status is nil, defaulting to gui")
        callback = "gui"
    elseif next_callback == nil then
        print("[lua] WARNING: next_callback is nil, defaulting to gui")
        callback = "gui"
    elseif next_callback[status] == nil then
        print("[lua] WARNING: next_callback[" .. tostring(status) .. "] is nil")
        if status == EScriptingReturnCode.eSRC_Quit then
            print("[lua] Clean exit")
            break
        else
            print("[lua] Unhandled callback (" .. tostring(status) .. "), defaulting to gui")
            callback = "gui"
        end
    else
        print("[lua] Calling next_callback handler for status: " .. tostring(status))
        local next_cb_status, next_cb = pcall(next_callback[status])
        if not next_cb_status then
            print("[lua] ERROR in callback handler: " .. tostring(next_cb))
            callback = "gui"
        else
            callback = validate_callback(next_cb)
            print("[lua] Next callback: " .. tostring(callback))
        end
    end
end

print("[lua] main.lua completed")
