-- Add debug prints to track execution
print("[audio.lua] Loading audio script")

-- Make sure Audio_PlayMusic is properly bound
if not Audio_PlayMusic then
    print("[audio.lua] ERROR: Audio_PlayMusic function is not bound!")
else
    print("[audio.lua] Audio_PlayMusic function is available")
end

function setupSoundTrack()
   local i,name
   for i,name in tracks do
      print("[scripting audio] found track '", name, "'")
      if name == settings.current_track then
	 current_track_index = i
	 return
      end
   end
   settings.current_track = tracks[1]
   current_track_index = 1
end

-- Original Song action handler
local old_Song_action = Song_action

-- Override the Song action handler to add debug
function Song_action(param)
    print("[audio.lua] Song_action called with param: " .. tostring(param))
    
    -- Call Audio_PlayMusic directly to ensure music plays
    if Audio_PlayMusic then
        print("[audio.lua] Calling Audio_PlayMusic directly")
        Audio_PlayMusic()
    else
        print("[audio.lua] Cannot call Audio_PlayMusic - function not bound")
    end
    
    -- Call the original handler if it exists
    if old_Song_action then
        print("[audio.lua] Calling original Song_action")
        return old_Song_action(param)
    end
end

-- Original Music action handler
local old_Music_action = Music_action

-- Override the Music action handler to add debug
function Music_action(param)
    print("[audio.lua] Music_action called with param: " .. tostring(param))
    
    -- Call Audio_PlayMusic directly to ensure music plays
    if Audio_PlayMusic then
        print("[audio.lua] Calling Audio_PlayMusic directly")
        Audio_PlayMusic()
    else
        print("[audio.lua] Cannot call Audio_PlayMusic - function not bound")
    end
    
    -- Call the original handler if it exists
    if old_Music_action then
        print("[audio.lua] Calling original Music_action")
        return old_Music_action(param)
    end
end

function nextTrack()
   print("[audio.lua] nextTrack function called")
   if current_track_index < table.getn(tracks) then
      current_track_index = current_track_index + 1
   else
      current_track_index = 1
   end
   settings.current_track = tracks[ current_track_index ]
   c_reloadTrack()
   
   -- Call Audio_PlayMusic directly to ensure music plays
   if Audio_PlayMusic then
      print("[audio.lua] Calling Audio_PlayMusic directly from nextTrack")
      Audio_PlayMusic()
   else
      print("[audio.lua] Cannot call Audio_PlayMusic - function not bound")
   end
end

function previousTrack()
   print("[audio.lua] previousTrack function called")
   if current_track_index > 1 then
      current_track_index = current_track_index - 1
   else
      current_track_index = table.getn(tracks) 
   end
   settings.current_track = tracks[ current_track_index ]
   c_reloadTrack()
   
   -- Call Audio_PlayMusic directly to ensure music plays
   if Audio_PlayMusic then
      print("[audio.lua] Calling Audio_PlayMusic directly from previousTrack")
      Audio_PlayMusic()
   else
      print("[audio.lua] Cannot call Audio_PlayMusic - function not bound")
   end
end

function MusicVolumeUp()
   settings.musicVolume = settings.musicVolume + 0.05
   if settings.musicVolume > 1.0 then
      settings.musicVolume = 1.0
   end
   c_update_audio_volume()
end

function MusicVolumeDown()
   settings.musicVolume = settings.musicVolume - 0.05
   if settings.musicVolume < 0.0 then
      settings.musicVolume = 0.0
   end
   c_update_audio_volume()
end

function FXVolumeUp()
   settings.fxVolume = settings.fxVolume + 0.05
   if settings.fxVolume > 1.0 then
      settings.fxVolume = 1.0
   end
   c_update_audio_volume()
end

function FXVolumeDown()
   settings.fxVolume = settings.fxVolume - 0.05
   if settings.fxVolume < 0.0 then
      settings.fxVolume = 0.0
   end
   c_update_audio_volume()
end

-- Add debug for AudioMenu action
local old_AudioMenu_action = AudioMenu_action

function AudioMenu_action(param)
    print("[audio.lua] AudioMenu_action called with param: " .. tostring(param))
    
    -- Check if music is enabled
    print("[audio.lua] Music enabled: " .. tostring(settings.playMusic))
    print("[audio.lua] Music volume: " .. tostring(settings.musicVolume))
    print("[audio.lua] Current track: " .. tostring(settings.current_track))
    
    -- Try to play music directly
    if settings.playMusic and Audio_PlayMusic then
        print("[audio.lua] Trying to play music directly from AudioMenu_action")
        Audio_PlayMusic()
    end
    
    -- Call the original handler
    if old_AudioMenu_action then
        print("[audio.lua] Calling original AudioMenu_action")
        return old_AudioMenu_action(param)
    end
end

-- Also add a debug function to check tracks
function debugTracks()
    print("[audio.lua] Debugging tracks table:")
    if not tracks then
        print("[audio.lua] ERROR: tracks table is nil!")
        return
    end
    
    print("[audio.lua] Number of tracks: " .. table.getn(tracks))
    for i, name in tracks do
        print("[audio.lua] Track " .. i .. ": " .. name)
    end
    
    print("[audio.lua] Current track index: " .. tostring(current_track_index))
    print("[audio.lua] Current track in settings: " .. tostring(settings.current_track))
end

-- Call the debug function at script load time
debugTracks()

print("[audio.lua] Audio script loaded successfully")
