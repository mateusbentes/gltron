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

-- Check if c_setCallback is defined
if type(c_setCallback) ~= "function" then
    print("[lua] ERROR: c_setCallback is not defined or not a function")
    print("[lua] Type of c_setCallback: " .. type(c_setCallback))
else
    print("[lua] c_setCallback is defined as a function")
end

-- Initialize return codes
safe_execute(function()
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
end, "initializing EScriptingReturnCode")

-- Check for timedemo mode
safe_execute(function()
    print("[lua] Checking for timedemo mode")
    timedemo = nil
    -- timedemo = 1
    if(timedemo) then
        print("[lua] Running in timedemo mode")
        if type(c_setCallback) == "function" then
            c_setCallback("timedemo")
        else
            print("[lua] ERROR: c_setCallback is not defined or not a function")
        end
        if type(c_mainLoop) == "function" then
            c_mainLoop()
        else
            print("[lua] ERROR: c_mainLoop is not defined or not a function")
        end
        os.exit()
    end
end, "timedemo check")

-- Initialize variables and callbacks
safe_execute(function()
    print("[lua] Initializing variables and callbacks")
    -- Initialize variables
    if callback == nil then
        callback = "gui"
    else
        callback = validate_callback(callback)
    end

    if Menu == nil then
        Menu = MainGameMenu
    end

    game_initialized = 0;

    -- Initialize callback mapping table
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
    next_callback[ EScriptingReturnCode.eSRC_Timedemo ] = nil
    next_callback[ EScriptingReturnCode.eSRC_Timedemo_Abort ] = nil
    next_callback[ EScriptingReturnCode.eSRC_Quit ] = nil
    -- next_callback[ EScriptingReturnCode.eSRC_32bitWarning_OK ] = function() return "gui"; end
end, "initializing variables and callbacks")

print("[lua] Starting main loop")
safe_execute(function()
    local iteration = 0
    while true do
        iteration = iteration + 1
        print("[lua] Main loop iteration " .. iteration)
        
        -- Validate callback before using it
        callback = validate_callback(callback)
        
        print("[lua] Setting callback: " .. tostring(callback))
        
        -- Check if c_setCallback is defined
        if type(c_setCallback) ~= "function" then
            print("[lua] ERROR: c_setCallback is not defined or not a function")
            print("[lua] Type of c_setCallback: " .. type(c_setCallback))
            break
        end
        
        -- Try to call c_setCallback with error handling and more debugging
        local cb_status, cb_err = pcall(function()
            print("[lua] About to call c_setCallback with: " .. tostring(callback))
            
            -- Check for any global variables that might be nil
            print("[lua] Checking global variables:")
            print("[lua] - EScriptingReturnCode: " .. tostring(EScriptingReturnCode))
            print("[lua] - next_callback: " .. tostring(next_callback))
            print("[lua] - game_initialized: " .. tostring(game_initialized))
            
            -- Try to call c_setCallback with just the string "gui" directly
            print("[lua] Calling c_setCallback with hardcoded 'gui'")
            c_setCallback("gui")
            
            print("[lua] c_setCallback call completed")
        end)
        
        if not cb_status then
            print("[lua] ERROR setting callback: " .. tostring(cb_err))
            
            -- Try to identify what's nil
            print("[lua] Trying to identify the nil value:")
            local debug_status, debug_err = pcall(function()
                -- Try to access common global variables
                print("[lua] - _G: " .. tostring(_G))
                print("[lua] - _G.c_setCallback: " .. tostring(_G.c_setCallback))
                
                -- Try to call c_setCallback with minimal arguments
                print("[lua] Calling c_setCallback with no arguments")
                c_setCallback()
            end)
            
            if not debug_status then
                print("[lua] DEBUG ERROR: " .. tostring(debug_err))
            end
            
            break
        end
        
        print("[lua] Running main loop")
        
        -- Check if c_mainLoop is defined
        if type(c_mainLoop) ~= "function" then
            print("[lua] ERROR: c_mainLoop is not defined or not a function")
            print("[lua] Type of c_mainLoop: " .. type(c_mainLoop))
            break
        end
        
        -- Try to call c_mainLoop with error handling
        local loop_status, status = pcall(function()
            return c_mainLoop()
        end)
        
        if not loop_status then
            print("[lua] ERROR in main loop: " .. tostring(status))
            break
        end
        
        print("[lua] System returned: " .. tostring(status))
        
        if next_callback[status] then
            print("[lua] Calling next_callback handler for status: " .. tostring(status))
            local next_cb_status, next_cb = pcall(next_callback[status])
            if not next_cb_status then
                print("[lua] ERROR in callback handler: " .. tostring(next_cb))
                break
            end
            callback = validate_callback(next_cb)
            print("[lua] Next callback: " .. tostring(callback))
        else
            if status == EScriptingReturnCode.eSRC_Quit then
                print("[lua] Clean exit")
                break
            else
                print("[lua] Unhandled callback (" .. tostring(status) .. ")")
                break
            end
        end
    end
end, "main loop")

print("[lua] main.lua completed")
