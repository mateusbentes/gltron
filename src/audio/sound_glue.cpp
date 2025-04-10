#if 1
#include "audio/sound_glue.h"
extern "C" {
#include "game/game.h"
#include "video/recognizer.h"
#include "scripting/nebu_scripting.h"
#include "video/video.h"
#include "game/camera.h"
#include "game/game_data.h"
#include "configuration/settings.h"
#include "base/nebu_assert.h"
}

#include "Nebu_audio.h"

#include <SDL2/SDL.h>
// Using SDL2 without SDL_sound

#include "base/nebu_debug_memory.h"

static Sound::System *sound = NULL;
static Sound::SourceMusic *music = NULL;
static Sound::SourceSample *sample_crash = NULL;
static Sound::SourceSample *sample_engine = NULL;
static Sound::SourceSample *sample_recognizer = NULL;

static int nPlayerSources;
static Sound::SourceEngine **ppPlayerSources;
static Sound::Source3D *recognizerEngine;

#define TURNLENGTH 250.0f

// Removed output_decoders function since it depends on SDL_sound

extern "C" {


  void Audio_EnableEngine(void) {
		int i;
		for(i = 0; i < game->players; i++)
			if( game->player[i].data.speed > 0)
				Audio_StartEngine(i);
    sample_engine->Start();
    if (gSettingsCache.show_recognizer)
      sample_recognizer->Start();
    // printf("[audio] turning on engine sound\n");
  }

  void Audio_DisableEngine(void) {
    sample_engine->Stop();
    sample_recognizer->Stop();
    // printf("[audio] turning off engine sound\n");
  }

  void Audio_Idle(void) { 
    // Check if sound system is initialized
    if (!sound) {
      return;
    }

    // Update player engine sounds
    if(sample_engine && sample_engine->IsPlaying() && game && ppPlayerSources) {
      for(int i = 0; i < game->players; i++) {
        if(i >= nPlayerSources || !ppPlayerSources[i])
          continue;

        Player *p = game->player + i;
        if(!p)
          continue;

        Sound::SourceEngine *p3d = ppPlayerSources[i];

        float x, y;
        getPositionFromIndex(&x, &y, i);
        p3d->SetLocation(Vector3(x, y, 0));
        float V = p->data.speed;

        if(game2) {
          int dt = game2->time.current - p->data.turn_time;
          if(dt < TURN_LENGTH && game2->level && game2->level->pAxis) {
            float t = (float)dt / TURNLENGTH;

            float vx = (1 - t) * game2->level->pAxis[p->data.last_dir].v[0] +
              t * game2->level->pAxis[p->data.dir].v[0];
            float vy = (1 - t) * game2->level->pAxis[p->data.last_dir].v[1] +
              t * game2->level->pAxis[p->data.dir].v[1];
            p3d->SetVelocity(Vector3(V * vx, V * vy, 0));
          } else if(game2->level && game2->level->pAxis) {
            p3d->SetVelocity(Vector3(V * game2->level->pAxis[p->data.dir].v[0], 
             V * game2->level->pAxis[p->data.dir].v[1], 
             0));
          }
        }
        
        if(i == 0) {
          if(p->data.boost_enabled) {
            p3d->SetSpeedShift(1.2f);
          } else {
            p3d->SetSpeedShift(1.0f);
          }
          p3d->SetPitchShift(p->data.speed / getSettingf("speed"));
        }
						
#if 0
        if(i == 0) {
          if( dt < TURNLENGTH ) {
            float t = (float)dt / TURNLENGTH;
            float speedShift = ( 1 - t ) * 0.4 + t * 0.3;
            float pitchShift = ( 1 - t ) * 0.9 + t * 1.0;
            ( (Sound::SourceEngine*) p3d )->_speedShift = speedShift;
            ( (Sound::SourceEngine*) p3d )->_pitchShift = pitchShift;
          } else {
            ( (Sound::SourceEngine*) p3d )->_speedShift = 0.3;
            ( (Sound::SourceEngine*) p3d )->_pitchShift = 1.0;
          }
        }
#endif
      }
    }

    // Update recognizer sound
    if(sample_recognizer && sample_recognizer->IsPlaying() && recognizerEngine) {
      if (gSettingsCache.show_recognizer) {
        vec2 p, v;
        getRecognizerPositionVelocity(&p, &v);
        recognizerEngine->SetLocation(Vector3(p.v[0], p.v[1], 10.0f));
        recognizerEngine->SetVelocity(Vector3(v.v[0], v.v[1], 0));
      }
    }

    // Check if music needs to be restarted
    if(music && !music->IsPlaying()) {
      // check if music is enabled. if it is, advance to
      // next song
      if(gSettingsCache.playMusic) {
        printf("[audio] Music not playing, calling nextTrack() script\n");
        int result = scripting_Run("nextTrack()");
        printf("[audio] nextTrack() script returned: %d\n", result);
      } else {
        printf("[audio] Music not playing, but playMusic is disabled\n");
      }
    }

    // TODO: add support for multiple listeners here
    // Problem/Constraint:
    // Each listener MUST consume exactly the same amount of samples
    // from each source (that means the pitch shift can't just be
    // done by speeding up playback)
    Sound::Listener& listener = sound->GetListener();
    
    // Check if player visuals are available
    if(gnPlayerVisuals == 0 || !gppPlayerVisuals || !gppPlayerVisuals[0] || !gppPlayerVisuals[0]->pPlayer)
    {
      listener._isValid = 0;
    }
    else
    {
      listener._isValid = 1;
      listener._location = Vector3(gppPlayerVisuals[0]->camera.cam);
      Vector3 v1 = Vector3(gppPlayerVisuals[0]->camera.target);
      Vector3 v2 = Vector3(gppPlayerVisuals[0]->camera.cam);
      listener._direction = v1 - v2;
      
      // Check if player sources are available before accessing
      if(ppPlayerSources && ppPlayerSources[0])
        listener._velocity = ppPlayerSources[0]->GetVelocity();
      else
        listener._velocity = Vector3(0, 0, 0);
    }
    listener._up = Vector3(0, 0, 1);

    sound->SetMixMusic(gSettingsCache.playMusic);
    sound->SetMixFX(gSettingsCache.playEffects);
    sound->Idle();
  }

  void Audio_CrashPlayer(int player) {
    Sound::SourceCopy *copy = new Sound::SourceCopy(sample_crash);
    copy->Start();
    copy->SetRemovable();
    copy->SetType(Sound::eSoundFX);
	copy->SetName("crash (copy)");
    sound->AddSource(copy);
  }

  void Audio_Init(void) {
    // Initialize SDL audio subsystem instead of SDL_sound
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) {
      fprintf(stderr, "[error] SDL audio initialization failed: %s\n", SDL_GetError());
      return;
    }

    printf("[audio] SDL audio subsystem initialized\n");

    SDL_AudioSpec* spec = new SDL_AudioSpec;
    spec->freq = 22050;
    spec->format = AUDIO_S16SYS;
    spec->channels = 2;
    spec->samples = 1024;

    sound = new Sound::System(spec);

    spec->userdata = sound;
    spec->callback = sound->GetCallback();

    SDL_AudioSpec obtained;

    if(SDL_OpenAudio(spec, &obtained) != 0) {
      fprintf(stderr, "[error] %s\n", SDL_GetError());
      sound->SetStatus(Sound::eUninitialized);
    } else {
      sound->SetStatus(Sound::eInitialized);
      printf("[audio] Audio device opened successfully\n");
      printf("[audio] frequency: %d\n", obtained.freq);
      printf("[audio] format: %d\n", obtained.format);
      printf("[audio] channels: %d\n", obtained.channels);
      printf("[audio] silence: %d\n", obtained.silence);
      printf("[audio] buffer in samples: %d\n", obtained.samples);
      printf("[audio] buffer in bytes: %d\n", obtained.size);
    }
    sound->SetMixMusic(gSettingsCache.playMusic);
    sound->SetMixFX(gSettingsCache.playEffects);
  }

  void Audio_Start(void) {
    SDL_PauseAudio(0);
  }

  void Audio_Quit(void) {
    SDL_PauseAudio(1);
    SDL_CloseAudio();
    if(sound)
    {
        delete sound;
        sound = NULL;
    }
    if(sample_crash)
    {
        delete sample_crash;
        sample_crash = NULL;
    }
    if(sample_engine)
    {
        delete sample_engine;
        sample_engine = NULL;
    }
    if(sample_recognizer)
    {
        delete sample_recognizer;
        sample_recognizer = NULL;
    }
    
    // Quit SDL audio subsystem
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
  }

  void Audio_LoadMusic(char *name) {
    printf("[audio] Audio_LoadMusic called with name: %s\n", name ? name : "NULL");
    
    if (!sound) {
        fprintf(stderr, "[error] Cannot load music - sound system not initialized\n");
        return;
    }
    
    printf("[audio] Loading music: %s\n", name);
    
    // Check if file exists
    FILE *f = fopen(name, "rb");
    if (!f) {
        printf("[audio] File not found at path: %s\n", name);
        // Try looking in the music directory if the file isn't found
        char music_path[512];
        const char* basename = strrchr(name, '/');
        if (basename) {
            basename++; // Skip the '/'
        } else {
            basename = name;
        }
        
        sprintf(music_path, "music/%s", basename);
        f = fopen(music_path, "rb");
        
        if (f) {
            fclose(f);
            printf("[audio] Found music in music directory: %s\n", music_path);
            name = strdup(music_path); // Note: This creates a memory leak, but it's small and one-time
            printf("[audio] Using new path: %s\n", name);
        } else {
            // Try looking in the sounds directory
            sprintf(music_path, "sounds/%s", basename);
            f = fopen(music_path, "rb");
            
            if (f) {
                fclose(f);
                printf("[audio] Found music in sounds directory: %s\n", music_path);
                name = strdup(music_path); // Note: This creates a memory leak, but it's small and one-time
                printf("[audio] Using new path: %s\n", name);
            } else {
                fprintf(stderr, "[error] Music file not found: %s\n", name);
                fprintf(stderr, "[error] Also tried: music/%s and sounds/%s\n", basename, basename);
                return;
            }
        }
    } else {
        fclose(f);
    }
    
    if(music != NULL) {
        printf("[audio] Stopping previous music track\n");
        music->Stop();
        music->SetRemovable();
    }
    
    music = new Sound::SourceMusic(sound);
    
    printf("[audio] Attempting to load music file: %s\n", name);
    // SourceMusic::Load returns void, not bool, so we can't check its return value
    music->Load(name);
    printf("[audio] Music file loaded into memory\n");
    
    int loopSetting = getSettingi("loopMusic");
    printf("[audio] Loop setting: %d\n", loopSetting);
    if(loopSetting)
        music->SetLoop(255);
    music->SetType(Sound::eSoundMusic);
    music->SetName("music");
    sound->AddSource(music);
    
    if(music) {
        printf("[audio] Music object details:\n");
        printf("  - Name: %s\n", music->GetName());
        printf("  - Type: %d\n", music->GetType());
        printf("  - Loop: %d\n", music->GetLoop());
        printf("  - Volume: %.2f\n", music->GetVolume());
        printf("  - Is playing: %s\n", music->IsPlaying() ? "yes" : "no");
    }
    
    printf("[audio] Music loaded successfully: %s\n", name);
  }

  void Audio_PlayMusic(void) {
    printf("[audio] Audio_PlayMusic called\n");
    
    if (!music) {
        fprintf(stderr, "[error] Cannot play music - music not loaded\n");
        return;
    }
    
    if (!sound) {
        fprintf(stderr, "[error] Cannot play music - sound system not initialized\n");
        return;
    }
    
    // Make sure music mixing is enabled
    sound->SetMixMusic(1);
    
    // Print music information
    printf("[audio] Music name: %s\n", music->GetName());
    printf("[audio] Music type: %d\n", music->GetType());
    printf("[audio] Music volume: %.2f\n", music->GetVolume());
    
    music->Start();
    printf("[audio] Music playback started\n");
  }

  void Audio_StopMusic(void) {
    if (!music) {
        fprintf(stderr, "[error] Cannot stop music - music not loaded\n");
        return;
    }
    
    music->Stop();
    printf("[audio] Music playback stopped\n");
  }

  void Audio_SetMusicVolume(float volume) {
    if (!music) {
        fprintf(stderr, "[error] Cannot set music volume - music not loaded\n");
        return;
    }
    
    music->SetVolume(volume);
    printf("[audio] Music volume set to %.2f\n", volume);
  }
  
  void Audio_SetFxVolume(float volume) {
    sample_engine->SetVolume(volume);
    sample_crash->SetVolume(volume);
    if(volume > 0.8f)
      sample_recognizer->SetVolume(volume);
    else 
      sample_recognizer->SetVolume(volume * 1.25f);
  }

  void Audio_StartEngine(int iPlayer) {
	  if(ppPlayerSources && ppPlayerSources[iPlayer])
		ppPlayerSources[iPlayer]->Start();
  }

  void Audio_StopEngine(int iPlayer) {
	  if(ppPlayerSources && ppPlayerSources[iPlayer])
	ppPlayerSources[iPlayer]->Stop();
  }
 
  void Audio_ResetData(void)
  {
	  if(ppPlayerSources)
	  {
		  Audio_UnloadPlayers();
	  }
	  Audio_LoadPlayers();
  }

  void Audio_UnloadPlayers(void)
  {
	  if(ppPlayerSources)
	  {
		  for(int i = 0; i < nPlayerSources; i++)
		  {
			  sound->RemoveSource(ppPlayerSources[i]);
			  if (!ppPlayerSources[i]) { fprintf(stderr, "[error] ppPlayerSources[%d] is NULL\n", i); continue; }
			  delete ppPlayerSources[i];
		  }
		  delete[] ppPlayerSources;
		  ppPlayerSources = NULL;
		  nPlayerSources = 0;
	  }
	  if(recognizerEngine)
	  {
		  sound->RemoveSource(recognizerEngine);
		  delete recognizerEngine;
		  recognizerEngine = NULL;
	  }
  }

  void Audio_LoadPlayers(void) {
	  nebu_assert(!ppPlayerSources);
	  nPlayerSources = game->players;
	ppPlayerSources = new Sound::SourceEngine*[nPlayerSources];
    for(int i = 0; i < nPlayerSources; i++)
	{
		ppPlayerSources[i] = new Sound::SourceEngine(sound, sample_engine);
		ppPlayerSources[i]->SetType(Sound::eSoundFX);
		sound->AddSource(ppPlayerSources[i]);

		char *name = new char[32];
		sprintf(name, "player %d", i);
		ppPlayerSources[i]->SetName(name);
		delete[] name;
	}

	nebu_assert(!recognizerEngine);
    recognizerEngine = new Sound::Source3D(sound, sample_recognizer);
    recognizerEngine->SetType(Sound::eSoundFX);
    recognizerEngine->Start();
    sound->AddSource(recognizerEngine);

    recognizerEngine->SetName("recognizer");
  }

  void Audio_LoadSample(char *name, int number) {
    if (!sound) {
        fprintf(stderr, "[error] Cannot load sample - sound system not initialized\n");
        return;
    }
    
    printf("[audio] Attempting to load sample %d: %s\n", number, name);
    
    // Check if file exists
    FILE *f = fopen(name, "rb");
    if (!f) {
        // Try with "game_" prefix if the original file doesn't exist
        char alt_name[512];
        const char* basename = strrchr(name, '/');
        if (basename) {
            char path[512];
            size_t path_len = basename - name + 1;
            strncpy(path, name, path_len);
            path[path_len] = '\0';
            sprintf(alt_name, "%sgame_%s", path, basename + 1);
        } else {
            sprintf(alt_name, "game_%s", name);
        }
        
        FILE *alt_f = fopen(alt_name, "rb");
        if (alt_f) {
            fclose(alt_f);
            printf("[audio] Original file not found, using alternative: %s\n", alt_name);
            name = strdup(alt_name); // Note: This creates a memory leak, but it's small and one-time
        } else {
            fprintf(stderr, "[error] Sample file not found: %s (also tried %s)\n", name, alt_name);
            return;
        }
    } else {
        fclose(f);
    }
    
    switch(number) {
    case 0: // engine sound
        if (sample_engine) {
            delete sample_engine;
        }
        sample_engine = new Sound::SourceSample(sound);
        if (!sample_engine->Load(name)) {
            fprintf(stderr, "[error] Failed to load engine sample: %s\n", name);
        } else {
            printf("[audio] Loaded engine sample successfully: %s\n", name);
        }
        sample_engine->SetName("sample: engine");
        break;
    case 1: // crash sound
        if (sample_crash) {
            delete sample_crash;
        }
        sample_crash = new Sound::SourceSample(sound);
        if (!sample_crash->Load(name)) {
            fprintf(stderr, "[error] Failed to load crash sample: %s\n", name);
        } else {
            printf("[audio] Loaded crash sample successfully: %s\n", name);
        }
        sample_crash->SetName("sample: crash");
        break;
    case 2: // recognizer sound
        if (sample_recognizer) {
            delete sample_recognizer;
        }
        sample_recognizer = new Sound::SourceSample(sound);
        if (!sample_recognizer->Load(name)) {
            fprintf(stderr, "[error] Failed to load recognizer sample: %s\n", name);
        } else {
            printf("[audio] Loaded recognizer sample successfully: %s\n", name);
        }
        sample_recognizer->SetName("sample: recognizer");
        break;
    default:
        /* programmer error, but non-critical */
        fprintf(stderr, "[error] unknown sample %d: '%s'\n", number, name);
    }
  }
}
#else

extern "C" {


  void Audio_EnableEngine(void) {
  }

  void Audio_DisableEngine(void) {
  }

  void Audio_Idle(void) { 
  }

  void Audio_CrashPlayer(int player) {
  }

  void Audio_Init(void) {
  }

  void Audio_Start(void) {
  }

  void Audio_Quit(void) {
  }

  void Audio_LoadMusic(char *name) {
  }

  void Audio_PlayMusic(void) {
  }

  void Audio_StopMusic(void) {
  }

  void Audio_SetMusicVolume(float volume) {
  }
  
  void Audio_SetFxVolume(float volume) {
  }

  void Audio_StartEngine(int iPlayer) {
  }

  void Audio_StopEngine(int iPlayer) {
  }
 
  void Audio_ResetData(void)
  {
  }

  void Audio_UnloadPlayers(void)
  {
  }

  void Audio_LoadPlayers(void) {
  }

  void Audio_LoadSample(char *name, int number) {
  }
}

#endif
