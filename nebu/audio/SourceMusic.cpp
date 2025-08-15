#include "audio/nebu_SourceMusic.h"
#include "audio/nebu_SoundSystem.h"
#include <SDL2/SDL.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

// Include libxmp headers
#include <xmp.h>

// Add these declarations if they're not in the header
#ifndef XMP_STATE_ENDED
#define XMP_STATE_ENDED 3
#endif

#include "base/nebu_debug_memory.h"

namespace Sound {

SourceMusic::SourceMusic(System *system) {
    _system = system;
    _buffer = nullptr;
    _buffersize = 0;
    _sample_buffersize = 0;
    _decoded = 0;
    _read = 0;
    _filename = nullptr;
    _rwops = nullptr;
    _xmp_context = nullptr;
    _loaded = false;
    _isPlaying = false;
    Reset();
}

SourceMusic::~SourceMusic() {
    CleanUp();
}

void SourceMusic::CreateSample() {
    // Not used in this implementation
}

void SourceMusic::Load(char *filename) {
    CleanUp();

    // Store filename
    int n = strlen(filename);
    _filename = new char[n + 1];
    memcpy(_filename, filename, n + 1);

    printf("[audio] Loading music file: %s\n", filename);

    // Determine file extension
    const char *ext = strrchr(filename, '.');
    if (ext) {
        ext++;
        if (strcasecmp(ext, "wav") == 0) {
            printf("[audio] Detected WAV format: %s\n", filename);
            LoadWAV(filename);
        } else if (strcasecmp(ext, "it") == 0) {
            printf("[audio] Detected IT format: %s\n", filename);
            if (!LoadIT(filename)) {
                fprintf(stderr, "[error] Failed to load IT file: %s\n", filename);
            }
        } else {
            fprintf(stderr, "[error] Unsupported music format: %s\n", ext);
            fprintf(stderr, "[error] Supported formats: WAV and IT\n");
        }
    } else {
        fprintf(stderr, "[error] No file extension found for music file: %s\n", filename);
    }
}

void SourceMusic::LoadWAV(char *filename) {
    SDL_AudioSpec wav_spec;
    Uint32 wav_length;
    Uint8 *wav_buffer;

    if (!SDL_LoadWAV(filename, &wav_spec, &wav_buffer, &wav_length)) {
        fprintf(stderr, "[error] Could not load WAV file: %s\n", SDL_GetError());
        return;
    }

    SDL_AudioCVT cvt;
    SDL_AudioSpec *spec = _system->GetSpec();

    if (SDL_BuildAudioCVT(&cvt, wav_spec.format, wav_spec.channels, wav_spec.freq,
                          spec->format, spec->channels, spec->freq) < 0) {
        fprintf(stderr, "[error] Could not build audio converter: %s\n", SDL_GetError());
        SDL_FreeWAV(wav_buffer);
        return;
    }

    cvt.len = wav_length;
    cvt.buf = new Uint8[cvt.len * cvt.len_mult];
    memcpy(cvt.buf, wav_buffer, wav_length);

    if (SDL_ConvertAudio(&cvt) < 0) {
        fprintf(stderr, "[error] Could not convert audio: %s\n", SDL_GetError());
        delete[] cvt.buf;
        SDL_FreeWAV(wav_buffer);
        return;
    }

    SDL_FreeWAV(wav_buffer);

    _buffer = cvt.buf;
    _buffersize = cvt.len_cvt;
    _decoded = _buffersize;
    _read = 0;
    _isPlaying = true;

    printf("[audio] WAV loaded and converted successfully - size: %d bytes\n", _buffersize);
}

bool SourceMusic::LoadIT(const char* filename) {
    CleanUp();

    xmp_context ctx = xmp_create_context();
    if (!ctx) {
        fprintf(stderr, "[error] Failed to create xmp context\n");
        return false;
    }

    if (xmp_load_module(ctx, filename) != 0) {
        fprintf(stderr, "[error] Failed to load module: %s\n", filename);
        xmp_free_context(ctx);
        return false;
    }

    xmp_set_player(ctx, XMP_PLAYER_VOLUME, 100);
    xmp_set_player(ctx, XMP_PLAYER_INTERP, XMP_INTERP_LINEAR);
    xmp_set_player(ctx, XMP_PLAYER_DSP, 1);

    if (xmp_start_player(ctx, 44100, 0) != 0) {
        fprintf(stderr, "[error] Failed to start player\n");
        xmp_free_context(ctx);
        return false;
    }

    _xmp_context = ctx;
    _loaded = true;
    _isPlaying = true;
    return true;
}

void SourceMusic::CleanUp() {
    if (_buffer) { delete[] _buffer; _buffer = nullptr; }
    if (_filename) { delete[] _filename; _filename = nullptr; }
    if (_rwops) { SDL_RWclose(_rwops); _rwops = nullptr; }

    if (_xmp_context) {
        xmp_end_player(_xmp_context);
        xmp_release_module(_xmp_context);
        xmp_free_context(_xmp_context);
        _xmp_context = nullptr;
    }

    _buffersize = 0;
    _sample_buffersize = 0;
    _decoded = 0;
    _read = 0;
    _loaded = false;
    _isPlaying = false;
}

int SourceMusic::Mix(Uint8 *data, int len) {
    if (!_isPlaying || (!_buffer && !_xmp_context)) return 0;

    if (_xmp_context) return MixIT(data, len);

    if (!_buffer || _read >= _decoded) return 0;

    int remaining = _decoded - _read;
    int mix_len = (len < remaining) ? len : remaining;
    float volume = (_type == eSoundMusic && !_system->GetMixMusic()) ? 0.0f : _volume;

    SDL_AudioSpec *spec = _system->GetSpec();
    if (spec->format == AUDIO_U8) {
        for (int i = 0; i < mix_len; i++) {
            int sample = data[i] + (int)((_buffer[_read + i] - 128) * volume);
            if (sample > 255) sample = 255;
            if (sample < 0) sample = 0;
            data[i] = (Uint8)sample;
        }
    } else if (spec->format == AUDIO_S16SYS) {
        Sint16 *out = (Sint16*)data;
        Sint16 *in = (Sint16*)(_buffer + _read);
        int samples = mix_len / 2;

        for (int i = 0; i < samples; i++) {
            int sample = out[i] + (int)(in[i] * volume);
            if (sample > 32767) sample = 32767;
            if (sample < -32768) sample = -32768;
            out[i] = (Sint16)sample;
        }

        mix_len = samples * 2;
    } else {
        for (int i = 0; i < mix_len; i++) {
            data[i] = (Uint8)(_buffer[_read + i] * volume);
        }
    }

    _read += mix_len;

    if (_read >= _decoded) {
        if (_loop) {
            _read = 0;
            if (_loop > 0) _loop--;
            _isPlaying = true;
        } else {
            _isPlaying = false;
        }
    }

    return mix_len;
}

int SourceMusic::MixIT(Uint8 *data, int len) {
    if (!_xmp_context) return 0;

    float volume = (_type == eSoundMusic && !_system->GetMixMusic()) ? 0.0f : _volume;

    int ret = xmp_play_buffer(_xmp_context, data, len, 0);
    if (ret < 0) {
        const char* errMsg;
        switch (ret) {
            case -XMP_ERROR_FORMAT: errMsg = "Unrecognized module format"; break;
            case -XMP_ERROR_LOAD:   errMsg = "Module recognized but failed to load"; break;
            case -XMP_ERROR_DEPACK: errMsg = "Module decompression failed"; break;
            case -XMP_ERROR_SYSTEM: errMsg = strerror(errno); break;
            default:                errMsg = "Unknown libxmp error"; break;
        }
        fprintf(stderr, "[error] Error playing IT file: %s (code %d)\n", errMsg, ret);
        return 0;
    }

    SDL_AudioSpec *spec = _system->GetSpec();
    if (spec->format == AUDIO_S16SYS) {
        Sint16 *buffer = (Sint16*)data;
        int samples = len / 2;
        for (int i = 0; i < samples; i++) buffer[i] = (Sint16)(buffer[i] * volume);
    } else if (spec->format == AUDIO_U8) {
        for (int i = 0; i < len; i++) data[i] = (Uint8)(data[i] * volume);
    }

    xmp_frame_info fi;
    xmp_get_frame_info(_xmp_context, &fi);

    if (fi.loop_count > 0) {
        if (_loop) {
            xmp_restart_module(_xmp_context);
            _isPlaying = true;
            if (_loop > 0) _loop--;
        } else {
            _isPlaying = false;
        }
    }

    return len;
}

void SourceMusic::Idle() {
    if (!_isPlaying) return;

    if (_buffer && _read >= _decoded && !_loop) {
        _isPlaying = false;
    }
}

} // namespace Sound
