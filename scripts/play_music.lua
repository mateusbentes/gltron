-- Play Music Function
function playMusic(filename)
    print("[play_music.lua] Attempting to play music: " .. filename)
    
    -- Check if the file exists
    local file = io.open(filename, "r")
    if not file then
        print("[play_music.lua] File not found: " .. filename)
        
        -- Try with music/ prefix
        local music_path = "music/" .. filename
        file = io.open(music_path, "r")
        if file then
            file:close()
            filename = music_path
            print("[play_music.lua] Found file with music/ prefix: " .. filename)
        else
            print("[play_music.lua] File not found with music/ prefix either")
            return false
        end
    else
        file:close()
    end
    
    -- Set as current track
    settings.current_track = filename:match("([^/]+)$")
    print("[play_music.lua] Set current_track to: " .. settings.current_track)
    
    -- Call c_reloadTrack to load and play the music
    if c_reloadTrack then
        print("[play_music.lua] Calling c_reloadTrack()")
        c_reloadTrack()
        return true
    else
        print("[play_music.lua] ERROR: c_reloadTrack function doesn't exist!")
        return false
    end
end

-- Try to play song_revenge_of_cats.it
print("[play_music.lua] Trying to play song_revenge_of_cats.it")
if not playMusic("song_revenge_of_cats.it") then
    -- Try to find and play any music file
    print("[play_music.lua] Trying to find any music file")
    local found = false
    for file in io.popen("ls music/*.it music/*.ogg music/*.mp3 2>/dev/null"):lines() do
        print("[play_music.lua] Found music file: " .. file)
        if playMusic(file) then
            found = true
            break
        end
    end
    
    if not found then
        print("[play_music.lua] No music files could be played")
    end
end