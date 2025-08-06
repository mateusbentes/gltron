#include <time.h>
#include <android/log.h>
#include <android/input.h>
#include <math.h>

#define LOG_TAG "GLTron_Input"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// Touch input state
typedef struct {
    float start_x, start_y;
    float current_x, current_y;
    int is_touching;
    int gesture_detected;
    long touch_start_time;
} TouchState;

static TouchState g_touch_state = {0};

// Game input state
typedef struct {
    int turn_left;
    int turn_right;
    int boost;
    int pause;
} GameInput;

static GameInput g_game_input = {0};

// Touch gesture thresholds
#define MIN_SWIPE_DISTANCE 50.0f
#define MAX_TAP_DURATION 200  // milliseconds
#define MAX_TAP_DISTANCE 20.0f

// Utility functions
static float distance(float x1, float y1, float x2, float y2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    return sqrtf(dx * dx + dy * dy);
}

static long get_time_ms() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

// Input processing functions
void android_input_init() {
    g_touch_state.is_touching = 0;
    g_touch_state.gesture_detected = 0;
    g_game_input.turn_left = 0;
    g_game_input.turn_right = 0;
    g_game_input.boost = 0;
    g_game_input.pause = 0;
    
    LOGI("Android input adapter initialized");
}

void android_input_handle_touch_down(float x, float y) {
    g_touch_state.start_x = x;
    g_touch_state.start_y = y;
    g_touch_state.current_x = x;
    g_touch_state.current_y = y;
    g_touch_state.is_touching = 1;
    g_touch_state.gesture_detected = 0;
    g_touch_state.touch_start_time = get_time_ms();
    
    LOGI("Touch down at (%.2f, %.2f)", x, y);
}

void android_input_handle_touch_move(float x, float y) {
    if (!g_touch_state.is_touching) return;
    
    g_touch_state.current_x = x;
    g_touch_state.current_y = y;
    
    // Check for swipe gestures
    if (!g_touch_state.gesture_detected) {
        float dx = x - g_touch_state.start_x;
        float dy = y - g_touch_state.start_y;
        float dist = distance(g_touch_state.start_x, g_touch_state.start_y, x, y);
        
        if (dist > MIN_SWIPE_DISTANCE) {
            g_touch_state.gesture_detected = 1;
            
            // Determine swipe direction
            if (fabsf(dx) > fabsf(dy)) {
                // Horizontal swipe
                if (dx > 0) {
                    // Swipe right - turn right
                    g_game_input.turn_right = 1;
                    g_game_input.turn_left = 0;
                    LOGI("Swipe right detected - turn right");
                } else {
                    // Swipe left - turn left
                    g_game_input.turn_left = 1;
                    g_game_input.turn_right = 0;
                    LOGI("Swipe left detected - turn left");
                }
            } else {
                // Vertical swipe
                if (dy > 0) {
                    // Swipe down - boost
                    g_game_input.boost = 1;
                    LOGI("Swipe down detected - boost");
                } else {
                    // Swipe up - pause
                    g_game_input.pause = 1;
                    LOGI("Swipe up detected - pause");
                }
            }
        }
    }
}

void android_input_handle_touch_up(float x, float y) {
    if (!g_touch_state.is_touching) return;
    
    long touch_duration = get_time_ms() - g_touch_state.touch_start_time;
    float touch_distance = distance(g_touch_state.start_x, g_touch_state.start_y, x, y);
    
    // Check for tap gesture
    if (!g_touch_state.gesture_detected && 
        touch_duration < MAX_TAP_DURATION && 
        touch_distance < MAX_TAP_DISTANCE) {
        
        g_game_input.pause = 1;
        LOGI("Tap detected - pause");
    }
    
    g_touch_state.is_touching = 0;
    g_touch_state.gesture_detected = 0;
    
    LOGI("Touch up at (%.2f, %.2f)", x, y);
}

// Game input interface
int android_input_get_turn_left() {
    int result = g_game_input.turn_left;
    g_game_input.turn_left = 0;  // Reset after reading
    return result;
}

int android_input_get_turn_right() {
    int result = g_game_input.turn_right;
    g_game_input.turn_right = 0;  // Reset after reading
    return result;
}

int android_input_get_boost() {
    int result = g_game_input.boost;
    g_game_input.boost = 0;  // Reset after reading
    return result;
}

int android_input_get_pause() {
    int result = g_game_input.pause;
    g_game_input.pause = 0;  // Reset after reading
    return result;
}

void android_input_reset() {
    g_game_input.turn_left = 0;
    g_game_input.turn_right = 0;
    g_game_input.boost = 0;
    g_game_input.pause = 0;
}

// Integration with GLTron input system
void android_input_update_gltron_input() {
    // This function would integrate with the original GLTron input system
    // by setting the appropriate input flags or calling input functions
    
    // Example integration (would need to match GLTron's input interface):
    /*
    if (android_input_get_turn_left()) {
        gltron_input_set_turn_left(1);
    }
    if (android_input_get_turn_right()) {
        gltron_input_set_turn_right(1);
    }
    if (android_input_get_boost()) {
        gltron_input_set_boost(1);
    }
    if (android_input_get_pause()) {
        gltron_input_set_pause(1);
    }
    */
}

