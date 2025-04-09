#ifndef _NEBU_SOUND_SYSTEM_H
#define _NEBU_SOUND_SYSTEM_H

// Include SDL2 header to get Uint8 definition
#include <SDL2/SDL.h>

// Forward declare SDL_AudioSpec to avoid including SDL headers in the header file
struct SDL_AudioSpec;

// Include Vector3 definition
#include "base/nebu_Vector3.h"

// Forward declare c_callback to ensure it's available
extern "C" {
  void c_callback(void *userdata, Uint8 *stream, int len);
}

#include "audio/nebu_Source.h"
#include "base/nebu_util.h"  // Changed from base/nebu_List.h to base/nebu_util.h

namespace Sound {
  enum { 
    eUninitialized = 0,
    eInitialized = 1
  };

  class Listener {
  public:
    Listener() : _isValid(false) {};
    ~Listener() {};

    // Add the missing members
    bool _isValid;
    Vector3 _location;
    Vector3 _velocity;
    Vector3 _direction;
    Vector3 _up;
  };

  class System {
  public:
    System(SDL_AudioSpec *spec); 
    ~System();
    typedef void(*Audio_Callback)(void *userdata, Uint8* data, int len);
    Audio_Callback GetCallback() { return c_callback; };
    void Callback(Uint8* data, int len);
    void Idle(); /* remove dead sound sources */
    void AddSource(Source* source);
    void RemoveSource(Source* source);
    
    Listener& GetListener() { return _listener; };
    void SetMixMusic(int value) { _mix_music = value; };
    void SetMixFX(int value) { _mix_fx = value; };
    void SetStatus(int eStatus) { _status = eStatus; };

  private:
    SDL_AudioSpec *_spec;
    Listener _listener;
    nebu_List *_sources;
    int _mix_music;
    int _mix_fx;
    int _status;
  };
}

#endif /* _NEBU_SOUND_SYSTEM_H */
