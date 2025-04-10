// At the top of SourceMusic.cpp, add this include:
#include "audio/nebu_SourceMusic.h"
#include "audio/nebu_SoundSystem.h"  // Add this line to include the full System class definition
#include <SDL2/SDL.h>
#include <string.h>
#include <stdio.h>

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
		fprintf(stderr, "[error] Could not load music file (only WAV supported): %s\n", SDL_GetError());
		fprintf(stderr, "[error] Consider converting your music to WAV format\n");
	}
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
	
	_buffersize = 0;
	_sample_buffersize = 0;
	_decoded = 0;
	_read = 0;
}

int SourceMusic::Mix(Uint8 *data, int len) {
	// Mix the music into the output buffer
	if (!_isPlaying || !_buffer || _read >= _decoded) {
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

void SourceMusic::Idle(void) {
	// Nothing to do for this implementation
	// as we load the entire file at once
	if (!_isPlaying) {
		return;
	}
	
	// Check if we've reached the end of the music
	if (_read >= _decoded && !_loop) {
		_isPlaying = 0;
	}
}

}
