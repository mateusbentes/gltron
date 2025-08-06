#ifndef ANDROID_RESOLUTION_H
#define ANDROID_RESOLUTION_H

#ifdef __cplusplus
extern "C" {
#endif

// Initialization and cleanup
void android_resolution_init(int native_width, int native_height);

// Settings persistence
void android_resolution_load_settings(void);
void android_resolution_save_settings(void);
void android_resolution_apply_current_settings(void);

// Resolution management
int android_resolution_get_count(void);
const char* android_resolution_get_name(int index);
void android_resolution_set_resolution(int index);
int android_resolution_get_current_index(void);

// Quality management
int android_resolution_get_quality_count(void);
const char* android_resolution_get_quality_name(int index);
void android_resolution_set_quality(int index);
int android_resolution_get_current_quality_index(void);

// Current resolution info
void android_resolution_get_current_resolution(int* width, int* height);
void android_resolution_get_viewport_size(int* width, int* height);
float android_resolution_get_aspect_ratio(void);

// Advanced settings
void android_resolution_set_vsync(int enabled);
int android_resolution_get_vsync(void);
void android_resolution_set_fullscreen(int enabled);
int android_resolution_get_fullscreen(void);

// Performance monitoring
void android_resolution_update_performance_stats(float fps, float frame_time);
void android_resolution_get_performance_stats(float* fps, float* frame_time);

// Auto-detection
void android_resolution_auto_detect_optimal(void);

// Integration
void android_resolution_update_gltron_graphics(void);

#ifdef __cplusplus
}
#endif

#endif /* ANDROID_RESOLUTION_H */

