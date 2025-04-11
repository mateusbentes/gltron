-- Menu Music Script
print("[menu_music.lua] Initializing menu music")

-- Define the tracks table if it doesn't exist
if not tracks then
    tracks = {}
end

-- Check if settings exist
if not settings then
    print("[menu_music.lua] ERROR: settings table doesn't exist!")
else
    -- Check music settings
    print("[menu_music.lua] Music enabled: " .. tostring(settings.playMusic))
    print("[menu_music.lua] Music volume: " .. tostring(settings.musicVolume))
    print("[menu_music.lua] Current track: " .. tostring(settings.current_track))
end

-- Function to check if a file exists
function file_exists(path)
    local file = io.open(path, "r")
    if file then
        file:close()
        return true
    end
    return false
end

-- Helper function to check if table contains value
function table.contains(table, element)
    for _, value in pairs(table) do
        if value == element then
            return true
        end
    end
    return false
end

-- Find music files and add them to tracks
function find_and_add_tracks()
    local found_tracks = false
    
    -- Try to list files in the music directory
    local handle = io.popen("ls music/*.it music/*.ogg music/*.mp3 2>/dev/null")
    if handle then
        for file in handle:lines() do
            print("[menu_music.lua] Found music file: " .. file)
            table.insert(tracks, file)
            found_tracks = true
        end
        handle:close()
    end
    
    -- Also check sounds directory
    handle = io.popen("ls sounds/*.it sounds/*.ogg sounds/*.mp3 2>/dev/null")
    if handle then
        for file in handle:lines() do
            print("[menu_music.lua] Found music file in sounds: " .. file)
            table.insert(tracks, file)
            found_tracks = true
        end
        handle:close()
    end
    
    -- Add default tracks if they exist
    local default_tracks = {
        "music/song_revenge_of_cats.it",
    }
    
    for _, track in ipairs(default_tracks) do
        if file_exists(track) and not table.contains(tracks, track) then
            print("[menu_music.lua] Adding default track: " .. track)
            table.insert(tracks, track)
            found_tracks = true
        end
    end
    
    return found_tracks
end

-- Find and add tracks
local found = find_and_add_tracks()

-- Print the tracks we found
if found then
    print("[menu_music.lua] Found " .. #tracks .. " music tracks:")
    for i, track in ipairs(tracks) do
        print("[menu_music.lua] Track " .. i .. ": " .. track)
    end
    
    -- Set current track if not set
    if not settings.current_track or settings.current_track == "" then
        settings.current_track = tracks[1]
        current_track_index = 1
        print("[menu_music.lua] Set current track to: " .. settings.current_track)
    else
        -- Find current track index
        for i, track in ipairs(tracks) do
            if track == settings.current_track then
                current_track_index = i
                break
            end
        end
        
        -- If track not found in list, default to first track
        if not current_track_index then
            current_track_index = 1
            settings.current_track = tracks[1]
            print("[menu_music.lua] Reset current track to: " .. settings.current_track)
        else
            print("[menu_music.lua] Current track index: " .. current_track_index)
        end
    end
else
    print("[menu_music.lua] No music tracks found")
end

-- Function to get the next track
function nextTrack()
    print("[menu_music.lua] nextTrack() called")
    
    if #tracks == 0 then
        print("[menu_music.lua] No tracks available")
        return 0
    end
    
    -- Move to next track
    current_track_index = current_track_index + 1
    if current_track_index > #tracks then
        current_track_index = 1
    end
    
    -- Set new current track
    settings.current_track = tracks[current_track_index]
    print("[menu_music.lua] Next track: " .. settings.current_track)
    
    -- Load and play the track
    if Audio_LoadMusic and Audio_PlayMusic then
        print("[menu_music.lua] Loading music: " .. settings.current_track)
        Audio_LoadMusic(settings.current_track)
        
        print("[menu_music.lua] Playing music")
        Audio_PlayMusic()
        
        return 1
    else
        -- Try to reload the current track using c_reloadTrack if available
        if c_reloadTrack then
            print("[menu_music.lua] Calling c_reloadTrack()")
            c_reloadTrack()
            return 1
        else
            print("[menu_music.lua] No audio functions available")
            return 0
        end
    end
end

-- Try to play the current track
if c_reloadTrack then
    print("[menu_music.lua] Calling c_reloadTrack()")
    c_reloadTrack()
else
    print("[menu_music.lua] WARNING: c_reloadTrack function doesn't exist!")
end

print("[menu_music.lua] Menu music initialization complete")
