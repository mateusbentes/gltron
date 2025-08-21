#ifndef ANDROID
#include "sound_backend.h"
#include "sound.h"

// Desktop: delegate to existing MikMod-based functions
int sb_init(void){ return 0; }
void sb_shutdown(void){}
int sb_load_music(const char* path){ (void)path; return 0; }
void sb_play_music(void){ playSound(); }
void sb_stop_music(void){ stopSound(); }
void sb_set_enabled(int sound_on, int music_on){
  game->settings->playSound = sound_on;
  game->settings->playMusic = music_on;
}
int sb_load_sfx(int id, const char* path){ (void)id; (void)path; return 0; }
void sb_play_sfx(int id){ (void)id; /* use playSampleEffect in existing code paths */ }
void sb_update(void){ soundIdle(); }
#endif
