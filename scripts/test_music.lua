-- Test script for music playback
print("[test_music.lua] Starting music test")

-- Check if Audio_PlayMusic is available
if not Audio_PlayMusic then
    print("[test_music.lua] ERROR: Audio_PlayMusic function is not bound!")
else
    print("[test_music.lua] Audio_PlayMusic function is available")
end

-- Check if tracks are defined
if not tracks then
    print("[test_music.lua] ERROR: tracks table is not defined!")
else
    print("[test_music.lua] Tracks table is defined with " .. table.getn(tracks) .. " tracks")
    
    -- Print all tracks
    for i, name in tracks do
        print("[test_music.lua] Track " .. i .. ": " .. name)
    end
end

-- Try to load and play the first track
if tracks and table.getn(tracks) > 0 then
    local track = tracks[1]
    print("[test_music.lua] Trying to load and play track: " .. track)
    
    -- Check if c_reloadTrack is available
    if not c_reloadTrack then
        print("[test_music.lua] ERROR: c_reloadTrack function is not bound!")
    else
        print("[test_music.lua] Setting current_track to: " .. track)
        settings.current_track = track
        current_track_index = 1
        
        print("[test_music.lua] Calling c_reloadTrack()")
        c_reloadTrack()
        
        print("[test_music.lua] Calling Audio_PlayMusic()")
        Audio_PlayMusic()
    end
else
    print("[test_music.lua] No tracks available to play")
end

print("[test_music.lua] Music test complete")