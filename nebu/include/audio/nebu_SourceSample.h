#ifndef NEBU_Sound_SourceSample_H
#define NEBU_Sound_SourceSample_H

#include "nebu_Sound.h"

#include "nebu_Source.h"
#include "nebu_SoundSystem.h"

namespace Sound {
  // Forward declaration for friendship
  class SourceSample : public Source {
    public:
      SourceSample(System *system);
      virtual ~SourceSample();
      bool Load(char *filename);  // Changed from void to bool
      virtual int Mix(Uint8 *data, int len);
      
      // Accessor methods for Source3D
      Uint8* GetBuffer() const { return _buffer; }
      int GetBufferSize() const { return _buffersize; }
      
    private:
      Uint8 *_buffer;
      int _buffersize;
      int _position;
      int _decoded;
      
      // Make Source3D a friend so it can access private members
      friend class Source3D;
    };
}
#endif
