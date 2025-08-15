#ifndef NEBU_SOURCEMUSIC_H
#define NEBU_SOURCEMUSIC_H

#include "audio/nebu_Source.h"
#include <xmp.h>

namespace Sound {

class SourceMusic : public Source {
public:
    SourceMusic(System *system);
    ~SourceMusic();

    void CreateSample(void);
    void Load(char *filename);
    void LoadWAV(char *filename);
    bool LoadIT(const char* filename);
    void CleanUp(void);
    int Mix(Uint8 *data, int len);
    int MixIT(Uint8 *data, int len);
    void Idle(void);

    // Member variables
    xmp_context _xmp_context;
    bool _loaded;

private:
    Uint8 *_buffer;
    int _buffersize;
    int _sample_buffersize;

    int _decoded;
    int _read;

    char *_filename;
    SDL_RWops *_rwops;
};

} // namespace Sound

#endif // NEBU_SOURCEMUSIC_H
