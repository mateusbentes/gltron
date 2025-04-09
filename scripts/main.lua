print("[lua] Starting main.lua")

-- Define a function to safely execute code with error handling
function safe_execute(func, description)
    local status, err = pcall(func)
    if not status then
        print("[lua] ERROR in " .. description .. ": " .. tostring(err))
        return false
    end
    return true
end

-- loop until RETURN_QUIT code is sent
safe_execute(function()
    EScriptingReturnCode =
    {
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

-- enable below to run in timedemo-only mode
safe_execute(function()
    timedemo = nil
    -- timedemo = 1
    if(timedemo) then
        c_setCallback("timedemo")
        c_mainLoop()
        os.exit()
    end
end, "timedemo check")

safe_execute(function()
    if callback == nil then
        callback = "gui"
    end

    if Menu == nil then
        Menu = MainGameMenu
    end

    game_initialized = 0;
end, "initializing variables")

safe_execute(function()
    next_callback = {}
    next_callback[ EScriptingReturnCode.eSRC_Game_Launch ] = 
        function() 
            game_initialized = 1;
            return "pause";
        end
    next_callback[ EScriptingReturnCode.eSRC_Game_End ] = function() return "pause"; end
    next_callback[ EScriptingReturnCode.eSRC_Game_Pause ] = function() return "pause"; end
    next_callback[ EScriptingReturnCode.eSRC_Game_Unpause ] = function() return "game"; end
    next_callback[ EScriptingReturnCode.eSRC_Credits ] = function() return "credits"; end

    next_callback[ EScriptingReturnCode.eSRC_Game_Escape ] = function() return "gui"; end
    next_callback[ EScriptingReturnCode.eSRC_GUI_Escape ] =
        function() 
            if(game_initialized == 1) then
                return "pause"
            else
                return "gui"
            end
        end
        
    next_callback[ EScriptingReturnCode.eSRC_GUI_Prompt_Escape ] = function() return "gui"; end
    next_callback[ EScriptingReturnCode.eSRC_Pause_Escape ] = function() return "gui"; end

    next_callback[ EScriptingReturnCode.eSRC_GUI_Prompt ] = function() return "configure"; end
    next_callback[ EScriptingReturnCode.eSRC_Timedemo ] = nil
    next_callback[ EScriptingReturnCode.eSRC_Timedemo_Abort ] = nil
    next_callback[ EScriptingReturnCode.eSRC_Quit ] = nil
    -- next_callback[ EScriptingReturnCode.eSRC_32bitWarning_OK ] = function() return "gui"; end
end, "initializing callbacks")

print("[lua] Starting main loop")
safe_execute(function()
    while 1 do
        print("[lua] Setting callback: " .. tostring(callback))
        c_setCallback(callback)
        
        print("[lua] Running main loop")
        status = c_mainLoop()
        print("[lua] System returned: " .. tostring(status))
        
        if(next_callback[ status ]) then
            callback = next_callback[ status ]()
            print("[lua] Next callback: " .. tostring(callback))
        else
            if(status == EScriptingReturnCode.eSRC_Quit) then
                print("[lua] clean exit")
                break
            else
                print("[lua] unhandled callback (" .. tostring(status) .. ")")
                break
            end
        end
    end
end, "main loop")

print("[lua] main.lua completed")
