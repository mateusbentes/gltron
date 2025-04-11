-- Simplified Music Player Script
print("[direct_music] Starting simplified music player")

-- Function to check if a file exists
function file_exists(path)
    local file = io.open(path, "r")
    if file then
        file:close()
        return true
    end
    return false
end

-- Function to find music files
function find_music_files()
    local music_files = {}
    
    -- Try to list files in the music directory
    local handle = io.popen("ls music/*.it music/*.ogg music/*.mp3 2>/dev/null")
    if handle then
        for file in handle:lines() do
            print("[direct_music] Found music file: " .. file)
            table.insert(music_files, file)
        end
        handle:close()
    end
    
    -- Also check sounds directory
    handle = io.popen("ls sounds/*.it sounds/*.ogg sounds/*.mp3 2>/dev/null")
    if handle then
        for file in handle:lines() do
            print("[direct_music] Found music file in sounds: " .. file)
            table.insert(music_files, file)
        end
        handle:close()
    end
    
    return music_files
end

-- Try to play a music file using c_reloadTrack
function try_play_music(file_path)
    print("[direct_music] Trying to play: " .. file_path)
    
    -- Check if file exists
    if not file_exists(file_path) then
        print("[direct_music] File not found: " .. file_path)
        return false
    end
    
    -- Extract filename from path
    local filename = string.gsub(file_path, "^.*/", "")
    print("[direct_music] Extracted filename: " .. filename)
    
    -- Set as current track in settings
    if not settings then
        print("[direct_music] ERROR: settings table not available")
        return false
    end
    
    settings.current_track = filename
    print("[direct_music] Set settings.current_track to: " .. filename)
    
    -- Check if c_reloadTrack is available
    if not c_reloadTrack then
        print("[direct_music] ERROR: c_reloadTrack function is not available")
        return false
    end
    
    -- Try to reload and play the track
    print("[direct_music] Calling c_reloadTrack()")
    local status, err = pcall(function()
        c_reloadTrack()
    end)
    
    if not status then
        print("[direct_music] ERROR calling c_reloadTrack: " .. tostring(err))
        return false
    end
    
    print("[direct_music] Music playback initiated successfully")
    return true
end

-- Main function
function play_music()
    print("[direct_music] Starting music playback")
    
    -- Make sure music is enabled in settings
    if settings then
        if settings.playMusic == nil or settings.playMusic == 0 then
            settings.playMusic = 1
            print("[direct_music] Enabled music in settings")
        end
        
        if settings.musicVolume == nil or settings.musicVolume == 0 then
            settings.musicVolume = 0.8
            print("[direct_music] Set music volume to 0.8 in settings")
        end
        
        -- Update audio volume if function is available
        if c_update_audio_volume then
            print("[direct_music] Calling c_update_audio_volume()")
            c_update_audio_volume()
        end
    else
        print("[direct_music] WARNING: settings table not available")
    end
    
    -- First try song_revenge_of_cats.it
    local revenge_path = "music/song_revenge_of_cats.it"
    if file_exists(revenge_path) then
        print("[direct_music] Found song_revenge_of_cats.it")
        if try_play_music(revenge_path) then
            return true
        end
    else
        print("[direct_music] song_revenge_of_cats.it not found")
    end
    
    -- Try in sounds directory
    revenge_path = "sounds/song_revenge_of_cats.it"
    if file_exists(revenge_path) then
        print("[direct_music] Found song_revenge_of_cats.it in sounds directory")
        if try_play_music(revenge_path) then
            return true
        end
    end
    
    -- Try any music file
    local music_files = find_music_files()
    if table.getn(music_files) > 0 then
        for i = 1, table.getn(music_files) do
            local file = music_files[i]
            if try_play_music(file) then
                return true
            end
        end
    end
    
    print("[direct_music] No music files could be played")
    return false
end

-- Execute the main function safely
print("[direct_music] Executing play_music()")
local success = false

-- Use pcall to catch any errors
local status, result = pcall(play_music)
if status then
    success = result
    print("[direct_music] play_music() returned: " .. tostring(success))
else
    print("[direct_music] ERROR in play_music(): " .. tostring(result))
end

-- Return success status
return success
