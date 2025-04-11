-- Force Music Script
print("[force_music] Starting force music script")

-- Function to force music to play
function force_music_play()
    print("[force_music] Forcing music to play")
    
    -- Make sure music is enabled
    if settings.playMusic == nil then
        settings.playMusic = 1
        print("[force_music] Enabled music (was nil)")
    elseif settings.playMusic == 0 then
        settings.playMusic = 1
        print("[force_music] Enabled music (was 0)")
    end
    
    -- Set music volume if not set
    if settings.musicVolume == nil or settings.musicVolume == 0 then
        settings.musicVolume = 0.8
        print("[force_music] Set music volume to 0.8")
    end
    
    -- Update audio volume
    if c_update_audio_volume and type(c_update_audio_volume) == "function" then
        c_update_audio_volume()
    end
    
    -- Try to find a music file
    local music_file = "music/song_revenge_of_cats.it"
    
    -- Check if file exists
    local file = io.open(music_file, "r")
    if not file then
        print("[force_music] File not found: " .. music_file)
        
        -- Try in sounds directory
        music_file = "sounds/song_revenge_of_cats.it"
        file = io.open(music_file, "r")
        
        if not file then
            print("[force_music] File not found in sounds directory either")
            return false
        end
    end
    
    if file then
        file:close()
    end
    
    -- Extract filename
    local filename = string.gsub(music_file, "^.*/", "")
    print("[force_music] Using music file: " .. filename)
    
    -- Set as current track
    settings.current_track = filename
    print("[force_music] Set current_track to: " .. filename)
    
    -- Try to play the music
    if c_reloadTrack then
        print("[force_music] Calling c_reloadTrack()")
        c_reloadTrack()
        return true
    else
        print("[force_music] c_reloadTrack not available")
        return false
    end
end

-- Call the function
local success = force_music_play()
print("[force_music] Result: " .. tostring(success))

return success