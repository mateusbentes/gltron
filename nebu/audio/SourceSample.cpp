#include "audio/nebu_SourceSample.h"

#include "base/nebu_assert.h"
#include <string.h>

#include "base/nebu_debug_memory.h"

namespace Sound {
  SourceSample::SourceSample(System *system) { 
    _system = system;

    _buffer = NULL;
    _buffersize = 8192;

    _position = 0;
    _decoded = 0;
  }

  SourceSample::~SourceSample() {
    // fprintf(stderr, "nebu_SourceSample destructor called\n");
    if(_buffer)
      delete[] _buffer;  // Fixed: use delete[] for arrays
    // Source::~Source();
  }

  void SourceSample::Load(char *filename) {
#define BUFSIZE 1024 * 1024
    SDL_RWops *rwops;

    rwops = SDL_RWFromFile(filename, "rb");
    
    // Stub implementation - no actual sample loading
    fprintf(stderr, "[warning] Sample loading disabled (SDL_sound not available): %s\n", filename);
    
    // Create an empty buffer
    _buffer = new Uint8[_buffersize];
    memset(_buffer, 0, _buffersize);
    
    if (rwops) {
        SDL_RWclose(rwops);
    }
    
    _position = 0;
  }

  int SourceSample::Mix(Uint8 *data, int len) {
    if(_buffer == NULL)
      return 0;

    int volume = (int)(_volume * SDL_MIX_MAXVOLUME);
    nebu_assert(len < _buffersize);

    if(len < _buffersize - _position) {
      SDL_MixAudio(data, _buffer + _position, len, volume);
      _position += len;
    } else { 
      SDL_MixAudio(data, _buffer + _position, _buffersize - _position,
           volume);
      len -= _buffersize - _position;

      // printf("end of sample reached!\n");
      if(_loop) {
        if(_loop != 255) 
          _loop--;

        _position = 0;
        SDL_MixAudio(data, _buffer + _position, len, volume);
        _position += len;
      } else {
        _isPlaying = 0;
      }
    }
    return 1;
  }
}
