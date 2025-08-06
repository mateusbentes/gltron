#include "config.h"

#ifdef ANDROID
#include "android_config.h"
#else
#include "platform_config.h"
#endif
#include <GLES2/gl2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Resolution settings structure
typedef struct {
    int width;
    int height;
    const char* name;
    float scale_factor; // For rendering scale
} ResolutionSetting;

// Graphics quality settings
typedef struct {
    const char* name;
    int texture_quality; // 0=low, 1=medium, 2=high
    int shadow_quality;
    int particle_quality;
    int anti_aliasing;
    float render_scale; // Rendering resolution scale
} GraphicsQuality;

// Available resolution settings
static ResolutionSetting g_resolutions[] = {
    {480, 320, "480x320 (Low)", 0.5f},
    {640, 480, "640x480 (Medium)", 0.67f},
    {800, 600, "800x600 (High)", 0.83f},
    {1024, 768, "1024x768 (Very High)", 1.0f},
    {1280, 720, "1280x720 (HD)", 1.0f},
    {1920, 1080, "1920x1080 (Full HD)", 1.0f},
    {0, 0, "Native (Auto)", 1.0f} // Native resolution
};

// Graphics quality presets
static GraphicsQuality g_quality_presets[] = {
    {"Low", 0, 0, 0, 0, 0.5f},
    {"Medium", 1, 1, 1, 0, 0.75f},
    {"High", 2, 2, 2, 1, 1.0f},
    {"Ultra", 2, 2, 2, 2, 1.0f}
};

// Current settings
typedef struct {
    int current_resolution_index;
    int current_quality_index;
    int native_width;
    int native_height;
    int render_width;
    int render_height;
    int viewport_width;
    int viewport_height;
    float aspect_ratio;
    int fullscreen;
    int vsync_enabled;
} DisplaySettings;

static DisplaySettings g_display_settings = {
    .current_resolution_index = 6, // Native by default
    .current_quality_index = 1, // Medium quality by default
    .native_width = 1920,
    .native_height = 1080,
    .render_width = 1920,
    .render_height = 1080,
    .viewport_width = 1920,
    .viewport_height = 1080,
    .aspect_ratio = 16.0f/9.0f,
    .fullscreen = 1,
    .vsync_enabled = 1
};

// Settings file path
static const char* DISPLAY_SETTINGS_FILE = "/data/data/com.gltron.android/files/display_settings.cfg";

// Initialize resolution manager
void android_resolution_init(int native_width, int native_height) {
    g_display_settings.native_width = native_width;
    g_display_settings.native_height = native_height;
    g_display_settings.aspect_ratio = (float)native_width / (float)native_height;
    
    // Set native resolution in the resolutions array
    g_resolutions[6].width = native_width;
    g_resolutions[6].height = native_height;
    
    LOGI("Resolution manager initialized: %dx%d (aspect: %.2f)", 
         native_width, native_height, g_display_settings.aspect_ratio);
    
    // Load saved settings
    android_resolution_load_settings();
    
    // Apply current settings
    android_resolution_apply_current_settings();
}

// Load display settings from file
void android_resolution_load_settings() {
    FILE* file = fopen(DISPLAY_SETTINGS_FILE, "rb");
    if (file) {
        size_t read = fread(&g_display_settings, sizeof(DisplaySettings), 1, file);
        fclose(file);
        
        if (read == 1) {
            LOGI("Display settings loaded successfully");
        } else {
            LOGE("Failed to read display settings file");
        }
    } else {
        LOGI("Display settings file not found, using defaults");
        android_resolution_save_settings(); // Create default settings file
    }
}

// Save display settings to file
void android_resolution_save_settings() {
    FILE* file = fopen(DISPLAY_SETTINGS_FILE, "wb");
    if (file) {
        size_t written = fwrite(&g_display_settings, sizeof(DisplaySettings), 1, file);
        fclose(file);
        
        if (written == 1) {
            LOGI("Display settings saved successfully");
        } else {
            LOGE("Failed to write display settings file");
        }
    } else {
        LOGE("Failed to open display settings file for writing");
    }
}

// Apply current resolution and quality settings
void android_resolution_apply_current_settings() {
    ResolutionSetting* res = &g_resolutions[g_display_settings.current_resolution_index];
    GraphicsQuality* quality = &g_quality_presets[g_display_settings.current_quality_index];
    
    // Calculate render resolution
    if (res->width == 0 && res->height == 0) {
        // Native resolution
        g_display_settings.render_width = g_display_settings.native_width;
        g_display_settings.render_height = g_display_settings.native_height;
    } else {
        // Fixed resolution
        g_display_settings.render_width = res->width;
        g_display_settings.render_height = res->height;
    }
    
    // Apply quality scaling
    g_display_settings.render_width = (int)(g_display_settings.render_width * quality->render_scale);
    g_display_settings.render_height = (int)(g_display_settings.render_height * quality->render_scale);
    
    // Set viewport (always native for Android)
    g_display_settings.viewport_width = g_display_settings.native_width;
    g_display_settings.viewport_height = g_display_settings.native_height;
    
    // Configure OpenGL viewport
    glViewport(0, 0, g_display_settings.viewport_width, g_display_settings.viewport_height);
    
    LOGI("Applied resolution: %dx%d (render: %dx%d, viewport: %dx%d)", 
         res->width, res->height,
         g_display_settings.render_width, g_display_settings.render_height,
         g_display_settings.viewport_width, g_display_settings.viewport_height);
}

// Resolution management functions
int android_resolution_get_count() {
    return sizeof(g_resolutions) / sizeof(ResolutionSetting);
}

const char* android_resolution_get_name(int index) {
    if (index >= 0 && index < android_resolution_get_count()) {
        return g_resolutions[index].name;
    }
    return NULL;
}

void android_resolution_set_resolution(int index) {
    if (index >= 0 && index < android_resolution_get_count()) {
        g_display_settings.current_resolution_index = index;
        android_resolution_apply_current_settings();
        LOGI("Resolution changed to: %s", g_resolutions[index].name);
    }
}

int android_resolution_get_current_index() {
    return g_display_settings.current_resolution_index;
}

// Quality management functions
int android_resolution_get_quality_count() {
    return sizeof(g_quality_presets) / sizeof(GraphicsQuality);
}

const char* android_resolution_get_quality_name(int index) {
    if (index >= 0 && index < android_resolution_get_quality_count()) {
        return g_quality_presets[index].name;
    }
    return NULL;
}

void android_resolution_set_quality(int index) {
    if (index >= 0 && index < android_resolution_get_quality_count()) {
        g_display_settings.current_quality_index = index;
        android_resolution_apply_current_settings();
        LOGI("Graphics quality changed to: %s", g_quality_presets[index].name);
    }
}

int android_resolution_get_current_quality_index() {
    return g_display_settings.current_quality_index;
}

// Get current resolution info
void android_resolution_get_current_resolution(int* width, int* height) {
    if (width) *width = g_display_settings.render_width;
    if (height) *height = g_display_settings.render_height;
}

void android_resolution_get_viewport_size(int* width, int* height) {
    if (width) *width = g_display_settings.viewport_width;
    if (height) *height = g_display_settings.viewport_height;
}

float android_resolution_get_aspect_ratio() {
    return g_display_settings.aspect_ratio;
}

// Advanced settings
void android_resolution_set_vsync(int enabled) {
    g_display_settings.vsync_enabled = enabled;
    LOGI("VSync %s", enabled ? "enabled" : "disabled");
    // Note: VSync control on Android is typically handled by the system
}

int android_resolution_get_vsync() {
    return g_display_settings.vsync_enabled;
}

void android_resolution_set_fullscreen(int enabled) {
    g_display_settings.fullscreen = enabled;
    LOGI("Fullscreen %s", enabled ? "enabled" : "disabled");
    // Note: Android apps are typically always fullscreen
}

int android_resolution_get_fullscreen() {
    return g_display_settings.fullscreen;
}

// Performance monitoring
typedef struct {
    float fps;
    float frame_time;
    int dropped_frames;
    float gpu_usage;
} PerformanceStats;

static PerformanceStats g_perf_stats = {0};

void android_resolution_update_performance_stats(float fps, float frame_time) {
    g_perf_stats.fps = fps;
    g_perf_stats.frame_time = frame_time;
    
    // Auto-adjust quality based on performance
    if (fps < 30.0f && g_display_settings.current_quality_index > 0) {
        // Performance is poor, suggest lower quality
        LOGI("Low FPS detected (%.1f), consider lowering graphics quality", fps);
    } else if (fps > 55.0f && g_display_settings.current_quality_index < android_resolution_get_quality_count() - 1) {
        // Performance is good, could handle higher quality
        LOGI("High FPS detected (%.1f), could increase graphics quality", fps);
    }
}

void android_resolution_get_performance_stats(float* fps, float* frame_time) {
    if (fps) *fps = g_perf_stats.fps;
    if (frame_time) *frame_time = g_perf_stats.frame_time;
}

// Auto-resolution detection based on device capabilities
void android_resolution_auto_detect_optimal() {
    // This would analyze device capabilities and set optimal settings
    // For now, use a simple heuristic based on native resolution
    
    int total_pixels = g_display_settings.native_width * g_display_settings.native_height;
    
    if (total_pixels >= 1920 * 1080) {
        // High-end device
        android_resolution_set_quality(2); // High quality
        android_resolution_set_resolution(6); // Native resolution
    } else if (total_pixels >= 1280 * 720) {
        // Mid-range device
        android_resolution_set_quality(1); // Medium quality
        android_resolution_set_resolution(4); // 1280x720
    } else {
        // Low-end device
        android_resolution_set_quality(0); // Low quality
        android_resolution_set_resolution(1); // 640x480
    }
    
    LOGI("Auto-detected optimal settings for %dx%d device", 
         g_display_settings.native_width, g_display_settings.native_height);
}

// Integration with GLTron graphics system
void android_resolution_update_gltron_graphics() {
    // This function would integrate with the original GLTron graphics system
    // by providing the Android-specific resolution and quality settings
    
    ResolutionSetting* res = &g_resolutions[g_display_settings.current_resolution_index];
    GraphicsQuality* quality = &g_quality_presets[g_display_settings.current_quality_index];
    
    // Example integration (would need to match GLTron's graphics interface):
    /*
    gltron_graphics_set_resolution(g_display_settings.render_width, g_display_settings.render_height);
    gltron_graphics_set_texture_quality(quality->texture_quality);
    gltron_graphics_set_shadow_quality(quality->shadow_quality);
    gltron_graphics_set_particle_quality(quality->particle_quality);
    gltron_graphics_set_anti_aliasing(quality->anti_aliasing);
    */
    
    LOGI("Updated GLTron graphics settings: %s quality, %s resolution", 
         quality->name, res->name);
}

