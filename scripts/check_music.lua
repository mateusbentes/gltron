-- Script to check music settings and status
print("[check_music.lua] Starting music check")

-- Global variable to track music initialization status
if music_initialized == nil then
    music_initialized = false
end

-- Function to check if music is playing and properly configured
function check_music()
    print("[check_music.lua] Checking music status")
    
    -- Check if settings table exists
    if not settings then
        print("[check_music.lua] ERROR: settings table doesn't exist!")
        return false
    end
    
    -- Check music settings
    print("[check_music.lua] Music enabled: " .. tostring(settings.playMusic))
    print("[check_music.lua] Music volume: " .. tostring(settings.musicVolume))
    
    -- Check if music is enabled in settings
    if settings.playMusic ~= 1 then
        print("[check_music.lua] Music is disabled in settings")
        return false
    end
    
    -- Check if music is initialized
    if not music_initialized then
        print("[check_music.lua] Music is not initialized")
        
        -- Try to initialize music
        print("[check_music.lua] Trying to initialize music")
        if loadfile("scripts/direct_music_player.lua") then
            local success = dofile("scripts/direct_music_player.lua")
            if success then
                print("[check_music.lua] Music started successfully")
                music_initialized = true
            else
                print("[check_music.lua] Failed to start music")
                return false
            end
        else
            print("[check_music.lua] WARNING: direct_music_player.lua not found")
            return false
        end
    end
    
    -- Check current track
    if not settings.current_track or settings.current_track == "" then
        print("[check_music.lua] No current track set")
        return false
    end
    
    print("[check_music.lua] Current track: " .. tostring(settings.current_track))
    
    -- Check if tracks table exists
    if not tracks then
        print("[check_music.lua] ERROR: tracks table doesn't exist!")
        return false
    else
        -- Print all tracks
        print("[check_music.lua] Number of tracks: " .. table.getn(tracks))
        for i, name in tracks do
            print("[check_music.lua] Track " .. i .. ": " .. name)
        end
        
        -- Check current track index
        if current_track_index then
            print("[check_music.lua] Current track index: " .. current_track_index)
        else
            print("[check_music.lua] ERROR: current_track_index doesn't exist!")
            return false
        end
    end
    
    -- Try to reload the current track
    if c_reloadTrack then
        print("[check_music.lua] Calling c_reloadTrack()")
        c_reloadTrack()
    else
        print("[check_music.lua] ERROR: c_reloadTrack function doesn't exist!")
        return false
    end
    
    return true
end

-- Run the check
local success = check_music()
print("[check_music.lua] Music check result: " .. tostring(success))

-- Return the result so this script can be used with dofile()
return success
