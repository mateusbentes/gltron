#ifndef NEBU_SOUND_SOURCE3D_H
#define NEBU_SOUND_SOURCE3D_H

#include "audio/nebu_Source.h"
#include "audio/nebu_SoundSystem.h"
#include "base/nebu_Vector3.h"

namespace Sound {
  class Source3D : public Source {
  public:
    Source3D(System* system, Source* source) : 
      _system(system), _source(source), _position(0) {
      _location = Vector3(0, 0, 0);
      _velocity = Vector3(0, 0, 0);
    }
    
    ~Source3D() {
      delete _source;
    }
    
    virtual void Start() { _source->Start(); }
    virtual void Stop() { _source->Stop(); }
    virtual void Pause() { _source->Pause(); }
    virtual void UnPause() { _source->UnPause(); }
    virtual void Idle() { _source->Idle(); }
    
    virtual int Mix(Uint8 *data, int len);
    
    virtual Uint8 IsPlaying() { return _source->IsPlaying(); }
    virtual void SetRemovable() { _source->SetRemovable(); }
    virtual Uint8 IsRemovable() { return _source->IsRemovable(); }
    virtual void SetVolume(float volume) { _source->SetVolume(volume); }
    virtual float GetVolume() { return _source->GetVolume(); }
    virtual void SetLoop(Uint8 loop) { _source->SetLoop(loop); }
    virtual Uint8 GetLoop() { return _source->GetLoop(); }
    virtual void SetType(int type) { _source->SetType(type); }
    virtual int GetType() { return _source->GetType(); }
    
    void SetLocation(const Vector3& v) { _location = v; }
    void SetVelocity(const Vector3& v) { _velocity = v; }
    const Vector3& GetLocation() const { return _location; }
    const Vector3& GetVelocity() const { return _velocity; }
    
    void GetModifiers(float& fPan, float& fVolume, float& fShift);
    
  protected:
    // Override Reset to call our own implementation
    virtual void Reset() { Source::Reset(); }
    
    System* _system;
    Source* _source;
    Vector3 _location;
    Vector3 _velocity;
    int _position;
  };
}

#endif
