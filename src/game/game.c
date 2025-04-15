#include "filesystem/path.h"
#include "game/gltron.h"
#include "game/timesystem.h"
#include "base/nebu_callbacks.h"
#include "game/game.h"
#include "game/camera.h"
#include "game/engine.h"
#include "input/input.h"
#include "input/nebu_input_system.h"
#include "video/video.h"
#include "audio/audio.h"
#include "video/nebu_video_system.h"
#include "scripting/nebu_scripting.h"
#include "audio/sound_glue.h"
#include "configuration/settings.h"
#include "configuration/configuration.h"

#include "base/nebu_debug_memory.h"

#include "base/nebu_assert.h"

// Touch input handling function declarations
void touchGame(int state, int x, int y, int screenWidth, int screenHeight);
void drawTouchControls(int screenWidth, int screenHeight);

void getPositionFromData(float *x, float *y, Data *data) {
    if (!data) {
        fprintf(stderr, "[getPositionFromData] Data is NULL\n");
        *x = 0.0f;
        *y = 0.0f;
        return;
    }
    
    // Get position from the data structure
    *x = data->posx;
    *y = data->posy;
    
    printf("[getPositionFromData] Position: (%f, %f)\n", *x, *y);
}

void Game_Update(void) {
    // Check if game2 is NULL
    if (!game2) {
        fprintf(stderr, "[game] game2 is NULL, cannot update game\n");
        return;
    }
    
    // Check if game is NULL
    if (!game) {
        fprintf(stderr, "[game] game is NULL, cannot update game\n");
        return;
    }
    
    // Get time delta
    float dt = game2->time.dt;
    if (dt <= 0) dt = 0.01f;  // Prevent division by zero
    
    // Update each player
    int i;
    for(i = 0; i < game->players; i++) {
        // Skip inactive players
        if(game->player[i].data.speed <= 0) {
            continue;
        }
        
        // Move player based on direction and speed
        float speed = game->player[i].data.speed * dt;
        
        // Update position based on current direction
        int direction = game->player[i].data.dir;
        
        // Store position in temporary variables
        float posx = game->player[i].data.posx;
        float posy = game->player[i].data.posy;
        
        // Update position based on direction
        switch(direction) {
            case 0: // Right
                posx += speed;
                break;
            case 1: // Up
                posy += speed;
                break;
            case 2: // Left
                posx -= speed;
                break;
            case 3: // Down
                posy -= speed;
                break;
        }
        
        // Check for collisions with walls
        float grid_size = 95.0f;
        if (posx < -grid_size || posx > grid_size || 
            posy < -grid_size || posy > grid_size) {
            // Player hit a wall
            fprintf(stderr, "[game] Player %d hit a wall at position (%f, %f)\n", 
                   i, posx, posy);
            
            // Disable player (set speed to 0)
            game->player[i].data.speed = 0;
            
            // Set explosion radius (for visual effect)
            game->player[i].data.exp_radius = 1.0f;
            
            // Play crash sound if available
            // Audio_CrashPlayer(i);
            
            continue;  // Skip to next player
        }
        
        // Check for collisions with other players' trails
        // This is a simplified version - in a real game, you'd check against actual trail segments
        int j;
        for(j = 0; j < game->players; j++) {
            // Skip self and inactive players
            if(j == i || game->player[j].data.speed <= 0) {
                continue;
            }
            
            // Simple collision check - are we very close to another player?
            float dx = posx - game->player[j].data.posx;
            float dy = posy - game->player[j].data.posy;
            float distance = sqrt(dx*dx + dy*dy);
            
            if (distance < 4.0f) {  // Collision threshold
                // Player hit another player
                fprintf(stderr, "[game] Player %d collided with player %d\n", i, j);
                
                // Disable player (set speed to 0)
                game->player[i].data.speed = 0;
                
                // Set explosion radius (for visual effect)
                game->player[i].data.exp_radius = 1.0f;
                
                // Play crash sound if available
                // Audio_CrashPlayer(i);
                
                goto next_player;  // Skip to next player
            }
        }
        
        // Update player position
        game->player[i].data.posx = posx;
        game->player[i].data.posy = posy;
        
        // Update camera for this player if needed
        if (gppPlayerVisuals && gppPlayerVisuals[i]) {
            Camera *cam = &gppPlayerVisuals[i]->camera;
            cam->target[0] = posx;
            cam->target[1] = posy;
            cam->target[2] = 0.0f;
            
            // Position camera based on player direction
            float cam_distance = 20.0f;
            float cam_height = 10.0f;
            
            switch(direction) {
                case 0: // Right
                    cam->cam[0] = cam->target[0] - cam_distance;
                    cam->cam[1] = cam->target[1];
                    cam->cam[2] = cam_height;
                    break;
                case 1: // Up
                    cam->cam[0] = cam->target[0];
                    cam->cam[1] = cam->target[1] - cam_distance;
                    cam->cam[2] = cam_height;
                    break;
                case 2: // Left
                    cam->cam[0] = cam->target[0] + cam_distance;
                    cam->cam[1] = cam->target[1];
                    cam->cam[2] = cam_height;
                    break;
                case 3: // Down
                    cam->cam[0] = cam->target[0];
                    cam->cam[1] = cam->target[1] + cam_distance;
                    cam->cam[2] = cam_height;
                    break;
            }
        }
        
    next_player:
        continue;
    }
    
    // Check game state (e.g., if only one player is left alive)
    int alive_count = 0;
    int last_alive = -1;
    
    for(i = 0; i < game->players; i++) {
        if(game->player[i].data.speed > 0) {
            alive_count++;
            last_alive = i;
        }
    }
    
    // If only one player is left alive, they win the round
    if (alive_count == 1 && game->players > 1) {
        fprintf(stderr, "[game] Player %d wins the round\n", last_alive);
        // Could trigger victory sequence here
    }
    // If no players are alive, round is a draw
    else if (alive_count == 0 && game->players > 0) {
        fprintf(stderr, "[game] Round ended in a draw\n");
        // Could trigger draw sequence here
    }
}

void Game_Idle(void) {
    // Get screen dimensions
    int screenWidth = 800;  // Default width
    int screenHeight = 600; // Default height
    
    // If we have access to the actual screen dimensions, use those instead
    nebu_Video_GetDimension(&screenWidth, &screenHeight);
    
    // Draw touch controls if touch is enabled
    #if defined(ANDROID) || defined(__ANDROID__) || defined(IOS) || defined(__IOS__)
    drawTouchControls(screenWidth, screenHeight);
    #endif
}

void GameMode_Idle(void) {
	Sound_idle();
	Time_Idle();
	if(game2->time.dt == 0)
		return;
	Game_Update();
	Game_Idle();
	Video_Idle();
	Input_Idle();
	Scripting_Idle();
	nebu_Time_FrameDelay(5);
	nebu_System_PostRedisplay();
}

void enterGame(void) { /* called when game mode is entered */
	updateSettingsCache();

	nebu_Input_HidePointer();
	nebu_Input_Mouse_WarpToOrigin();
	game2->time.offset = nebu_Time_GetElapsed() - game2->time.current;
	Audio_EnableEngine();
 
	// disable booster & wallbuster
	{
		int i;
		for(i = 0; i < game->players; i++) {
			game->player[i].data.boost_enabled = 0;
			game->player[i].data.wall_buster_enabled = 0;
		}
	}
	/* fprintf(stderr, "init game\n"); */
}

void exitGame(void) {
  Audio_DisableEngine();
  /* fprintf(stderr, "exit game\n"); */
}

void gameMouse(int buttons, int state, int x, int y) {
	if(state == SYSTEM_MOUSEPRESSED) {
		if(buttons == SYSTEM_MOUSEBUTTON_LEFT) gInput.mouse1 = 1;
		if(buttons == SYSTEM_MOUSEBUTTON_RIGHT) gInput.mouse2 = 1;
	} else if(state == SYSTEM_MOUSERELEASED) {
		if(buttons == SYSTEM_MOUSEBUTTON_LEFT) gInput.mouse1 = 0;
		if(buttons == SYSTEM_MOUSEBUTTON_RIGHT) gInput.mouse2 = 0;
	}

  /*
  if(getSettingi("camType") == CAM_TYPE_MOUSE) 
    if(state == SYSTEM_MOUSEPRESSED) {
      if(buttons == SYSTEM_MOUSEBUTTON_LEFT) {
	cam_r -= CAM_DR;
	if(cam_r < CAM_R_MIN) cam_r = CAM_R_MIN;
      } else if(buttons == SYSTEM_MOUSEBUTTON_RIGHT) {
	cam_r += CAM_DR;
	if(cam_r > CAM_R_MAX) cam_r = CAM_R_MAX;
      }
    }
  */
  /* fprintf(stderr, "new cam_r: %.2f\n", cam_r); */
  
  // On touch-enabled platforms, also handle this as a touch event
  #if defined(ANDROID) || defined(__ANDROID__) || defined(IOS) || defined(__IOS__)
  int screenWidth = 800, screenHeight = 600;
  nebu_Video_GetDimension(&screenWidth, &screenHeight);
  
  int touchState = (state == SYSTEM_MOUSEPRESSED) ? 1 : 
                  ((state == SYSTEM_MOUSERELEASED) ? 0 : 2);
  touchGame(touchState, x, y, screenWidth, screenHeight);
  #endif
}

// Function prototype for reshape callback
void gameReshape(int x, int y);

Callbacks gameCallbacks = { 
  displayGame, GameMode_Idle, keyGame, enterGame, exitGame, gameMouse, gameReshape, "game"
};

// Implementation of the reshape callback
void gameReshape(int x, int y) {
    // This function is called when the window is resized
    printf("[game] Window resized to %dx%d\n", x, y);
}

// Touch input handling implementation
void touchGame(int state, int x, int y, int screenWidth, int screenHeight) {
    // Skip if game is not initialized
    if (!game || !game2) {
        return;
    }
    
    // Define touch control regions
    int leftRegion = screenWidth / 4;
    int rightRegion = screenWidth * 3 / 4;
    int topRegion = screenHeight / 4;
    int bottomRegion = screenHeight * 3 / 4;
    
    // Current player (assuming player 0 for touch controls)
    int player = 0;
    
    // Handle touch based on state
    if (state == 1) { // Touch down
        // Determine direction based on touch position
        if (x < leftRegion) {
            // Turn left (relative to current direction)
            int newDir = (game->player[player].data.dir + 1) % 4;
            game->player[player].data.dir = newDir;
        } else if (x > rightRegion) {
            // Turn right (relative to current direction)
            int newDir = (game->player[player].data.dir + 3) % 4;
            game->player[player].data.dir = newDir;
        } else if (y < topRegion) {
            // Boost if available
            if (game->player[player].data.boost_enabled == 0) {
                game->player[player].data.boost_enabled = 1;
            }
        } else if (y > bottomRegion) {
            // Wall buster if available
            if (game->player[player].data.wall_buster_enabled == 0) {
                game->player[player].data.wall_buster_enabled = 1;
            }
        }
    } else if (state == 0) { // Touch up
        // Disable boosts on touch release if in boost region
        if (y < topRegion) {
            game->player[player].data.boost_enabled = 0;
        } else if (y > bottomRegion) {
            game->player[player].data.wall_buster_enabled = 0;
        }
    }
    
    printf("[touchGame] Touch %s at (%d, %d)\n", 
           state == 1 ? "down" : (state == 0 ? "up" : "move"), x, y);
}

// Draw touch controls overlay
void drawTouchControls(int screenWidth, int screenHeight) {
    // This function would draw the touch control UI elements
    // Implementation depends on the rendering system used
    
    // For now, just log that we're drawing controls
    printf("[drawTouchControls] Drawing touch controls for %dx%d screen\n", 
           screenWidth, screenHeight);
    
    // In a real implementation, this would draw:
    // - Left/right turn buttons
    // - Boost button
    // - Wall buster button
}
