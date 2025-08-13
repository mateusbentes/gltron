#include "game/camera.h"
#include "game/engine.h"
#include "game/event.h"
#include "game/game.h"
#include "game/game_data.h"
#include "game/gltron.h"

#include "scripting/scripting.h"

#include "configuration/configuration.h"

#include "video/video.h"

#include "audio/sound_glue.h"
#include "audio/audio.h"

#include "scripting/nebu_scripting.h"
#include "input/nebu_input_system.h"
#include "base/nebu_callbacks.h"
#include "video/nebu_console.h"

extern video_level *gWorld;  // Use the correct type for gWorld
extern void video_LoadLevel(void);

/* very brief - just the pause mode */

void idlePause(void) {
    Sound_idle();
    // Allow the game to run by not setting dt to 0
    // Instead, let's update the time but at a slower rate
    if (game2 && game2->time.dt > 0) {
        // Slow down time while paused (optional)
        // game2->time.dt = game2->time.dt / 2;
    }
    
    doCameraMovement();
    {
        int dx, dy;
        nebu_Input_Mouse_GetDelta(&dx,&dy);
        if(dx || dy)
            nebu_Input_Mouse_WarpToOrigin();
    }
    scripting_RunGC();
    nebu_Time_FrameDelay(10);
    nebu_System_PostRedisplay();
}

void keyboardPause(int state, int key, int x, int y) {
    fprintf(stderr, "[debug] keyboardPause: state=%d, key=%d\n", state, key);
    
    if(state == NEBU_INPUT_KEYSTATE_UP)
        return;

    switch(key) {
    case 27:
        fprintf(stderr, "[debug] keyboardPause: ESC key pressed, exiting with eSRC_Pause_Escape\n");
        nebu_System_ExitLoop(eSRC_Pause_Escape);
        break;
    case SYSTEM_KEY_F1: changeDisplay(0); break;
    case SYSTEM_KEY_F2: changeDisplay(1); break;
    case SYSTEM_KEY_F3: changeDisplay(2); break;
    case SYSTEM_KEY_F4: changeDisplay(3); break;

        // somehow, this breaks the 'keys' array, and saving
        // at the end of the game fails
        // case SYSTEM_KEY_F5: saveSettings(); return;

    case SYSTEM_KEY_F10: nextCameraType(); break;

    case SYSTEM_KEY_F11: doBmpScreenShot(gScreen); break;
    case SYSTEM_KEY_F12: doPngScreenShot(gScreen); break;
        
    case SYSTEM_KEY_UP: console_Seek(-1); break;
    case SYSTEM_KEY_DOWN: console_Seek(1); break;

    case SYSTEM_KEY_TAB: 
        // nebu_System_ExitLoop(RETURN_MENU_PROMPT);
        break;

    default:
        fprintf(stderr, "[debug] keyboardPause: default case, key=%d\n", key);
        if(game->pauseflag == PAUSE_GAME_FINISHED) {
            fprintf(stderr, "[debug] keyboardPause: game finished, resetting data\n");
            game_ResetData();
            video_ResetData();
            Audio_ResetData();
        }
        else
        {
            fprintf(stderr, "[debug] keyboardPause: unpausing game\n");
            game->pauseflag = PAUSE_GAME_RUNNING;
            nebu_System_ExitLoop(eSRC_Game_Unpause);
        }
        /* lasttime = SystemGetElapsedTime(); */
        break;
    }
}

void initPause(void) {
    /* Check if gWorld is NULL and initialize it if needed */
    if (!gWorld) {
        fprintf(stderr, "[status] initPause: loading level\n");
        video_LoadLevel();
        
        /* Check if initialization was successful */
        if (!gWorld) {
            fprintf(stderr, "[error] initPause: failed to load level\n");
            return;
        }
    }
    
    /* Make sure game is properly initialized */
    if (!game) {
        fprintf(stderr, "[error] initPause: game is NULL\n");
        return;
    }
    
    /* Set the pause flag to indicate we're in pause mode */
    game->pauseflag = PAUSE_GAME_SUSPENDED;
    
    nebu_Input_HidePointer();
    nebu_Input_Mouse_WarpToOrigin();

    /* disable game sound effects */
    Audio_DisableEngine();

    /* 
    * TODO: Provide an option to disable game music here. 
    * Game should be totally silent in pause mode. (Nice when 
    * the boss is walking by, phone call, etc...)
    */

    updateSettingsCache();
}

void exitPause(void) {
    /* If we're exiting pause mode to go to game mode, make sure the game is running */
    if (game) {
        game->pauseflag = PAUSE_GAME_RUNNING;
    }
    
    /* Re-enable game sound effects */
    Audio_EnableEngine();
}

void pauseReshape(int x, int y) {
    /* This function can be empty or handle window resizing if needed */
}

void keyboardPrompt(int state, int key, int x, int y) {
    if(state == NEBU_INPUT_KEYSTATE_UP)
        return;

    switch(key) {
    case 27:
    case SYSTEM_KEY_TAB:
        nebu_System_ExitLoop(eSRC_Pause_Escape);
        break;
    case SYSTEM_KEY_RETURN:
        /* promptEvaluate(); */
        break;
    }
}

void initPrompt(void) { }
void exitPrompt(void) { }

/* Define a reshape function for prompt callbacks */
void promptReshape(int x, int y) {
    /* This function can be empty or handle window resizing if needed */
}
