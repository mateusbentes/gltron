#include "audio/nebu_SoundSystem.h"

// Include SDL2 headers instead of SDL
#include <SDL2/SDL.h>
#include <string.h>

#include "base/nebu_debug_memory.h"
#include "base/nebu_util.h"  // Include the header that contains nebu_List declarations

namespace Sound {
  System::System(SDL_AudioSpec *spec) { 
    _spec = spec;
    
    _mix_music = 1; // TODO: add 'master' volume for music and fx
    _mix_fx = 1;

    _sources = nebu_List_Create();

    _status = eUninitialized; // sound system is not initialized
  }
  
  System::~System()
  {
    for(nebu_List *p = _sources; p->next; p = p->next)
    {
      Source* source = static_cast<Source*>(p->data);
      delete source;
    }
    nebu_List_Free(_sources);

    delete _spec;
  }

  void System::Callback(Uint8* data, int len) {
    // ensure silence
    memset(data, _spec->silence, len);

    if(_status == eUninitialized) 
      return;

    nebu_List* p;
    int sources_mixed = 0;
    for(p = _sources; p->next != NULL; p = p->next) {
      Source* s = static_cast<Source*>(p->data);
      if(s->IsPlaying()) {
        if(!((s->GetType() & eSoundFX && !_mix_fx) ||
             (s->GetType() & eSoundMusic && !_mix_music)))
        {
          if(s->Mix(data, len))
            sources_mixed++;
        }
      }
    }
  }


  void System::AddSource(Source* source) { 
    nebu_List_AddTail(_sources, source);
  }

  void System::RemoveSource(Source* source)
  {
    nebu_List *p, *pPrev = NULL;
    for(p = _sources; p->next != NULL; p = p->next)
    {
      if(static_cast<Source*>(p->data) == source)
      {
        nebu_List_RemoveAt(p, pPrev);
        break;
      }
      pPrev = p;
    }
  }

  void System::Idle(void) {
    /* idle processing */
    nebu_List *p, *pNext;
    
    // First call Idle on all sources
    for(p = _sources; p->next != NULL; p = p->next)
    {
      Source *source = static_cast<Source*>(p->data);
      source->Idle();
    }
    
    // Then check for removable sources
    nebu_List *pPrev = NULL;
    p = _sources;
    
    while(p->next != NULL)
    {
      Source *source = static_cast<Source*>(p->data);
      pNext = p->next;
      
      if(source->IsRemovable() && !source->IsPlaying())
      {
        delete source;
        nebu_List_RemoveAt(p, pPrev);
        p = pNext;
      }
      else
      {
        pPrev = p;
        p = pNext;
      }
    }
  }
  
}

extern "C" {
  void c_callback(void *userdata, Uint8 *stream, int len) { 
    static_cast<Sound::System*>(userdata)->Callback(stream, len);
  }
}
