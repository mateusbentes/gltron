#pragma once
#include "gltron.h"

#ifdef __cplusplus
extern "C" {
#endif

int sb_init(void);
void sb_shutdown(void);
int sb_load_music(const char* path);
void sb_play_music(void);
void sb_stop_music(void);
void sb_set_enabled(int sound_on, int music_on);
int sb_load_sfx(int id, const char* path);
void sb_play_sfx(int id);
void sb_update(void);

#ifdef __cplusplus
}
#endif
