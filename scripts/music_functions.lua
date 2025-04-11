-- Music Functions Script
print("[music_functions] Loading music functions")

-- Initialize tracks table if it doesn't exist
if not tracks then
    tracks = {}
    
    -- Try to find music files
    local function find_music_files()
        -- Try to list files in the music directory
        local handle = io.popen("ls music/*.it music/*.ogg music/*.mp3 2>/dev/null")
        if handle then
            for file in handle:lines() do
                print("[music_functions] Found music file: " .. file)
                table.insert(tracks, file)
            end
            handle:close()
        end
        
        -- Also check sounds directory
        handle = io.popen("ls sounds/*.it sounds/*.ogg sounds/*.mp3 2>/dev/null")
        if handle then
            for file in handle:lines() do
                print("[music_functions] Found music file in sounds: " .. file)
                table.insert(tracks, file)
            end
            handle:close()
        end
    end
    
    find_music_files()
    
    -- If no tracks were found, add a default one
    if table.getn(tracks) == 0 then
        -- Try to find song_revenge_of_cats.it
        local revenge_path = "music/song_revenge_of_cats.it"
        local file = io.open(revenge_path, "r")
        if file then
            file:close()
            print("[music_functions] Found song_revenge_of_cats.it")
            table.insert(tracks, revenge_path)
        else
            -- Try in sounds directory
            revenge_path = "sounds/song_revenge_of_cats.it"
            file = io.open(revenge_path, "r")
            if file then
                file:close()
                print("[music_functions] Found song_revenge_of_cats.it in sounds directory")
                table.insert(tracks, revenge_path)
            end
        end
    end
    
    print("[music_functions] Found " .. table.getn(tracks) .. " tracks")
end

-- Initialize current_track_index if it doesn't exist
if not current_track_index then
    current_track_index = 1
    print("[music_functions] Set current_track_index to 1")
end

-- Function to get the next track
function nextTrack()
    print("[music_functions] nextTrack() called")
    
    if table.getn(tracks) == 0 then
        print("[music_functions] No tracks available")
        return 0
    end
    
    -- Move to next track
    current_track_index = current_track_index + 1
    if current_track_index > table.getn(tracks) then
        current_track_index = 1
    end
    
    -- Get the track path
    local track_path = tracks[current_track_index]
    print("[music_functions] Next track: " .. track_path)
    
    -- Extract filename from path
    local filename = string.gsub(track_path, "^.*/", "")
    print("[music_functions] Extracted filename: " .. filename)
    
    -- Set as current track
    settings.current_track = filename
    print("[music_functions] Set current_track to: " .. filename)
    
    -- Try to play the music
    if c_reloadTrack then
        print("[music_functions] Calling c_reloadTrack()")
        c_reloadTrack()
        return 1
    else
        print("[music_functions] c_reloadTrack not available")
        return 0
    end
end

-- Function to play a specific track
function playTrack(index)
    print("[music_functions] playTrack(" .. index .. ") called")
    
    if table.getn(tracks) == 0 then
        print("[music_functions] No tracks available")
        return 0
    end
    
    -- Validate index
    if index < 1 or index > table.getn(tracks) then
        print("[music_functions] Invalid track index: " .. index)
        return 0
    end
    
    -- Set current track index
    current_track_index = index
    
    -- Get the track path
    local track_path = tracks[current_track_index]
    print("[music_functions] Playing track: " .. track_path)
    
    -- Extract filename from path
    local filename = string.gsub(track_path, "^.*/", "")
    print("[music_functions] Extracted filename: " .. filename)
    
    -- Set as current track
    settings.current_track = filename
    print("[music_functions] Set current_track to: " .. filename)
    
    -- Try to play the music
    if c_reloadTrack then
        print("[music_functions] Calling c_reloadTrack()")
        c_reloadTrack()
        return 1
    else
        print("[music_functions] c_reloadTrack not available")
        return 0
    end
end

print("[music_functions] Music functions loaded")

return 1