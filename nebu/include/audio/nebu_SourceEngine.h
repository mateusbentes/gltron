#ifndef NEBU_SOUND_SOURCE_ENGINE_H
#define NEBU_SOUND_SOURCE_ENGINE_H

#include "audio/nebu_Source3D.h"
#include "audio/nebu_SourceSample.h" // Include the missing header

namespace Sound {
  class SourceEngine : public Source3D {
    public:
      SourceEngine(System* system, SourceSample* source) : Source3D(system, source) {
        _speedShift = 1.0f;
        _pitchShift = 1.0f;
      }
      
      void GetModifiers(float& fPan, float& fVolume, float& fShift);
      
      void SetSpeedShift(float fSpeed) { _speedShift = fSpeed; }
      void SetPitchShift(float fPitch) { _pitchShift = fPitch; }
      float GetSpeedShift() const { return _speedShift; }
      float GetPitchShift() const { return _pitchShift; }
      
    private:
      float _speedShift;
      float _pitchShift;
    };
}

#endif
