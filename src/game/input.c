#include "game/game.h"
#include "game/gltron.h"
#include "input/input.h"
#include "video/video.h"
#include "audio/audio.h"
#include "audio/sound_glue.h"
#include "configuration/settings.h"
#include "base/nebu_system.h"
#include <SDL2/SDL.h>
#include <stdio.h>

// Ensure game and game2 are properly declared as external variables
extern Game *game;
extern Game2 *game2;

// Global touch state
TouchState gTouchState = {0, 0, 0, TOUCH_REGION_NONE};

// Function to determine which touch region was pressed
TouchRegion getTouchRegion(int x, int y, int screenWidth, int screenHeight) {
    float leftRegionWidth = screenWidth * 0.25f;
    float rightRegionWidth = screenWidth * 0.25f;
    float upRegionHeight = screenHeight * 0.25f;
    float downRegionHeight = screenHeight * 0.25f;

    if (x < leftRegionWidth) {
        return TOUCH_REGION_LEFT;
    }
    if (x > screenWidth - rightRegionWidth) {
        return TOUCH_REGION_RIGHT;
    }
    if (y < upRegionHeight) {
        return TOUCH_REGION_UP;
    }
    if (y > screenHeight - downRegionHeight) {
        return TOUCH_REGION_DOWN;
    }
    return TOUCH_REGION_BOOST;
}

// Function to handle SDL2 touch/mouse input
void inputHandleSDLEvent(const SDL_Event *event, int screenWidth, int screenHeight) {
    int x = 0, y = 0;
    int state = -1; // 1=down, 0=up, 2=move

    // Handle touch events (SDL2 touch coordinates are normalized 0..1)
    if (event->type == SDL_FINGERDOWN || event->type == SDL_FINGERUP || event->type == SDL_FINGERMOTION) {
        x = (int)(event->tfinger.x * screenWidth);
        y = (int)(event->tfinger.y * screenHeight);
        if (event->type == SDL_FINGERDOWN) state = 1;
        else if (event->type == SDL_FINGERUP) state = 0;
        else if (event->type == SDL_FINGERMOTION) state = 2;
    }
    // Handle mouse events for desktop
    else if (event->type == SDL_MOUSEBUTTONDOWN || event->type == SDL_MOUSEBUTTONUP || event->type == SDL_MOUSEMOTION) {
        x = event->button.x;
        y = event->button.y;
        if (event->type == SDL_MOUSEBUTTONDOWN) state = 1;
        else if (event->type == SDL_MOUSEBUTTONUP) state = 0;
        else if (event->type == SDL_MOUSEMOTION && (event->motion.state & SDL_BUTTON_LMASK)) state = 2;
        else return; // Ignore mouse motion without button pressed
    } else {
        return; // Not a relevant event
    }

    // Now process as before
    if (!game || !game2 || !game->player || game->players <= 0) return;
    Player *player = &game->player[0];
    if (player->data.speed <= 0) return;

    if (state == 1) {  // Touch/mouse down
        gTouchState.active = 1;
        gTouchState.x = x;
        gTouchState.y = y;
        gTouchState.region = getTouchRegion(x, y, screenWidth, screenHeight);

        switch (gTouchState.region) {
            case TOUCH_REGION_LEFT:
                player->data.last_dir = player->data.dir;
                player->data.dir = (player->data.dir + 1) % 4;
                player->data.turn_time = game2->time.current;
                break;
            case TOUCH_REGION_RIGHT:
                player->data.last_dir = player->data.dir;
                player->data.dir = (player->data.dir + 3) % 4;
                player->data.turn_time = game2->time.current;
                break;
            case TOUCH_REGION_UP:
                player->data.speed += 2.0f;
                if (player->data.speed > 20.0f) player->data.speed = 20.0f;
                break;
            case TOUCH_REGION_DOWN:
                player->data.speed -= 2.0f;
                if (player->data.speed < 5.0f) player->data.speed = 5.0f;
                break;
            case TOUCH_REGION_BOOST:
                player->data.boost_enabled = !player->data.boost_enabled;
                if (player->data.boost_enabled) player->data.speed *= 1.5f;
                else player->data.speed /= 1.5f;
                break;
            default:
                break;
        }
    } else if (state == 0) {  // Touch/mouse up
        gTouchState.active = 0;
        gTouchState.region = TOUCH_REGION_NONE;
    } else if (state == 2) {  // Touch/mouse move
        int dx = x - gTouchState.x;
        int dy = y - gTouchState.y;
        if (dx*dx + dy*dy > 100) {
            TouchRegion newRegion = getTouchRegion(x, y, screenWidth, screenHeight);
            if (newRegion != gTouchState.region) {
                gTouchState.region = newRegion;
                gTouchState.x = x;
                gTouchState.y = y;
                switch (gTouchState.region) {
                    case TOUCH_REGION_LEFT:
                        player->data.last_dir = player->data.dir;
                        player->data.dir = (player->data.dir + 1) % 4;
                        player->data.turn_time = game2->time.current;
                        break;
                    case TOUCH_REGION_RIGHT:
                        player->data.last_dir = player->data.dir;
                        player->data.dir = (player->data.dir + 3) % 4;
                        player->data.turn_time = game2->time.current;
                        break;
                    case TOUCH_REGION_UP:
                        player->data.speed += 2.0f;
                        if (player->data.speed > 20.0f) player->data.speed = 20.0f;
                        break;
                    case TOUCH_REGION_DOWN:
                        player->data.speed -= 2.0f;
                        if (player->data.speed < 5.0f) player->data.speed = 5.0f;
                        break;
                    case TOUCH_REGION_BOOST:
                        player->data.boost_enabled = !player->data.boost_enabled;
                        if (player->data.boost_enabled) player->data.speed *= 1.5f;
                        else player->data.speed /= 1.5f;
                        break;
                    default:
                        break;
                }
            }
        }
    }
}

// Function to draw touch control overlay using SDL2
void inputDrawTouchControls(SDL_Renderer *renderer, int screenWidth, int screenHeight) {
    // Enable blending for transparency
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    float leftRegionWidth = screenWidth * 0.25f;
    float rightRegionWidth = screenWidth * 0.25f;
    float upRegionHeight = screenHeight * 0.25f;
    float downRegionHeight = screenHeight * 0.25f;

    SDL_Rect rect;

    // Draw left region (red)
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 77); // 0.3*255 ≈ 77
    rect.x = 0; rect.y = 0; rect.w = (int)leftRegionWidth; rect.h = screenHeight;
    SDL_RenderFillRect(renderer, &rect);

    // Draw right region (green)
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 77);
    rect.x = screenWidth - (int)rightRegionWidth; rect.y = 0; rect.w = (int)rightRegionWidth; rect.h = screenHeight;
    SDL_RenderFillRect(renderer, &rect);

    // Draw up region (blue)
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 77);
    rect.x = (int)leftRegionWidth; rect.y = 0; rect.w = screenWidth - (int)leftRegionWidth - (int)rightRegionWidth; rect.h = (int)upRegionHeight;
    SDL_RenderFillRect(renderer, &rect);

    // Draw down region (yellow)
    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 77);
    rect.x = (int)leftRegionWidth; rect.y = screenHeight - (int)downRegionHeight; rect.w = screenWidth - (int)leftRegionWidth - (int)rightRegionWidth; rect.h = (int)downRegionHeight;
    SDL_RenderFillRect(renderer, &rect);

    // Draw center region (orange)
    SDL_SetRenderDrawColor(renderer, 255, 128, 0, 77);
    rect.x = (int)leftRegionWidth; rect.y = (int)upRegionHeight; rect.w = screenWidth - (int)leftRegionWidth - (int)rightRegionWidth; rect.h = screenHeight - (int)upRegionHeight - (int)downRegionHeight;
    SDL_RenderFillRect(renderer, &rect);

    // Optionally: Draw outlines or labels using SDL2_gfx or SDL_ttf for text
}
