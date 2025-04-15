#include "game/game.h"
#include "game/gltron.h"
#include "input/input.h"
#include "video/video.h"
#include "audio/audio.h"
#include "audio/sound_glue.h"
#include "configuration/settings.h"
#include "base/nebu_system.h"

// Add OpenGL headers
#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

// Ensure game and game2 are properly declared as external variables
extern Game *game;
extern Game2 *game2;

// Define key constants if they're not already defined
#ifndef SYSTEM_KEY_LEFT
#define SYSTEM_KEY_LEFT SDLK_LEFT
#endif

#ifndef SYSTEM_KEY_RIGHT
#define SYSTEM_KEY_RIGHT SDLK_RIGHT
#endif

#ifndef SYSTEM_KEY_UP
#define SYSTEM_KEY_UP SDLK_UP
#endif

#ifndef SYSTEM_KEY_DOWN
#define SYSTEM_KEY_DOWN SDLK_DOWN
#endif

#ifndef SYSTEM_KEY_SPACE
#define SYSTEM_KEY_SPACE SDLK_SPACE
#endif

#ifndef SYSTEM_KEYPRESS
#define SYSTEM_KEYPRESS 1
#endif

// Touch control regions
typedef enum {
    TOUCH_REGION_NONE = 0,
    TOUCH_REGION_LEFT,
    TOUCH_REGION_RIGHT,
    TOUCH_REGION_UP,
    TOUCH_REGION_DOWN,
    TOUCH_REGION_BOOST
} TouchRegion;

// Touch control state
typedef struct {
    int active;             // Is touch currently active
    int x, y;               // Current touch position
    TouchRegion region;     // Current touch region
} TouchState;

// Global touch state
TouchState gTouchState = {0, 0, 0, TOUCH_REGION_NONE};

// Function to determine which touch region was pressed
TouchRegion getTouchRegion(int x, int y, int screenWidth, int screenHeight) {
    // Define regions as percentages of screen size
    float leftRegionWidth = screenWidth * 0.25f;
    float rightRegionWidth = screenWidth * 0.25f;
    float upRegionHeight = screenHeight * 0.25f;
    float downRegionHeight = screenHeight * 0.25f;
    
    // Left region (left 25% of screen)
    if (x < leftRegionWidth) {
        return TOUCH_REGION_LEFT;
    }
    
    // Right region (right 25% of screen)
    if (x > screenWidth - rightRegionWidth) {
        return TOUCH_REGION_RIGHT;
    }
    
    // Up region (top 25% of screen)
    if (y < upRegionHeight) {
        return TOUCH_REGION_UP;
    }
    
    // Down region (bottom 25% of screen)
    if (y > screenHeight - downRegionHeight) {
        return TOUCH_REGION_DOWN;
    }
    
    // Center region (boost)
    return TOUCH_REGION_BOOST;
}

// Function to handle touch input
void touchGame(int state, int x, int y, int screenWidth, int screenHeight) {
    // Check if game is NULL
    if (!game) {
        fprintf(stderr, "[input] game is NULL, cannot process touch input\n");
        return;
    }
    
    // Check if game2 is NULL
    if (!game2) {
        fprintf(stderr, "[input] game2 is NULL, cannot process touch input\n");
        return;
    }
    
    // Check if player array exists
    if (!game->player) {
        fprintf(stderr, "[input] game->player is NULL, cannot process touch input\n");
        return;
    }
    
    // Check if there are any players
    if (game->players <= 0) {
        fprintf(stderr, "[input] No players available (game->players = %d)\n", game->players);
        return;
    }
    
    // Get the human player (player 0)
    Player *player = &game->player[0];
    
    // Check if player is active
    if (player->data.speed <= 0) {
        fprintf(stderr, "[input] Player is inactive (speed = %f), ignoring touch input\n", player->data.speed);
        return;  // Player is inactive, ignore input
    }
    
    // Handle touch state
    if (state == 1) {  // Touch down
        gTouchState.active = 1;
        gTouchState.x = x;
        gTouchState.y = y;
        gTouchState.region = getTouchRegion(x, y, screenWidth, screenHeight);
        
        printf("[input] Touch down at (%d, %d) in region %d\n", x, y, gTouchState.region);
        
        // Process the touch region
        switch (gTouchState.region) {
            case TOUCH_REGION_LEFT:
                // Turn left (counter-clockwise)
                player->data.last_dir = player->data.dir;
                player->data.dir = (player->data.dir + 1) % 4;
                player->data.turn_time = game2->time.current;
                printf("[input] Player turned left to direction %d\n", player->data.dir);
                break;
                
            case TOUCH_REGION_RIGHT:
                // Turn right (clockwise)
                player->data.last_dir = player->data.dir;
                player->data.dir = (player->data.dir + 3) % 4;  // +3 is equivalent to -1 in modulo 4
                player->data.turn_time = game2->time.current;
                printf("[input] Player turned right to direction %d\n", player->data.dir);
                break;
                
            case TOUCH_REGION_UP:
                // Increase speed
                player->data.speed += 2.0f;
                if (player->data.speed > 20.0f) {
                    player->data.speed = 20.0f;  // Cap speed
                }
                printf("[input] Player speed increased to %f\n", player->data.speed);
                break;
                
            case TOUCH_REGION_DOWN:
                // Decrease speed
                player->data.speed -= 2.0f;
                if (player->data.speed < 5.0f) {
                    player->data.speed = 5.0f;  // Minimum speed
                }
                printf("[input] Player speed decreased to %f\n", player->data.speed);
                break;
                
            case TOUCH_REGION_BOOST:
                // Toggle boost
                player->data.boost_enabled = !player->data.boost_enabled;
                if (player->data.boost_enabled) {
                    player->data.speed *= 1.5f;  // Boost speed
                    printf("[input] Boost enabled, speed: %f\n", player->data.speed);
                } else {
                    player->data.speed /= 1.5f;  // Return to normal speed
                    printf("[input] Boost disabled, speed: %f\n", player->data.speed);
                }
                break;
                
            default:
                break;
        }
    } else if (state == 0) {  // Touch up
        printf("[input] Touch up at (%d, %d)\n", x, y);
        gTouchState.active = 0;
        gTouchState.region = TOUCH_REGION_NONE;
    } else if (state == 2) {  // Touch move
        // Only process move if it's a significant change
        int dx = x - gTouchState.x;
        int dy = y - gTouchState.y;
        
        if (dx*dx + dy*dy > 100) {  // Threshold for movement
            TouchRegion newRegion = getTouchRegion(x, y, screenWidth, screenHeight);
            
            // Only process if region changed
            if (newRegion != gTouchState.region) {
                printf("[input] Touch moved to region %d\n", newRegion);
                gTouchState.region = newRegion;
                gTouchState.x = x;
                gTouchState.y = y;
                
                // Process the new touch region (same logic as touch down)
                switch (gTouchState.region) {
                    case TOUCH_REGION_LEFT:
                        // Turn left (counter-clockwise)
                        player->data.last_dir = player->data.dir;
                        player->data.dir = (player->data.dir + 1) % 4;
                        player->data.turn_time = game2->time.current;
                        printf("[input] Player turned left to direction %d\n", player->data.dir);
                        break;
                        
                    case TOUCH_REGION_RIGHT:
                        // Turn right (clockwise)
                        player->data.last_dir = player->data.dir;
                        player->data.dir = (player->data.dir + 3) % 4;  // +3 is equivalent to -1 in modulo 4
                        player->data.turn_time = game2->time.current;
                        printf("[input] Player turned right to direction %d\n", player->data.dir);
                        break;
                        
                    case TOUCH_REGION_UP:
                        // Increase speed
                        player->data.speed += 2.0f;
                        if (player->data.speed > 20.0f) {
                            player->data.speed = 20.0f;  // Cap speed
                        }
                        printf("[input] Player speed increased to %f\n", player->data.speed);
                        break;
                        
                    case TOUCH_REGION_DOWN:
                        // Decrease speed
                        player->data.speed -= 2.0f;
                        if (player->data.speed < 5.0f) {
                            player->data.speed = 5.0f;  // Minimum speed
                        }
                        printf("[input] Player speed decreased to %f\n", player->data.speed);
                        break;
                        
                    case TOUCH_REGION_BOOST:
                        // Toggle boost
                        player->data.boost_enabled = !player->data.boost_enabled;
                        if (player->data.boost_enabled) {
                            player->data.speed *= 1.5f;  // Boost speed
                            printf("[input] Boost enabled, speed: %f\n", player->data.speed);
                        } else {
                            player->data.speed /= 1.5f;  // Return to normal speed
                            printf("[input] Boost disabled, speed: %f\n", player->data.speed);
                        }
                        break;
                        
                    default:
                        break;
                }
            }
        }
    }
}

void keyGame(int state, int k, int x, int y) {
    // Check if game is NULL
    if (!game) {
        fprintf(stderr, "[input] game is NULL, cannot process input\n");
        return;
    }
    
    // Check if game2 is NULL
    if (!game2) {
        fprintf(stderr, "[input] game2 is NULL, cannot process input\n");
        return;
    }
    
    // Check if player array exists
    if (!game->player) {
        fprintf(stderr, "[input] game->player is NULL, cannot process input\n");
        return;
    }
    
    // Check if there are any players
    if (game->players <= 0) {
        fprintf(stderr, "[input] No players available (game->players = %d)\n", game->players);
        return;
    }
    
    // Only process key press events (state == SYSTEM_KEYPRESS)
    if (state != SYSTEM_KEYPRESS) {
        return;
    }
    
    // Get the human player (player 0)
    Player *player = &game->player[0];
    
    // Check if player is active
    if (player->data.speed <= 0) {
        fprintf(stderr, "[input] Player is inactive (speed = %f), ignoring input\n", player->data.speed);
        return;  // Player is inactive, ignore input
    }
    
    // Print the key code for debugging
    printf("[input] Key pressed: %d\n", k);
    
    // Handle direction changes
    switch (k) {
        case SYSTEM_KEY_LEFT:
            // Turn left (counter-clockwise)
            player->data.last_dir = player->data.dir;
            player->data.dir = (player->data.dir + 1) % 4;
            player->data.turn_time = game2->time.current;
            printf("[input] Player turned left to direction %d\n", player->data.dir);
            break;
            
        case SYSTEM_KEY_RIGHT:
            // Turn right (clockwise)
            player->data.last_dir = player->data.dir;
            player->data.dir = (player->data.dir + 3) % 4;  // +3 is equivalent to -1 in modulo 4
            player->data.turn_time = game2->time.current;
            printf("[input] Player turned right to direction %d\n", player->data.dir);
            break;
            
        case SYSTEM_KEY_UP:
            // Increase speed
            player->data.speed += 2.0f;
            if (player->data.speed > 20.0f) {
                player->data.speed = 20.0f;  // Cap speed
            }
            printf("[input] Player speed increased to %f\n", player->data.speed);
            break;
            
        case SYSTEM_KEY_DOWN:
            // Decrease speed
            player->data.speed -= 2.0f;
            if (player->data.speed < 5.0f) {
                player->data.speed = 5.0f;  // Minimum speed
            }
            printf("[input] Player speed decreased to %f\n", player->data.speed);
            break;
            
        case SYSTEM_KEY_SPACE:
            // Toggle boost
            player->data.boost_enabled = !player->data.boost_enabled;
            if (player->data.boost_enabled) {
                player->data.speed *= 1.5f;  // Boost speed
                printf("[input] Boost enabled, speed: %f\n", player->data.speed);
            } else {
                player->data.speed /= 1.5f;  // Return to normal speed
                printf("[input] Boost disabled, speed: %f\n", player->data.speed);
            }
            break;
            
        case 27:  // ESC key
            // Pause game
            game->pauseflag = PAUSE_GAME_SUSPENDED;
            printf("[input] Game paused\n");
            break;
            
        // Add alternative key bindings for WASD controls
        case 'a':
        case 'A':
            // Turn left (counter-clockwise)
            player->data.last_dir = player->data.dir;
            player->data.dir = (player->data.dir + 1) % 4;
            player->data.turn_time = game2->time.current;
            printf("[input] Player turned left to direction %d\n", player->data.dir);
            break;
            
        case 'd':
        case 'D':
            // Turn right (clockwise)
            player->data.last_dir = player->data.dir;
            player->data.dir = (player->data.dir + 3) % 4;  // +3 is equivalent to -1 in modulo 4
            player->data.turn_time = game2->time.current;
            printf("[input] Player turned right to direction %d\n", player->data.dir);
            break;
            
        case 'w':
        case 'W':
            // Increase speed
            player->data.speed += 2.0f;
            if (player->data.speed > 20.0f) {
                player->data.speed = 20.0f;  // Cap speed
            }
            printf("[input] Player speed increased to %f\n", player->data.speed);
            break;
            
        case 's':
        case 'S':
            // Decrease speed
            player->data.speed -= 2.0f;
            if (player->data.speed < 5.0f) {
                player->data.speed = 5.0f;  // Minimum speed
            }
            printf("[input] Player speed decreased to %f\n", player->data.speed);
            break;
            
        default:
            // Other keys
            printf("[input] Unhandled key: %d\n", k);
            break;
    }
}

// Function to draw touch control overlay
void drawTouchControls(int screenWidth, int screenHeight) {
    // Enable blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Disable depth testing to ensure overlay is visible
    glDisable(GL_DEPTH_TEST);
    
    // Set up orthographic projection for 2D drawing
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, screenWidth, screenHeight, 0, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    // Define regions as percentages of screen size
    float leftRegionWidth = screenWidth * 0.25f;
    float rightRegionWidth = screenWidth * 0.25f;
    float upRegionHeight = screenHeight * 0.25f;
    float downRegionHeight = screenHeight * 0.25f;
    
    // Draw left region
    glColor4f(1.0f, 0.0f, 0.0f, 0.3f);  // Red with transparency
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(leftRegionWidth, 0);
    glVertex2f(leftRegionWidth, screenHeight);
    glVertex2f(0, screenHeight);
    glEnd();
    
    // Draw right region
    glColor4f(0.0f, 1.0f, 0.0f, 0.3f);  // Green with transparency
    glBegin(GL_QUADS);
    glVertex2f(screenWidth - rightRegionWidth, 0);
    glVertex2f(screenWidth, 0);
    glVertex2f(screenWidth, screenHeight);
    glVertex2f(screenWidth - rightRegionWidth, screenHeight);
    glEnd();
    
    // Draw up region
    glColor4f(0.0f, 0.0f, 1.0f, 0.3f);  // Blue with transparency
    glBegin(GL_QUADS);
    glVertex2f(leftRegionWidth, 0);
    glVertex2f(screenWidth - rightRegionWidth, 0);
    glVertex2f(screenWidth - rightRegionWidth, upRegionHeight);
    glVertex2f(leftRegionWidth, upRegionHeight);
    glEnd();
    
    // Draw down region
    glColor4f(1.0f, 1.0f, 0.0f, 0.3f);  // Yellow with transparency
    glBegin(GL_QUADS);
    glVertex2f(leftRegionWidth, screenHeight - downRegionHeight);
    glVertex2f(screenWidth - rightRegionWidth, screenHeight - downRegionHeight);
    glVertex2f(screenWidth - rightRegionWidth, screenHeight);
    glVertex2f(leftRegionWidth, screenHeight);
    glEnd();
    
    // Draw center region (boost)
    glColor4f(1.0f, 0.5f, 0.0f, 0.3f);  // Orange with transparency
    glBegin(GL_QUADS);
    glVertex2f(leftRegionWidth, upRegionHeight);
    glVertex2f(screenWidth - rightRegionWidth, upRegionHeight);
    glVertex2f(screenWidth - rightRegionWidth, screenHeight - downRegionHeight);
    glVertex2f(leftRegionWidth, screenHeight - downRegionHeight);
    glEnd();
    
    // Draw labels
    // (This would require text rendering, which is beyond the scope of this example)
    
    // Restore matrices
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    
    // Restore OpenGL state
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}
