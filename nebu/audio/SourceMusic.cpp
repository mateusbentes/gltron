#include "audio/nebu_SourceMusic.h"

#include <string.h>

#include "base/nebu_debug_memory.h"

namespace Sound {

SourceMusic::SourceMusic(System *system) { 
	_system = system;

	_sample_buffersize = 8192;
	_buffersize = 20 * _sample_buffersize;
	_buffer = new Uint8[_buffersize];
	memset(_buffer, 0, _buffersize);

	_decoded = 0;
	_read = 0;

	_filename = NULL;
	_rwops = NULL;
}

SourceMusic::~SourceMusic()
{
	// fprintf(stderr, "nebu_SourceMusic destructor called\n");
#ifndef macintosh
	// SDL_SemWait(_sem); // Removed semaphore usage
#else
    SDL_LockAudio();
#endif
	if(_buffer)
		delete[] _buffer;  // Fixed: use delete[] for arrays
	
	if(_filename)
		delete[] _filename;  // Fixed: use delete[] for arrays

#ifndef macintosh		
	// SDL_SemPost(_sem); // Removed semaphore usage
#else
    SDL_UnlockAudio();
#endif
}

/*! 
	\fn void SourceMusic::CreateSample(void)
	
	call this function only between semaphores
*/

void SourceMusic::CreateSample(void) {
	_rwops = SDL_RWFromFile(_filename, "rb");
	
	// Stub implementation - no actual sample loading
	fprintf(stderr, "[warning] Music playback disabled (SDL_sound not available)\n");

    _read = 0;
    _decoded = 0;
}

void SourceMusic::Load(char *filename) {
	int n = strlen(filename);
	_filename = new char[n+1];
	memcpy(_filename, filename, n + 1);
	CreateSample();
}

void SourceMusic::CleanUp(void) {
	_read = 0;
    _decoded = 0;

    if(_rwops) {
		SDL_RWclose(_rwops);
		_rwops = NULL;
	}
}

int SourceMusic::Mix(Uint8 *data, int len) {
    // Stub implementation - just return 0 to indicate no mixing was done
    return 0;
}

void SourceMusic::Idle(void) {
	// Stub implementation - nothing to do
	if(!_isPlaying) {
		return;
	}
	
	// Just mark as not playing since we can't actually play music
	_isPlaying = 0;
}

}
