#include "audio/nebu_SourceMusic.h"
#include "audio/nebu_SoundSystem.h"
#include <SDL2/SDL.h>
#include <string.h>
#include <stdio.h>

// Include libxmp headers
#include <xmp.h>

// Add these declarations if they're not in the header
#ifndef XMP_STATE_ENDED
#define XMP_STATE_ENDED 3
#endif

// Forward declaration for xmp_get_error if needed
const char *xmp_get_error(xmp_context ctx);

#include "base/nebu_debug_memory.h"

namespace Sound {

SourceMusic::SourceMusic(System *system) {
    _system = system;
    _buffer = NULL;
    _buffersize = 0;
    _sample_buffersize = 0;
    _decoded = 0;
    _read = 0;
    _filename = NULL;
    _rwops = NULL;
    _xmp_context = NULL;  // Initialize to NULL
    _loaded = false;      // Initialize to false
    Reset();
}

SourceMusic::~SourceMusic()
{
    // fprintf(stderr, "nebu_SourceMusic destructor called\n");
    CleanUp();
}

/*!
    \fn void SourceMusic::CreateSample(void)

    call this function only between semaphores
*/

void SourceMusic::CreateSample(void) {
    // This function is not needed for our implementation
    // as we load the entire file at once in Load()
}

void SourceMusic::Load(char *filename) {
    CleanUp();

    // Store the filename for later use
    int n = strlen(filename);
    _filename = new char[n+1];
    memcpy(_filename, filename, n + 1);

    printf("[audio] Loading music file: %s\n", filename);

    // Check file extension to determine format
    const char *ext = strrchr(filename, '.');
    if (ext) {
        ext++; // Skip the '.'
        if (strcasecmp(ext, "wav") == 0) {
            printf("[audio] Detected WAV format for music file: %s\n", filename);
            LoadWAV(filename);
        } else if (strcasecmp(ext, "it") == 0) {
            printf("[audio] Detected IT format for music file: %s\n", filename);
            if (!LoadIT(filename)) {
                fprintf(stderr, "[error] Failed to load IT file: %s\n", filename);
            }
        } else {
            printf("[audio] Detected unknown format for music file: %s\n", filename);
            fprintf(stderr, "[error] Unsupported music file format: %s\n", ext);
            fprintf(stderr, "[error] Supported formats are WAV and IT\n");
        }
    } else {
        printf("[audio] No file extension found for music file: %s\n", filename);
        fprintf(stderr, "[error] Cannot determine music file format\n");
        fprintf(stderr, "[error] Please ensure your music file has a proper extension\n");
    }
}

void SourceMusic::LoadWAV(char *filename) {
    // Try to load as WAV
    SDL_AudioSpec wav_spec;
    Uint32 wav_length;
    Uint8 *wav_buffer;

    if (SDL_LoadWAV(filename, &wav_spec, &wav_buffer, &wav_length) != NULL) {
        printf("[audio] Music file loaded as WAV\n");

        // Convert the audio to the system format if needed
        SDL_AudioCVT cvt;
        SDL_AudioSpec *spec = _system->GetSpec();

        if (SDL_BuildAudioCVT(&cvt, wav_spec.format, wav_spec.channels, wav_spec.freq,
                              spec->format, spec->channels, spec->freq) < 0) {
            fprintf(stderr, "[error] Could not build audio converter: %s\n", SDL_GetError());
            SDL_FreeWAV(wav_buffer);
            return;
        }

        // Allocate memory for the converted audio
        cvt.len = wav_length;
        cvt.buf = new Uint8[cvt.len * cvt.len_mult];
        memcpy(cvt.buf, wav_buffer, wav_length);

        // Convert the audio
        if (SDL_ConvertAudio(&cvt) < 0) {
            fprintf(stderr, "[error] Could not convert audio: %s\n", SDL_GetError());
            delete[] cvt.buf;
            SDL_FreeWAV(wav_buffer);
            return;
        }

        // Free the original WAV buffer
        SDL_FreeWAV(wav_buffer);

        // Store the converted audio
        _buffer = cvt.buf;
        _buffersize = cvt.len_cvt;
        _decoded = _buffersize;
        _read = 0;

        printf("[audio] Music converted successfully - size: %d bytes\n", _buffersize);
    } else {
        fprintf(stderr, "[error] Could not load WAV file: %s\n", SDL_GetError());
    }
}


bool SourceMusic::LoadIT(const char* filename) {
    // Clean past context
    CleanUp();

    // Create context
    xmp_context ctx = xmp_create_context();
    if (!ctx) {
        fprintf(stderr, "[error] Failed to create xmp context\n");
        return false;
    }

    // Load the module
    if (xmp_load_module(ctx, filename) != 0) {
        fprintf(stderr, "[error] Failed to load module: %s\n", filename);
        xmp_free_context(ctx);
        return false;
    }

    // Configure player settings
    xmp_set_player(ctx, XMP_PLAYER_VOLUME, 100); // volume 0–100
    xmp_set_player(ctx, XMP_PLAYER_INTERP, XMP_INTERP_LINEAR); // interpolação
    xmp_set_player(ctx, XMP_PLAYER_DSP, 1); // DSP ligado (opcional)

    // Start player (stereo 44100 Hz)
    if (xmp_start_player(ctx, 44100, 0) != 0) {
        fprintf(stderr, "[error] Failed to start player\n");
        xmp_free_context(ctx);
        return false;
    }

    // Saving context
    _xmp_context = ctx;
    _loaded = true;
    return true;
}

void SourceMusic::CleanUp(void) {
    if(_buffer) {
        delete[] _buffer;
        _buffer = NULL;
    }

    if(_filename) {
        delete[] _filename;
        _filename = NULL;
    }

    if(_rwops) {
        SDL_RWclose(_rwops);
        _rwops = NULL;
    }

    // Clean up libxmp context if it exists
    if (_xmp_context) {
        xmp_end_player(_xmp_context);
        xmp_release_module(_xmp_context);
        xmp_free_context(_xmp_context);
        _xmp_context = NULL;
    }

    _buffersize = 0;
    _sample_buffersize = 0;
    _decoded = 0;
    _read = 0;
    _loaded = false;
}

int SourceMusic::Mix(Uint8 *data, int len) {
    // Mix the music into the output buffer
    if (!_isPlaying || (!_buffer && !_xmp_context)) {
        return 0;
    }

    // If we have a libxmp context, use it to generate audio
    if (_xmp_context) {
        return MixIT(data, len);
    }

    // Otherwise, use the WAV buffer
    if (!_buffer || _read >= _decoded) {
        return 0;
    }

    int remaining = _decoded - _read;
    int mix_len = (len < remaining) ? len : remaining;

    // Apply volume
    float volume = _volume;
    if (_type == eSoundMusic && !_system->GetMixMusic()) {
        volume = 0.0f;
    }

    // Mix the audio (simple mixing for 8-bit unsigned audio)
    SDL_AudioSpec *spec = _system->GetSpec();
    if (spec->format == AUDIO_U8) {
        for (int i = 0; i < mix_len; i++) {
            int sample = data[i] + (int)((_buffer[_read + i] - 128) * volume);
            if (sample > 255) sample = 255;
            if (sample < 0) sample = 0;
            data[i] = (Uint8)sample;
        }
    }
    // Mix for 16-bit signed audio (most common format)
    else if (spec->format == AUDIO_S16SYS) {
        Sint16 *out = (Sint16*)data;
        Sint16 *in = (Sint16*)(_buffer + _read);
        int samples = mix_len / 2; // 16-bit = 2 bytes per sample

        for (int i = 0; i < samples; i++) {
            int sample = out[i] + (int)(in[i] * volume);
            if (sample > 32767) sample = 32767;
            if (sample < -32768) sample = -32768;
            out[i] = (Sint16)sample;
        }

        mix_len = samples * 2; // Convert back to bytes
    }
    // For other formats, just copy with volume adjustment
    else {
        for (int i = 0; i < mix_len; i++) {
            data[i] = (Uint8)(_buffer[_read + i] * volume);
        }
    }

    _read += mix_len;

    // Loop if needed
    if (_read >= _decoded && _loop) {
        _read = 0;
        if (_loop > 0) _loop--;
    } else if (_read >= _decoded) {
        _isPlaying = 0;
    }

    return mix_len;
}

int SourceMusic::MixIT(Uint8 *data, int len) {
    if (!_xmp_context) {
        return 0;
    }

    // Apply volume
    float volume = _volume;
    if (_type == eSoundMusic && !_system->GetMixMusic()) {
        volume = 0.0f;
    }

    // Generate audio from libxmp
    int ret = xmp_play_buffer(_xmp_context, data, len, 0);

    if (ret < 0) {
        fprintf(stderr, "[error] Error playing IT file: %s\n", xmp_get_error(_xmp_context));
        return 0;
    }

    // Apply volume to the generated audio
    SDL_AudioSpec *spec = _system->GetSpec();
    if (spec->format == AUDIO_S16SYS) {
        Sint16 *buffer = (Sint16*)data;
        int samples = len / 2; // 16-bit = 2 bytes per sample

        for (int i = 0; i < samples; i++) {
            buffer[i] = (Sint16)(buffer[i] * volume);
        }
    } else if (spec->format == AUDIO_U8) {
        for (int i = 0; i < len; i++) {
            data[i] = (Uint8)(data[i] * volume);
        }
    }

    // Check if playback has finished
    xmp_frame_info fi;
    xmp_get_frame_info(_xmp_context, &fi);  // Remove the assignment to ret

    if (fi.loop_count > 0) {
        if (_loop) {
            xmp_restart_module(_xmp_context);
            if (_loop > 0) _loop--;
        } else {
            _isPlaying = 0;
        }
    }

    return len;
}

void SourceMusic::Idle(void) {
    // Nothing to do for this implementation
    // as we load the entire file at once
    if (!_isPlaying) {
        return;
    }

    // Check if we've reached the end of the music
    if (_buffer && _read >= _decoded && !_loop) {
        _isPlaying = 0;
    }
}

} // namespace Sound
