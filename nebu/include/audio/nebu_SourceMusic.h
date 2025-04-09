#ifndef NEBU_Sound_SourceMusic_H
#define NEBU_Sound_SourceMusic_H

#include "nebu_Sound.h"

#include "nebu_Source.h"
#include "nebu_SoundSystem.h"

#include <SDL.h>

namespace Sound {
  class SourceMusic : public Source {
  public:
    SourceMusic(System *system);
    virtual ~SourceMusic();
    void Load(char *filename);
    virtual int Mix(Uint8 *data, int len);
    virtual void Idle(void);

  protected:
    virtual void Reset() { 
      // Stub implementation
      _read = 0;
      _decoded = 0;
    }
      
  private:
    void CreateSample(void);
    void CleanUp(void);

    // Buffer management
    Uint8* _buffer;
    int _buffersize;
    int _sample_buffersize;
    int _read;
    int _decoded;

    // File management
    char *_filename;
    SDL_RWops *_rwops;
  };
}
#endif
