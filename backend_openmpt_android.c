#ifdef ANDROID
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "sound_backend.h"
#include "gltron.h" // for getFullPath declaration

#include <libopenmpt/libopenmpt.h>

#define SAMPLE_RATE 44100
#define BUFFER_SAMPLES 1024
#define SFX_MAX 8

typedef struct { short* data; int frames; int channels; int playing; int pos; } Sfx;

static SLObjectItf engineObject = NULL;
static SLEngineItf engineEngine = NULL;
static SLObjectItf outputMixObject = NULL;
static SLObjectItf playerObject = NULL;
static SLPlayItf playerPlay = NULL;
static SLAndroidSimpleBufferQueueItf playerQueue = NULL;

static short bufferA[BUFFER_SAMPLES * 2];
static short bufferB[BUFFER_SAMPLES * 2];
static int curbuf = 0;

static int enable_sound = 1;
static int enable_music = 1;

static openmpt_module* mod = NULL;
static Sfx sfx[SFX_MAX];

static void mix(short* out, int frames) {
  memset(out, 0, sizeof(short) * frames * 2);
  // music
  if (enable_music && mod) {
    int count = openmpt_module_read_interleaved_stereo(mod, SAMPLE_RATE, frames, out);
    (void)count;
  }
  // sfx
  if (enable_sound) {
    for (int i = 0; i < SFX_MAX; ++i) {
      if (!sfx[i].playing || !sfx[i].data) continue;
      for (int f = 0; f < frames; ++f) {
        if (sfx[i].pos >= sfx[i].frames) { sfx[i].playing = 0; break; }
        int idx = sfx[i].pos * sfx[i].channels;
        short l = sfx[i].data[idx];
        short r = (sfx[i].channels > 1) ? sfx[i].data[idx+1] : l;
        int ol = out[2*f] + l;
        int or = out[2*f+1] + r;
        if (ol > 32767) ol = 32767; if (ol < -32768) ol = -32768;
        if (or > 32767) or = 32767; if (or < -32768) or = -32768;
        out[2*f] = (short)ol; out[2*f+1] = (short)or;
        sfx[i].pos++;
      }
    }
  }
}

static void buffer_callback(SLAndroidSimpleBufferQueueItf bq, void *context) {
  (void)bq; (void)context;
  short* buf = (curbuf == 0) ? bufferA : bufferB;
  mix(buf, BUFFER_SAMPLES);
  (*playerQueue)->Enqueue(playerQueue, buf, sizeof(short) * BUFFER_SAMPLES * 2);
  curbuf ^= 1;
}

static int create_engine() {
  SLresult result;
  result = slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);
  if (result != SL_RESULT_SUCCESS) return 0;
  result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
  if (result != SL_RESULT_SUCCESS) return 0;
  result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);
  if (result != SL_RESULT_SUCCESS) return 0;
  result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 0, NULL, NULL);
  if (result != SL_RESULT_SUCCESS) return 0;
  result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
  if (result != SL_RESULT_SUCCESS) return 0;
  return 1;
}

static int create_player() {
  SLresult result;
  SLDataLocator_AndroidSimpleBufferQueue loc_bufq = { SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2 };
  SLDataFormat_PCM format_pcm = {
    SL_DATAFORMAT_PCM, 2, SL_SAMPLINGRATE_44_1,
    SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
    SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT, SL_BYTEORDER_LITTLEENDIAN };
  SLDataSource audioSrc = { &loc_bufq, &format_pcm };
  SLDataLocator_OutputMix loc_outmix = { SL_DATALOCATOR_OUTPUTMIX, outputMixObject };
  SLDataSink audioSnk = { &loc_outmix, NULL };

  const SLInterfaceID ids[1] = { SL_IID_ANDROIDSIMPLEBUFFERQUEUE };
  const SLboolean req[1] = { SL_BOOLEAN_TRUE };
  result = (*engineEngine)->CreateAudioPlayer(engineEngine, &playerObject, &audioSrc, &audioSnk, 1, ids, req);
  if (result != SL_RESULT_SUCCESS) return 0;
  result = (*playerObject)->Realize(playerObject, SL_BOOLEAN_FALSE);
  if (result != SL_RESULT_SUCCESS) return 0;
  result = (*playerObject)->GetInterface(playerObject, SL_IID_PLAY, &playerPlay);
  if (result != SL_RESULT_SUCCESS) return 0;
  result = (*playerObject)->GetInterface(playerObject, SL_IID_ANDROIDSIMPLEBUFFERQUEUE, &playerQueue);
  if (result != SL_RESULT_SUCCESS) return 0;
  (*playerQueue)->RegisterCallback(playerQueue, buffer_callback, NULL);
  (*playerPlay)->SetPlayState(playerPlay, SL_PLAYSTATE_PLAYING);
  // Prime both buffers
  buffer_callback(playerQueue, NULL);
  buffer_callback(playerQueue, NULL);
  return 1;
}

static void destroy_audio() {
  if (playerObject) { (*playerObject)->Destroy(playerObject); playerObject = NULL; playerQueue = NULL; playerPlay = NULL; }
  if (outputMixObject) { (*outputMixObject)->Destroy(outputMixObject); outputMixObject = NULL; }
  if (engineObject) { (*engineObject)->Destroy(engineObject); engineObject = NULL; engineEngine = NULL; }
}

// Minimal WAV loader for PCM16, 44.1kHz mono/stereo
static int load_wav(const char* path, short** out_data, int* out_frames, int* out_channels) {
  FILE* f = fopen(path, "rb");
  if (!f) return 0;
  char riff[4]; fread(riff,1,4,f); if (memcmp(riff,"RIFF",4)!=0){ fclose(f); return 0; }
  fseek(f, 22, SEEK_SET); // channels
  unsigned short channels; fread(&channels,2,1,f);
  unsigned int sampleRate; fread(&sampleRate,4,1,f);
  fseek(f, 34, SEEK_SET); unsigned short bits; fread(&bits,2,1,f);
  if (bits != 16){ fclose(f); return 0; }
  // find data chunk
  fseek(f, 12, SEEK_SET);
  char id[4]; unsigned int sz;
  while (fread(id,1,4,f)==4){ fread(&sz,4,1,f); if (memcmp(id,"data",4)==0) break; fseek(f, sz, SEEK_CUR); }
  if (memcmp(id,"data",4)!=0){ fclose(f); return 0; }
  int frames = sz / (2 * channels);
  short* data = (short*)malloc(sz);
  fread(data,1,sz,f);
  fclose(f);
  *out_data = data; *out_frames = frames; *out_channels = channels;
  if (sampleRate != SAMPLE_RATE) {
    // naive: ignore resampling; will play at wrong pitch; TODO: resample if needed
    fprintf(stderr, "WAV %s sampleRate %u differs from %d; playback pitch may be off\n", path, sampleRate, SAMPLE_RATE);
  }
  return 1;
}

// Backend interface
int sb_init(void) {
  memset(sfx, 0, sizeof(sfx));
  if (!create_engine()) return 0;
  if (!create_player()) return 0;
  return 1;
}

void sb_shutdown(void) {
  if (mod) { openmpt_module_destroy(mod); mod = NULL; }
  for (int i=0;i<SFX_MAX;++i){ if (sfx[i].data) { free(sfx[i].data); sfx[i].data=NULL; } }
  destroy_audio();
}

int sb_load_music(const char* path) {
  if (mod) { openmpt_module_destroy(mod); mod = NULL; }
  char* full = getFullPath((char*)path);
  if (!full) return 0;
  FILE* f = fopen(full, "rb");
  if (!f) { free(full); return 0; }
  fseek(f, 0, SEEK_END); long sz = ftell(f); fseek(f, 0, SEEK_SET);
  void* buf = malloc(sz);
  fread(buf,1,sz,f);
  fclose(f);
  mod = openmpt_module_create_from_memory2(buf, sz, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
  free(buf);
  free(full);
  return mod != NULL;
}

void sb_play_music(void) {
  // nothing explicit; buffer callback will pull from mod if present
}

void sb_stop_music(void) {
  if (mod) { openmpt_module_destroy(mod); mod = NULL; }
}

void sb_set_enabled(int sound_on, int music_on) {
  enable_sound = sound_on;
  enable_music = music_on;
}

int sb_load_sfx(int id, const char* path) {
  if (id < 0 || id >= SFX_MAX) return 0;
  char* full = getFullPath((char*)path);
  if (!full) return 0;
  short* data=NULL; int frames=0; int ch=0;
  int ok = load_wav(full, &data, &frames, &ch);
  if (!ok) return 0;
  free(full);
  if (sfx[id].data) free(sfx[id].data);
  sfx[id].data = data; sfx[id].frames = frames; sfx[id].channels = ch; sfx[id].playing = 0; sfx[id].pos = 0;
  return 1;
}

void sb_play_sfx(int id) {
  if (id < 0 || id >= SFX_MAX) return;
  if (!sfx[id].data) return;
  sfx[id].pos = 0; sfx[id].playing = 1;
}

void sb_update(void) {
  // nothing required; audio is pulled in callback
}

#endif // ANDROID
