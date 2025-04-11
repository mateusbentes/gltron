-- Save script with Music Functions
print("[save] Loading save.lua")

-- Load music functions if they're not already loaded
if not nextTrack then
    print("[save] Loading music functions")
    dofile("scripts/music_functions.lua")
end

-- dump global environment
function savevar (n,v)
 if v == nil then return end
 if type(v)=="userdata" or type(v)=="function" then return end
 -- if type(v)=="userdata" or type(v)=="function" then io.write("\t-- ") end
 -- don't print lua constants
 if string.sub(n, 1, 1) == "_" then return end
 io.write("settings.", n," = ")
 if type(v) == "string" then io.write(string.format("%q",v))
 elseif type(v) == "table" then
   if v.__visited__ ~= nil then
     io.write(v.__visited__)
   else
    io.write("{ }\n")
    v.__visited__ = n
    for r,f in v do
      if r ~= "__visited__" then
        if type(r) == 'string' then
          savevar(n.."."..r,f)
	else
          savevar(n.."["..r.."]",f)
	end
      end
    end
   end
 else io.write(tostring(v)) end
 io.write("\n")
end

-- Function to save all settings
function save()
  for n, v in pairs(settings) do
    savevar(n, v)
  end
end

-- Function to save settings with music handling
function saveSettings()
    print("[save] Saving settings")
    
    -- Make sure music is enabled
    if settings.playMusic == nil then
        settings.playMusic = 1
        print("[save] Enabled music (was nil)")
    end
    
    -- Set music volume if not set
    if settings.musicVolume == nil or settings.musicVolume == 0 then
        settings.musicVolume = 0.8
        print("[save] Set music volume to 0.8")
    end
    
    -- Update audio volume
    if c_update_audio_volume then
        c_update_audio_volume()
    end
    
    -- Try to play music if it's not already playing
    if settings.playMusic == 1 and settings.current_track == nil then
        print("[save] No current track set, trying to play music")
        if nextTrack then
            nextTrack()
        end
    end
    
    -- Save all settings
    save()
    
    return 1
end

-- Function to load settings
function loadSettings()
    print("[save] Loading settings")
    
    -- Make sure music is enabled
    if settings.playMusic == nil then
        settings.playMusic = 1
        print("[save] Enabled music (was nil)")
    end
    
    -- Set music volume if not set
    if settings.musicVolume == nil or settings.musicVolume == 0 then
        settings.musicVolume = 0.8
        print("[save] Set music volume to 0.8")
    end
    
    -- Update audio volume
    if c_update_audio_volume then
        c_update_audio_volume()
    end
    
    -- Try to play music if it's not already playing
    if settings.playMusic == 1 and settings.current_track == nil then
        print("[save] No current track set, trying to play music")
        if nextTrack then
            nextTrack()
        end
    elseif settings.playMusic == 1 and settings.current_track then
        print("[save] Current track is set to: " .. settings.current_track)
        if c_reloadTrack then
            print("[save] Calling c_reloadTrack()")
            c_reloadTrack()
        end
    end
    
    return 1
end

-- Call loadSettings to initialize
loadSettings()

print("[save] Save script loaded")

return 1
