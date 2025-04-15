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
    
    // Update each player
    int i;
    for(i = 0; i < game->players; i++) {
        // Skip inactive players
        if(game->player[i].data.boost_enabled == 0) {  // Using boost_enabled as a proxy for alive
            continue;
        }
        
        // Move player based on direction and speed
        float dt = 0.1f; // Use a fixed dt for now
        float speed = 10.0f * dt;  // Using a fixed speed for now
        
        // Update position based on current direction
        int direction = game->player[i].data.dir;  // Get the actual direction from player data
        
        // Store position in temporary variables
        float posx = 0.0f;
        float posy = 0.0f;
        
        // Get current position from player data
        getPositionFromData(&posx, &posy, &game->player[i].data);
        
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
        
        // Update player position
        game->player[i].data.posx = posx;
        game->player[i].data.posy = posy;
        
        // Check for collisions with walls
        float grid_size = 95.0f; // Use a fixed grid size for now
        if (posx < -grid_size || 
            posx > grid_size || 
            posy < -grid_size || 
            posy > grid_size) {
            // Player hit a wall
            fprintf(stderr, "[game] Player %d hit a wall\n", i);
            game->player[i].data.boost_enabled = 0;  // Using boost_enabled as a proxy for alive
            
            // Play crash sound if available
            // Audio_CrashPlayer(i);
        }
        
        // Check for collisions with trails (to be implemented)
        
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
    }
    
    // Check game state (e.g., if only one player is left alive)
    int alive_count = 0;
    int last_alive = -1;
    
    for(i = 0; i < game->players; i++) {
        if(game->player[i].data.boost_enabled) {  // Using boost_enabled as a proxy for alive
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
