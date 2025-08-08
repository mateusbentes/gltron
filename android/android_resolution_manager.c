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
#include <android/log.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>

#define LOG_TAG "GLTron_Resolution"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

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
    int initialized; // FIXED: Add initialization flag
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
    .vsync_enabled = 1,
    .initialized = 0
};

// FIXED: Use internal storage path instead of hardcoded system path
static char g_settings_file_path[256] = {0};

// FIXED: Add validation functions
static int validate_resolution_index(int index) {
    int count = sizeof(g_resolutions) / sizeof(ResolutionSetting);
    return (index >= 0 && index < count);
}

static int validate_quality_index(int index) {
    int count = sizeof(g_quality_presets) / sizeof(GraphicsQuality);
    return (index >= 0 && index < count);
}

static int validate_dimensions(int width, int height) {
    return (width > 0 && height > 0 && width <= 4096 && height <= 4096);
}

// FIXED: Safe file path construction
static int setup_settings_file_path(const char* internal_data_path) {
    if (!internal_data_path) {
        LOGE("Internal data path is NULL");
        return 0;
    }
    
    int ret = snprintf(g_settings_file_path, sizeof(g_settings_file_path), 
                      "%s/display_settings.cfg", internal_data_path);
    
    if (ret >= sizeof(g_settings_file_path) || ret < 0) {
        LOGE("Settings file path too long or invalid");
        return 0;
    }
    
    LOGI("Settings file path: %s", g_settings_file_path);
    return 1;
}

// FIXED: Check file permissions and create directory if needed
static int ensure_settings_directory() {
    if (strlen(g_settings_file_path) == 0) {
        LOGE("Settings file path not set");
        return 0;
    }
    
    // Extract directory path
    char dir_path[256];
    strncpy(dir_path, g_settings_file_path, sizeof(dir_path) - 1);
    dir_path[sizeof(dir_path) - 1] = '\0';
    
    char* last_slash = strrchr(dir_path, '/');
    if (last_slash) {
        *last_slash = '\0';
        
        // Check if directory exists
        struct stat st;
        if (stat(dir_path, &st) != 0) {
            // Directory doesn't exist, try to create it
            if (mkdir(dir_path, 0755) != 0) {
                LOGE("Failed to create settings directory: %s (errno: %d)", dir_path, errno);
                return 0;
            }
            LOGI("Created settings directory: %s", dir_path);
        }
    }
    
    return 1;
}

// Initialize resolution manager
void android_resolution_init(int native_width, int native_height) {
    // FIXED: Validate input dimensions
    if (!validate_dimensions(native_width, native_height)) {
        LOGE("Invalid native dimensions: %dx%d", native_width, native_height);
        // Use safe defaults
        native_width = 1920;
        native_height = 1080;
    }
    
    g_display_settings.native_width = native_width;
    g_display_settings.native_height = native_height;
    
    // FIXED: Safe aspect ratio calculation
    if (native_height > 0) {
        g_display_settings.aspect_ratio = (float)native_width / (float)native_height;
    } else {
        LOGE("Invalid native height: %d, using default aspect ratio", native_height);
        g_display_settings.aspect_ratio = 16.0f / 9.0f;
    }
    
    // Set native resolution in the resolutions array
    g_resolutions[6].width = native_width;
    g_resolutions[6].height = native_height;
    
    // FIXED: Setup settings file path (would need to be passed from Java/JNI)
    // For now, use a default internal path
    if (!setup_settings_file_path("/data/data/com.gltron.android/files")) {
        LOGE("Failed to setup settings file path");
    }
    
    g_display_settings.initialized = 1;
    
    LOGI("Resolution manager initialized: %dx%d (aspect: %.2f)", 
         native_width, native_height, g_display_settings.aspect_ratio);
    
    // Load saved settings
    android_resolution_load_settings();
    
    // Apply current settings
    android_resolution_apply_current_settings();
}

// Load display settings from file
void android_resolution_load_settings() {
    if (!g_display_settings.initialized) {
        LOGE("Resolution manager not initialized");
        return;
    }
    
    if (strlen(g_settings_file_path) == 0) {
        LOGE("Settings file path not set");
        return;
    }
    
    FILE* file = fopen(g_settings_file_path, "rb");
    if (file) {
        DisplaySettings loaded_settings;
        size_t read = fread(&loaded_settings, sizeof(DisplaySettings), 1, file);
        fclose(file);
        
        if (read == 1) {
            // FIXED: Validate loaded settings before using
            if (validate_resolution_index(loaded_settings.current_resolution_index) &&
                validate_quality_index(loaded_settings.current_quality_index) &&
                validate_dimensions(loaded_settings.native_width, loaded_settings.native_height)) {
                
                // Copy validated settings
                g_display_settings.current_resolution_index = loaded_settings.current_resolution_index;
                g_display_settings.current_quality_index = loaded_settings.current_quality_index;
                g_display_settings.vsync_enabled = loaded_settings.vsync_enabled;
                g_display_settings.fullscreen = loaded_settings.fullscreen;
                
                LOGI("Display settings loaded and validated successfully");
            } else {
                LOGE("Loaded settings failed validation, using defaults");
            }
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
    if (!g_display_settings.initialized) {
        LOGE("Resolution manager not initialized");
        return;
    }
    
    if (strlen(g_settings_file_path) == 0) {
        LOGE("Settings file path not set");
        return;
    }
    
    if (!ensure_settings_directory()) {
        LOGE("Failed to ensure settings directory exists");
        return;
    }
    
    FILE* file = fopen(g_settings_file_path, "wb");
    if (file) {
        size_t written = fwrite(&g_display_settings, sizeof(DisplaySettings), 1, file);
        fclose(file);
        
        if (written == 1) {
            LOGI("Display settings saved successfully");
        } else {
            LOGE("Failed to write display settings file");
        }
    } else {
        LOGE("Failed to open display settings file for writing: %s (errno: %d)", 
             g_settings_file_path, errno);
    }
}

// Apply current resolution and quality settings
void android_resolution_apply_current_settings() {
    if (!g_display_settings.initialized) {
        LOGE("Resolution manager not initialized");
        return;
    }
    
    // FIXED: Validate indices before using
    if (!validate_resolution_index(g_display_settings.current_resolution_index)) {
        LOGE("Invalid resolution index: %d, using default", g_display_settings.current_resolution_index);
        g_display_settings.current_resolution_index = 6; // Native
    }
    
    if (!validate_quality_index(g_display_settings.current_quality_index)) {
        LOGE("Invalid quality index: %d, using default", g_display_settings.current_quality_index);
        g_display_settings.current_quality_index = 1; // Medium
    }
    
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
    
    // FIXED: Validate quality render scale
    float render_scale = quality->render_scale;
    if (render_scale <= 0.0f || render_scale > 2.0f) {
        LOGE("Invalid render scale: %f, using 1.0", render_scale);
        render_scale = 1.0f;
    }
    
    // Apply quality scaling
    g_display_settings.render_width = (int)(g_display_settings.render_width * render_scale);
    g_display_settings.render_height = (int)(g_display_settings.render_height * render_scale);
    
    // FIXED: Ensure minimum render dimensions
    if (g_display_settings.render_width < 320) g_display_settings.render_width = 320;
    if (g_display_settings.render_height < 240) g_display_settings.render_height = 240;
    
    // Set viewport (always native for Android)
    g_display_settings.viewport_width = g_display_settings.native_width;
    g_display_settings.viewport_height = g_display_settings.native_height;
    
    // Configure OpenGL viewport
    glViewport(0, 0, g_display_settings.viewport_width, g_display_settings.viewport_height);
    
    // FIXED: Check for OpenGL errors
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        LOGE("OpenGL error after glViewport: 0x%x", error);
    }
    
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
    if (validate_resolution_index(index)) {
        return g_resolutions[index].name;
    }
    LOGE("Invalid resolution index: %d", index);
    return NULL;
}

void android_resolution_set_resolution(int index) {
    if (!g_display_settings.initialized) {
        LOGE("Resolution manager not initialized");
        return;
    }
    
    if (validate_resolution_index(index)) {
        g_display_settings.current_resolution_index = index;
        android_resolution_apply_current_settings();
        LOGI("Resolution changed to: %s", g_resolutions[index].name);
    } else {
        LOGE("Invalid resolution index: %d", index);
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
    if (validate_quality_index(index)) {
        return g_quality_presets[index].name;
    }
    LOGE("Invalid quality index: %d", index);
    return NULL;
}

void android_resolution_set_quality(int index) {
    if (!g_display_settings.initialized) {
        LOGE("Resolution manager not initialized");
        return;
    }
    
    if (validate_quality_index(index)) {
        g_display_settings.current_quality_index = index;
        android_resolution_apply_current_settings();
        LOGI("Graphics quality changed to: %s", g_quality_presets[index].name);
    } else {
        LOGE("Invalid quality index: %d", index);
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
    if (!g_display_settings.initialized) {
        LOGE("Resolution manager not initialized");
        return;
    }
    
    g_display_settings.vsync_enabled = enabled ? 1 : 0;
    LOGI("VSync %s", enabled ? "enabled" : "disabled");
    // Note: VSync control on Android is typically handled by the system
}

int android_resolution_get_vsync() {
    return g_display_settings.vsync_enabled;
}

void android_resolution_set_fullscreen(int enabled) {
    if (!g_display_settings.initialized) {
        LOGE("Resolution manager not initialized");
        return;
    }
    
    g_display_settings.fullscreen = enabled ? 1 : 0;
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
    int frame_count;
    long last_update_time;
} PerformanceStats;

static PerformanceStats g_perf_stats = {0};

void android_resolution_update_performance_stats(float fps, float frame_time) {
    // FIXED: Validate input parameters
    if (fps < 0.0f || fps > 1000.0f) {
        LOGE("Invalid FPS value: %f", fps);
        return;
    }
    
    if (frame_time < 0.0f || frame_time > 1000.0f) {
        LOGE("Invalid frame time value: %f", frame_time);
        return;
    }
    
    g_perf_stats.fps = fps;
    g_perf_stats.frame_time = frame_time;
    g_perf_stats.frame_count++;
    
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
    if (!g_display_settings.initialized) {
        LOGE("Resolution manager not initialized");
        return;
    }
    
    // This would analyze device capabilities and set optimal settings
    // For now, use a simple heuristic based on native resolution
    
    long total_pixels = (long)g_display_settings.native_width * g_display_settings.native_height;
    
    if (total_pixels >= 1920L * 1080L) {
        // High-end device
        android_resolution_set_quality(2); // High quality
        android_resolution_set_resolution(6); // Native resolution
    } else if (total_pixels >= 1280L * 720L) {
        // Mid-range device
        android_resolution_set_quality(1); // Medium quality
        android_resolution_set_resolution(4); // 1280x720
    } else {
        // Low-end device
        android_resolution_set_quality(0); // Low quality
        android_resolution_set_resolution(1); // 640x480
    }
    
    LOGI("Auto-detected optimal settings for %dx%d device (total pixels: %ld)", 
         g_display_settings.native_width, g_display_settings.native_height, total_pixels);
}

// Integration with GLTron graphics system
void android_resolution_update_gltron_graphics() {
    if (!g_display_settings.initialized) {
        LOGE("Resolution manager not initialized");
        return;
    }
    
    // This function would integrate with the original GLTron graphics system
    // by providing the Android-specific resolution and quality settings
    
    ResolutionSetting* res = &g_resolutions[g_display_settings.current_resolution_index];
    GraphicsQuality* quality = &g_quality_presets[g_display_settings.current_quality_index];
    
    // Example integration (would need to match GLTron's graphics interface):
    #ifdef GLTRON_GRAPHICS_AVAILABLE
    /*
    gltron_graphics_set_resolution(g_display_settings.render_width, g_display_settings.render_height);
    gltron_graphics_set_texture_quality(quality->texture_quality);
    gltron_graphics_set_shadow_quality(quality->shadow_quality);
    gltron_graphics_set_particle_quality(quality->particle_quality);
    gltron_graphics_set_anti_aliasing(quality->anti_aliasing);
    */
    #endif
    
    LOGI("Updated GLTron graphics settings: %s quality, %s resolution", 
         quality->name, res->name);
}

// FIXED: Add function to set internal data path from JNI
void android_resolution_set_data_path(const char* data_path) {
    if (!data_path) {
        LOGE("Data path is NULL");
        return;
    }
    
    if (!setup_settings_file_path(data_path)) {
        LOGE("Failed to setup settings file path with: %s", data_path);
    }
}

// FIXED: Add cleanup function
void android_resolution_cleanup() {
    if (g_display_settings.initialized) {
        android_resolution_save_settings();
        g_display_settings.initialized = 0;
        memset(g_settings_file_path, 0, sizeof(g_settings_file_path));
        LOGI("Resolution manager cleaned up");
    }
}

