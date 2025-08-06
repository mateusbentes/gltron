-- Menu Music Continuity Script
print("[menu_continuity] Loading menu music continuity script")

-- Store the original Menu.action function
local original_menu_action = Menu.action

-- Override Menu.action to ensure music continues playing
Menu.action = function(...)
    print("[menu_continuity] Menu action called")
    
    -- Always check if we need to play music when menu action is called
    if settings.playMusic == 1 then
        print("[menu_continuity] Checking music status")
        
        -- Check if we're in the RootMenu
        if Menu and Menu.current == "RootMenu" then
            print("[menu_continuity] In RootMenu, ensuring music is playing")
            
            -- Try to play music directly
            if loadfile("scripts/direct_music_player.lua") then
                local success = dofile("scripts/direct_music_player.lua")
                if success then
                    print("[menu_continuity] Music started successfully")
                    music_initialized = true
                else
                    print("[menu_continuity] Failed to start music")
                end
            else
                print("[menu_continuity] WARNING: direct_music_player.lua not found")
            end
        end
    end
    
    -- Call the original function
    return original_menu_action(...)
end

-- Also override the nextTrack function to ensure it works
if nextTrack then
    local original_nextTrack = nextTrack
    
    nextTrack = function()
        print("[menu_continuity] nextTrack called")
        
        -- Call the original function
        local result = original_nextTrack()
        
        -- If it didn't work, try direct approach
        if not result and settings.current_track and settings.current_track ~= "" then
            print("[menu_continuity] nextTrack failed, trying direct approach")
            
            if c_reloadTrack then
                print("[menu_continuity] Calling c_reloadTrack directly")
                c_reloadTrack()
                return true
            end
        end
        
        return result
    end
end

print("[menu_continuity] Menu music continuity script loaded")
