#ifndef NEBU_SOUND_SOURCE_MUSIC_H
#define NEBU_SOUND_SOURCE_MUSIC_H

// Include SDL2 headers instead of SDL
#include <SDL2/SDL.h>

// Forward declaration for libxmp context
typedef struct xmp_context xmp_context;

#include "audio/nebu_Source.h"

namespace Sound {
  class SourceMusic : public Source {
  public:
    SourceMusic(System *system);
    ~SourceMusic();

    void Load(char *filename);
    void CleanUp(void);
    void CreateSample(void);

    virtual int Mix(Uint8 *data, int len);
    virtual void Idle(void);

    // Override GetBuffer and GetBufferSize to provide access to the buffer
    virtual Uint8* GetBuffer() { return _buffer; }
    virtual int GetBufferSize() { return _buffersize; }

    // Add these new methods for IT file support
    void LoadWAV(char *filename);
    void LoadIT(char *filename);
    int MixIT(Uint8 *data, int len);

  private:
    Uint8 *_buffer;
    int _buffersize;
    int _sample_buffersize;

    int _decoded;
    int _read;

    char *_filename;
    SDL_RWops *_rwops;

    // Add this for libxmp support
    xmp_context *_xmp_context;
  };
}

#endif
