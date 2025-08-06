#ifndef ANDROID_AUDIO_H
#define ANDROID_AUDIO_H

#ifdef __cplusplus
extern "C" {
#endif

// Audio engine functions
int android_audio_init(void);
void android_audio_cleanup(void);

// Volume control functions
void android_audio_set_master_volume(float volume);
void android_audio_set_sfx_volume(float volume);
void android_audio_set_music_volume(float volume);
float android_audio_get_master_volume(void);
float android_audio_get_sfx_volume(void);
float android_audio_get_music_volume(void);

// Audio enable/disable
void android_audio_enable(int enabled);
int android_audio_is_enabled(void);

// Sound effect functions
void android_audio_play_sound_effect(int sound_id);
void android_audio_play_engine_sound(void);
void android_audio_play_crash_sound(void);
void android_audio_play_turn_sound(void);
void android_audio_play_boost_sound(void);

// Music functions
void android_audio_play_music(const char* music_file);
void android_audio_stop_music(void);
void android_audio_pause_music(void);
void android_audio_resume_music(void);

// Integration functions
void android_audio_update_gltron_audio(void);

// Settings persistence functions
void android_audio_load_settings(void);
void android_audio_save_settings(void);

// Game audio settings interface
void gltron_audio_settings_set_master_volume(float volume);
void gltron_audio_settings_set_sfx_volume(float volume);
void gltron_audio_settings_set_music_volume(float volume);
void gltron_audio_settings_toggle_audio(void);
void gltron_audio_settings_set_3d_audio(int enabled);
void gltron_audio_settings_set_quality(int quality);

// Getter functions for settings
float gltron_audio_settings_get_master_volume(void);
float gltron_audio_settings_get_sfx_volume(void);
float gltron_audio_settings_get_music_volume(void);
int gltron_audio_settings_is_audio_enabled(void);
int gltron_audio_settings_is_3d_audio_enabled(void);
int gltron_audio_settings_get_quality(void);

// Menu interface functions
int gltron_audio_settings_get_slider_count(void);
int gltron_audio_settings_get_option_count(void);
const char* gltron_audio_settings_get_slider_name(int index);
const char* gltron_audio_settings_get_option_name(int index);
void gltron_audio_settings_adjust_slider(int index, float delta);
void gltron_audio_settings_cycle_option(int index);
float gltron_audio_settings_get_slider_value(int index);
const char* gltron_audio_settings_get_option_value(int index);

// Sound effect IDs
#define SOUND_ENGINE 1
#define SOUND_CRASH 2
#define SOUND_TURN 3
#define SOUND_BOOST 4
#define SOUND_MENU_SELECT 5
#define SOUND_MENU_MOVE 6

#ifdef __cplusplus
}
#endif

#endif /* ANDROID_AUDIO_H */

