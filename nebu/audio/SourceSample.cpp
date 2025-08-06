#include "audio/nebu_SourceSample.h"
#include "audio/nebu_SoundSystem.h"  // Add this line to include the full System class definition
#include <SDL2/SDL.h>
#include <stdio.h>
#include <string.h>

namespace Sound {

  SourceSample::SourceSample(System *system) { 
    _system = system;
    _buffer = NULL;
    _buffersize = 0;
    _position = 0;
    _decoded = 0;
    Reset();
  }

  SourceSample::~SourceSample() {
    if(_buffer) {
      delete[] _buffer;
      _buffer = NULL;
    }
  }

  bool SourceSample::Load(char *filename) {
    // Use SDL2's built-in WAV loading
    SDL_AudioSpec wav_spec;
    Uint32 wav_length;
    Uint8 *wav_buffer;

    // Load the WAV file
    if (SDL_LoadWAV(filename, &wav_spec, &wav_buffer, &wav_length) == NULL) {
      fprintf(stderr, "[error] Could not load WAV file: %s\n", SDL_GetError());
      return false;
    }

    printf("[audio] Loaded WAV file: %s\n", filename);
    printf("[audio] WAV specs - freq: %d, format: %d, channels: %d\n", 
           wav_spec.freq, wav_spec.format, wav_spec.channels);

    // Convert the audio to the system format if needed
    SDL_AudioCVT cvt;
    SDL_AudioSpec *spec = _system->GetSpec();
    
    if (SDL_BuildAudioCVT(&cvt, wav_spec.format, wav_spec.channels, wav_spec.freq,
                          spec->format, spec->channels, spec->freq) < 0) {
      fprintf(stderr, "[error] Could not build audio converter: %s\n", SDL_GetError());
      SDL_FreeWAV(wav_buffer);
      return false;
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
      return false;
    }

    // Free the original WAV buffer
    SDL_FreeWAV(wav_buffer);

    // Store the converted audio
    if (_buffer) {
      delete[] _buffer;
    }
    
    _buffer = cvt.buf;
    _buffersize = cvt.len_cvt;
    _position = 0;
    _decoded = _buffersize;

    printf("[audio] Sample converted successfully - size: %d bytes\n", _buffersize);
    return true;
  }

  int SourceSample::Mix(Uint8 *data, int len) {
    // Mix the sample into the output buffer
    if (!_isPlaying || !_buffer || _position >= _decoded) {
      return 0;
    }

    int remaining = _decoded - _position;
    int mix_len = (len < remaining) ? len : remaining;
    int volume = (int)(_volume * SDL_MIX_MAXVOLUME);

    // Apply volume based on sound type
    if (_type == eSoundFX && !_system->GetMixFX()) {
      volume = 0;
    } else if (_type == eSoundMusic && !_system->GetMixMusic()) {
      volume = 0;
    }

    // Mix the audio using SDL's built-in mixing function
    SDL_MixAudio(data, _buffer + _position, mix_len, volume);
    _position += mix_len;

    // Handle looping
    if (_position >= _decoded) {
      if (_loop) {
        if (_loop != 255) {
          _loop--;
        }
        _position = 0;
        
        // If we still have data to mix after looping
        if (mix_len < len) {
          SDL_MixAudio(data + mix_len, _buffer, len - mix_len, volume);
          _position += len - mix_len;
        }
      } else {
        _isPlaying = 0;
      }
    }
    
    return 1;
  }
}
