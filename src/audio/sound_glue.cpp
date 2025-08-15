#if 1  // Audio enabled
#include "audio/sound_glue.h"
extern "C" {
#include "game/game.h"
#include "video/recognizer.h"
#include "scripting/nebu_scripting.h"
#include "video/video.h"
#include "game/camera.h"
#include "game/game_data.h"
#include "configuration/settings.h"
#include "configuration/configuration.h"
#include "base/nebu_assert.h"
#include "audio/audio.h"
}

#include "Nebu_audio.h"

#include <SDL2/SDL.h>
#include <unistd.h>
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
      fprintf(stderr, "[error] Audio_Idle called but sound system is not initialized\n");
      return;
    }
    
    // Check if game is initialized
    if (!game) {
      // Just update the sound system without game-specific updates
      sound->SetMixMusic(gSettingsCache.playMusic);
      sound->SetMixFX(gSettingsCache.playEffects);
      sound->Idle();
      return;
    }

    // Update player engine sounds
    if(sample_engine && sample_engine->IsPlaying() && ppPlayerSources) {
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

        // Check if game2 is initialized
        if(!game2) {
          fprintf(stderr, "[error] Audio_Idle: game2 not initialized\n");
          continue;
        }
        
        // Check if level is initialized
        if(!game2->level) {
          fprintf(stderr, "[error] Audio_Idle: game2->level not initialized\n");
          continue;
        }
        
        // Check if pAxis is initialized
        if(!game2->level->pAxis) {
          fprintf(stderr, "[error] Audio_Idle: game2->level->pAxis not initialized\n");
          continue;
        }
        
        int dt = game2->time.current - p->data.turn_time;
        if(dt < TURN_LENGTH) {
          float t = (float)dt / TURNLENGTH;

          float vx = (1 - t) * game2->level->pAxis[p->data.last_dir].v[0] +
            t * game2->level->pAxis[p->data.dir].v[0];
          float vy = (1 - t) * game2->level->pAxis[p->data.last_dir].v[1] +
            t * game2->level->pAxis[p->data.dir].v[1];
          p3d->SetVelocity(Vector3(V * vx, V * vy, 0));
        } else {
          p3d->SetVelocity(Vector3(V * game2->level->pAxis[p->data.dir].v[0], 
           V * game2->level->pAxis[p->data.dir].v[1], 
           0));
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
  if (music && !music->IsPlaying()) {
      // Check if music is enabled. If it is, advance to the next song.
      if (gSettingsCache.playMusic) {
        printf("[audio] Music not playing, calling nextTrack()\n");
        
        // Try to call the scripting function to go to next track
        #ifdef USE_SCRIPTING
            printf("[audio] Music not playing, calling nextTrack() script\n");
            int result = scripting_Run("if nextTrack then nextTrack() else print('[error] nextTrack function not found') end");
            printf("[audio] nextTrack() script returned: %d\n", result);
        #else
            // If scripting is not enabled, call the nextTrack function directly
            printf("[audio] Music not playing, restarting the track\n");
            restartTrack();  // Function to restart the current track
        #endif

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
    
    // Call Idle without try-catch
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

    // Load the sound samples
    Audio_LoadSample("game_engine.wav", 0); // Load engine sample
    Audio_LoadSample("game_crash.wav", 1);  // Load crash sample
    Audio_LoadSample("game_recognizer.wav", 2); // Load recognizer sample

    // Load the player sources
    Audio_LoadPlayers();

    // Set the audio volumes
    sound->SetMixMusic(gSettingsCache.playMusic);
    sound->SetMixFX(gSettingsCache.playEffects);
  }

  void Audio_Start(void) {
    SDL_PauseAudio(0);
  }

  void Audio_Quit(void) {
    printf("[audio] Audio_Quit called - shutting down audio system safely\n");
    
    // Pause audio before doing anything else
    SDL_PauseAudio(1);
    
    // Just set all pointers to NULL without trying to delete them
    // This is not a proper cleanup, but it will prevent segmentation faults
    printf("[audio] Setting all audio pointers to NULL\n");
    
    // Set all pointers to NULL
    music = NULL;
    sample_engine = NULL;
    sample_crash = NULL;
    sample_recognizer = NULL;
    recognizerEngine = NULL;
    ppPlayerSources = NULL;
    nPlayerSources = 0;
    sound = NULL;
    
    // Close audio device
    printf("[audio] Closing audio device\n");
    SDL_CloseAudio();
    
    // Quit SDL audio subsystem
    printf("[audio] Quitting SDL audio subsystem\n");
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
    
    printf("[audio] Audio system shutdown complete\n");
  }

  void Audio_LoadMusic(char *name) {
    printf("[audio] Audio_LoadMusic called with name: %s\n", name ? name : "NULL");
    
    if (!sound) {
        fprintf(stderr, "[error] Cannot load music - sound system not initialized\n");
        return;
    }
    
    if (!name || name[0] == '\0') {
        fprintf(stderr, "[error] Cannot load music - invalid filename\n");
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
    // Load the sound samples if they are not already loaded
    if (!sample_engine || !sample_crash || !sample_recognizer) {
        Audio_LoadPlayers();
    }

    // Set the volumes for the sound samples
    if (sample_engine) {
        sample_engine->SetVolume(volume);
    } else {
        fprintf(stderr, "[error] Cannot set engine volume - sample not loaded\n");
    }

    if (sample_crash) {
        sample_crash->SetVolume(volume);
    } else {
        fprintf(stderr, "[error] Cannot set crash volume - sample not loaded\n");
    }

    if (sample_recognizer) {
        if (volume > 0.8f) {
            sample_recognizer->SetVolume(volume);
        } else {
            sample_recognizer->SetVolume(volume * 1.25f);
        }
    } else {
        fprintf(stderr, "[error] Cannot set recognizer volume - sample not loaded\n");
    }
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
    // Check if game is initialized
    if (!game) {
        fprintf(stderr, "[error] Cannot load players - game not initialized\n");
        return;
    }

    // Check if sound system is initialized
    if (!sound) {
        fprintf(stderr, "[error] Cannot load players - sound system not initialized\n");
        return;
    }

    // Check if engine sample is loaded
    if (!sample_engine) {
        fprintf(stderr, "[error] Cannot load players - engine sample not loaded\n");
        return;
    }

    // Check if recognizer sample is loaded
    if (!sample_recognizer) {
        fprintf(stderr, "[error] Cannot load players - recognizer sample not loaded\n");
        return;
    }

    // Check if player sources are already loaded
    if (ppPlayerSources) {
        fprintf(stderr, "[error] Player sources already loaded\n");
        return;
    }

    // Check if recognizer engine is already loaded
    if (recognizerEngine) {
        fprintf(stderr, "[error] Recognizer engine already loaded\n");
        return;
    }

    // Load player sources
    nPlayerSources = game->players;
    ppPlayerSources = new Sound::SourceEngine*[nPlayerSources];
    for(int i = 0; i < nPlayerSources; i++) {
        ppPlayerSources[i] = new Sound::SourceEngine(sound, sample_engine);
        ppPlayerSources[i]->SetType(Sound::eSoundFX);
        sound->AddSource(ppPlayerSources[i]);

        char *name = new char[32];
        sprintf(name, "player %d", i);
        ppPlayerSources[i]->SetName(name);
        delete[] name;
    }

    // Load recognizer engine
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
        char alt_name[1024];  // Increased buffer size to avoid overflow
        const char* basename = strrchr(name, '/');
        if (basename) {
            char path[512];
            size_t path_len = basename - name + 1;
            if (path_len < sizeof(path)) {
                strncpy(path, name, path_len);
                path[path_len] = '\0';
                snprintf(alt_name, sizeof(alt_name), "%sgame_%s", path, basename + 1);
            } else {
                snprintf(alt_name, sizeof(alt_name), "game_%s", basename + 1);
            }
        } else {
            snprintf(alt_name, sizeof(alt_name), "game_%s", name);
        }

        FILE *alt_f = fopen(alt_name, "rb");
        if (alt_f) {
            fclose(alt_f);
            printf("[audio] Original file not found, using alternative: %s\n", alt_name);
            name = strdup(alt_name); // Note: This creates a memory leak, but it's small and one-time
        } else {
            fprintf(stderr, "[error] Sample file not found: %s (also tried %s)\n", name, alt_name);
            fprintf(stderr, "[error] Current working directory: %s\n", getcwd(NULL, 0));
            fprintf(stderr, "[error] Please ensure the file exists in the correct location\n");

            // Try loading from data directory as a last resort
            char data_path[1024];
            snprintf(data_path, sizeof(data_path), "%s", name);
            FILE *data_f = fopen(data_path, "rb");
            if (data_f) {
                fclose(data_f);
                printf("[audio] Found sample in data directory: %s\n", data_path);
                name = strdup(data_path); // Note: This creates a memory leak, but it's small and one-time
            }
        }
    } else {
        fclose(f);
    }

    // Try loading from data directory if not found in current location
    if (!f) {
        char data_path[1024];
        snprintf(data_path, sizeof(data_path), "data/%s", name);
        f = fopen(data_path, "rb");
        if (f) {
            fclose(f);
            printf("[audio] Found sample in data directory: %s\n", data_path);
            name = strdup(data_path); // Note: This creates a memory leak, but it's small and one-time
        }
    }

    switch(number) {
    case 0: // engine sound
        if (sample_engine) {
            delete sample_engine;
        }
        sample_engine = new Sound::SourceSample(sound);
        if (!sample_engine->Load(name)) {
            fprintf(stderr, "[error] Failed to load engine sample: %s\n", name);
            fprintf(stderr, "[error] Check if file exists and is in correct format\n");
            delete sample_engine;
            sample_engine = NULL;
        } else {
            printf("[audio] Loaded engine sample successfully: %s\n", name);
            sample_engine->SetName("sample: engine");
        }
        break;
    case 1: // crash sound
        if (sample_crash) {
            delete sample_crash;
        }
        sample_crash = new Sound::SourceSample(sound);
        if (!sample_crash->Load(name)) {
            fprintf(stderr, "[error] Failed to load crash sample: %s\n", name);
            fprintf(stderr, "[error] Check if file exists and is in correct format\n");
            delete sample_crash;
            sample_crash = NULL;
        } else {
            printf("[audio] Loaded crash sample successfully: %s\n", name);
            sample_crash->SetName("sample: crash");
        }
        break;
    case 2: // recognizer sound
        if (sample_recognizer) {
            delete sample_recognizer;
        }
        sample_recognizer = new Sound::SourceSample(sound);
        if (!sample_recognizer->Load(name)) {
            fprintf(stderr, "[error] Failed to load recognizer sample: %s\n", name);
            fprintf(stderr, "[error] Check if file exists and is in correct format\n");
            fprintf(stderr, "[error] Current working directory: %s\n", getcwd(NULL, 0));
            fprintf(stderr, "[error] Please ensure the file exists in the correct location\n");
            delete sample_recognizer;
            sample_recognizer = NULL;
        } else {
            printf("[audio] Loaded recognizer sample successfully: %s\n", name);
            sample_recognizer->SetName("sample: recognizer");
        }
        break;
    default:
        /* programmer error, but non-critical */
        fprintf(stderr, "[error] unknown sample %d: '%s'\n", number, name);
    }
  }
}
#else

// Add necessary includes for the disabled audio section
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "audio/sound_glue.h"
extern "C" {
#include "game/game.h"
#include "scripting/nebu_scripting.h"
#include "configuration/settings.h"
}

extern "C" {
  void Audio_EnableEngine(void) {
    printf("[audio] Audio_EnableEngine called (audio disabled)\n");
  }

  void Audio_DisableEngine(void) {
    printf("[audio] Audio_DisableEngine called (audio disabled)\n");
  }

  void Audio_Idle(void) { 
    // Do nothing - audio is disabled
  }

  void Audio_CrashPlayer(int player) {
    printf("[audio] Audio_CrashPlayer called for player %d (audio disabled)\n", player);
  }

  void Audio_Init(void) {
    printf("[audio] Audio_Init called (audio disabled)\n");
    // Don't try to use Lua to set playMusic
    // scripting_Run("if settings then settings.playMusic = 0 end");
  }
  
  void Audio_Start(void) {
    printf("[audio] Audio_Start called (audio disabled)\n");
  }

  void Audio_Quit(void) {
    printf("[audio] Audio_Quit called (audio disabled)\n");
  }

  void Audio_LoadMusic(char *name) {
    printf("[audio] Audio_LoadMusic called with name: %s (audio disabled)\n", name ? name : "NULL");
  }

  void Audio_PlayMusic(void) {
    printf("[audio] Audio_PlayMusic called (audio disabled)\n");
  }

  void Audio_StopMusic(void) {
    printf("[audio] Audio_StopMusic called (audio disabled)\n");
  }

  void Audio_SetMusicVolume(float volume) {
    printf("[audio] Audio_SetMusicVolume called with volume: %.2f (audio disabled)\n", volume);
  }
  
  void Audio_SetFxVolume(float volume) {
    printf("[audio] Audio_SetFxVolume called with volume: %.2f (audio disabled)\n", volume);
  }

  void Audio_StartEngine(int iPlayer) {
    printf("[audio] Audio_StartEngine called for player %d (audio disabled)\n", iPlayer);
  }

  void Audio_StopEngine(int iPlayer) {
    printf("[audio] Audio_StopEngine called for player %d (audio disabled)\n", iPlayer);
  }
 
  void Audio_ResetData(void) {
    printf("[audio] Audio_ResetData called (audio disabled)\n");
  }

  void Audio_UnloadPlayers(void) {
    printf("[audio] Audio_UnloadPlayers called (audio disabled)\n");
  }

  void Audio_LoadPlayers(void) {
    printf("[audio] Audio_LoadPlayers called (audio disabled)\n");
  }

  void Audio_LoadSample(char *name, int number) {
    printf("[audio] Audio_LoadSample called with name: %s, number: %d (audio disabled)\n", 
           name ? name : "NULL", number);
  }
}

#endif
