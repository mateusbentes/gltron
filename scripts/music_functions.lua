-- Music Functions Script
print("[music_functions] Loading music functions")

-- Initialize tracks table if it doesn't exist
if not tracks then
    tracks = {}
    
    -- Function to check if ffmpeg is available
    local function has_ffmpeg()
        local handle = io.popen("which ffmpeg 2>/dev/null")
        local result = handle:read("*a")
        handle:close()
        return result and result ~= ""
    end
    
    -- Function to convert IT to WAV
    local function convert_to_wav(input_file)
        print("[music_functions] Converting " .. input_file .. " to WAV")
        
        -- Create output filename
        local output_file = string.gsub(input_file, "%.it$", ".wav")
        if output_file == input_file then
            -- If the extension wasn't .it, append .wav
            output_file = input_file .. ".wav"
        end
        
        -- Check if output file already exists
        local f = io.open(output_file, "r")
        if f then
            f:close()
            print("[music_functions] WAV file already exists: " .. output_file)
            return output_file
        end
        
        -- Convert using ffmpeg
        local cmd = "ffmpeg -y -i \"" .. input_file .. "\" \"" .. output_file .. "\" 2>/dev/null"
        print("[music_functions] Running: " .. cmd)
        local result = os.execute(cmd)
        
        -- Check if conversion was successful
        f = io.open(output_file, "r")
        if f then
            f:close()
            print("[music_functions] Conversion successful: " .. output_file)
            return output_file
        else
            print("[music_functions] Conversion failed")
            return nil
        end
    end
    
    -- Try to find music files
    local function find_music_files()
        local can_convert = has_ffmpeg()
        
        -- First look for WAV files
        local handle = io.popen("ls music/*.wav sounds/*.wav 2>/dev/null")
        if handle then
            for file in handle:lines() do
                print("[music_functions] Found WAV music file: " .. file)
                table.insert(tracks, file)
            end
            handle:close()
        end
        
        -- Look for OGG and MP3 files
        handle = io.popen("ls music/*.ogg music/*.mp3 sounds/*.ogg sounds/*.mp3 2>/dev/null")
        if handle then
            for file in handle:lines() do
                print("[music_functions] Found music file: " .. file)
                table.insert(tracks, file)
            end
            handle:close()
        end
        
        -- If ffmpeg is available, try to convert IT files
        if can_convert then
            print("[music_functions] ffmpeg is available, can convert IT files")
            
            -- Look for IT files
            handle = io.popen("ls music/*.it sounds/*.it 2>/dev/null")
            if handle then
                for file in handle:lines() do
                    print("[music_functions] Found IT music file: " .. file)
                    local wav_file = convert_to_wav(file)
                    if wav_file then
                        table.insert(tracks, wav_file)
                    else
                        -- If conversion failed, still add the original IT file
                        table.insert(tracks, file)
                    end
                end
                handle:close()
            end
        else
            print("[music_functions] ffmpeg not available, cannot convert IT files")
            -- Add IT files without conversion
            handle = io.popen("ls music/*.it sounds/*.it 2>/dev/null")
            if handle then
                for file in handle:lines() do
                    print("[music_functions] Found IT music file: " .. file)
                    table.insert(tracks, file)
                end
                handle:close()
            end
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
            
            -- Try to convert if ffmpeg is available
            if has_ffmpeg() then
                local wav_file = convert_to_wav(revenge_path)
                if wav_file then
                    table.insert(tracks, wav_file)
                else
                    table.insert(tracks, revenge_path)
                end
            else
                table.insert(tracks, revenge_path)
            end
        else
            -- Try in sounds directory
            revenge_path = "sounds/song_revenge_of_cats.it"
            file = io.open(revenge_path, "r")
            if file then
                file:close()
                print("[music_functions] Found song_revenge_of_cats.it in sounds directory")
                
                -- Try to convert if ffmpeg is available
                if has_ffmpeg() then
                    local wav_file = convert_to_wav(revenge_path)
                    if wav_file then
                        table.insert(tracks, wav_file)
                    else
                        table.insert(tracks, revenge_path)
                    end
                else
                    table.insert(tracks, revenge_path)
                end
            end
        end
        
        -- If still no tracks, try to use game sound files
        if table.getn(tracks) == 0 then
            print("[music_functions] No music tracks found, using game sound files")
            
            -- Try to find game sound files to use as music
            local sound_files = {
                "sounds/game_win.wav",
                "sounds/game_start.wav",
                "sounds/menu_action.wav",
                "sounds/game_crash.wav",
                "sounds/game_engine.wav"
            }
            
            for i, file in ipairs(sound_files) do
                local f = io.open(file, "r")
                if f then
                    f:close()
                    print("[music_functions] Using sound file as music: " .. file)
                    table.insert(tracks, file)
                end
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

-- Function to get the previous track
function previousTrack()
    print("[music_functions] previousTrack() called")
    
    if table.getn(tracks) == 0 then
        print("[music_functions] No tracks available")
        return 0
    end
    
    -- Move to previous track
    current_track_index = current_track_index - 1
    if current_track_index < 1 then
        current_track_index = table.getn(tracks)
    end
    
    -- Get the track path
    local track_path = tracks[current_track_index]
    print("[music_functions] Previous track: " .. track_path)
    
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

-- Helper functions for volume control
function MusicVolumeUp()
    print("[music_functions] MusicVolumeUp() called")
    
    if settings.musicVolume < 1.0 then
        settings.musicVolume = settings.musicVolume + 0.1
        if settings.musicVolume > 1.0 then
            settings.musicVolume = 1.0
        end
        
        if c_update_audio_volume then
            c_update_audio_volume()
        end
    end
end

function MusicVolumeDown()
    print("[music_functions] MusicVolumeDown() called")
    
    if settings.musicVolume > 0.0 then
        settings.musicVolume = settings.musicVolume - 0.1
        if settings.musicVolume < 0.0 then
            settings.musicVolume = 0.0
        end
        
        if c_update_audio_volume then
            c_update_audio_volume()
        end
    end
end

function FXVolumeUp()
    print("[music_functions] FXVolumeUp() called")
    
    if settings.fxVolume < 1.0 then
        settings.fxVolume = settings.fxVolume + 0.1
        if settings.fxVolume > 1.0 then
            settings.fxVolume = 1.0
        end
        
        if c_update_audio_volume then
            c_update_audio_volume()
        end
    end
end

function FXVolumeDown()
    print("[music_functions] FXVolumeDown() called")
    
    if settings.fxVolume > 0.0 then
        settings.fxVolume = settings.fxVolume - 0.1
        if settings.fxVolume < 0.0 then
            settings.fxVolume = 0.0
        end
        
        if c_update_audio_volume then
            c_update_audio_volume()
        end
    end
end

print("[music_functions] Music functions loaded")

return 1
