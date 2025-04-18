#include "game/gltron.h"
#include "game/camera.h"
#include "game/event.h"
#include "game/game.h"
#include "video/video.h"
#include "game/engine.h"
#include "video/recognizer.h"
#include "video/nebu_console.h"
#include "configuration/settings.h"
#include "configuration/configuration.h"
#include "scripting/nebu_scripting.h"
#include "input/nebu_input_system.h"
#include "audio/audio.h"
#include "base/nebu_random.h"
#include "scripting/scripting.h"

// Define the Callbacks structure
typedef struct {
    void (*display)(void);
    void (*idle)(void);
    void (*keyboard)(int state, int key, int x, int y);
    void (*init)(void);
    void (*exit)(void);
    void (*mouse)(int buttons, int state, int x, int y);
    void (*mouseMotion)(int x, int y);
    void (*reshape)(int width, int height);
    const char *name;
} Callbacks;

static int startTime = 0;
static int frames = 0;

void idleTimedemo(void) {
	int t;
	int i, j;

	nebu_List *p, *l;

	Sound_idle();
	
	game2->time.current += 20;
	
	for(j = 0; j < 2; j++) { // run display at a theoretical 50 Hz
		t = 10; // run game physics at 100 Hz

		game2->time.dt = 10;
		
		for(i = 0; i < game->players; i++) {
			if(game->player[i].ai.active == AI_COMPUTER &&
				 PLAYER_IS_ACTIVE(&game->player[i])) {
				doComputer(i, 0);
			}
		}

		/* process any outstanding events generated by the AI (turns, etc) */
		for(p = &(game2->events); p->next != NULL; p = p->next) {
			if(processEvent((GameEvent*) p->data))
				return;
		}

		/* free events */
		p = game2->events.next;
		while(p != NULL) {
			l = p;
			p = p->next;
			free(l);
		}
		game2->events.next = NULL;

		doMovement(t);
        // TODO: process events
	}
	
	game2->time.dt = 20;
	doCameraMovement();
	nebu_Input_Mouse_WarpToOrigin();
	doRecognizerMovement();
	scripting_RunGC();
	nebu_System_PostRedisplay();
	frames++;
	game2->time.lastFrame += 20;
}

void keyTimedemo(int state, int key, int x, int y) {
	if(state == NEBU_INPUT_KEYSTATE_UP)
		return;

	if(key == 27)
		nebu_System_ExitLoop(eSRC_Timedemo_Abort);
}

struct saveRules {
	float speed;
	int eraseCrashed;
} saveRules;

void initTimedemo(void) {
	int i = 0;

	printf("-- initializing timedemo\n");

	frames = 0;
	startTime = nebu_Time_GetElapsed();

	nebu_srand(12313);

	resetRecognizer();

	updateSettingsCache();

	// overwrite AI skills & rules in settingsCache
	gSettingsCache.ai_level = 2;
	gSettingsCache.show_ai_status = 0;
	gSettingsCache.show_fps = 0;
	gSettingsCache.camType = CAM_CIRCLE;
	gSettingsCache.show_console = 0;

	saveRules.speed = getSettingf("speed");
	saveRules.eraseCrashed = getSettingi("erase_crashed");

	setSettingf("speed", 12);
	setSettingi("erase_crashed", 1);
		
	game_ResetData();
	changeDisplay(-1);

	for(i = 0; i < game->players; i++) {
		game->player[i].ai.active = AI_COMPUTER;
	}

	nebu_Input_HidePointer();
	nebu_Input_Mouse_WarpToOrigin();
	game2->time.offset = nebu_Time_GetElapsed() - game2->time.current;
}

void exitTimedemo(void) {
	int dt = nebu_Time_GetElapsed() - startTime;
	if(dt) {
		displayMessage(TO_STDERR | TO_CONSOLE, 
			"timedemo FPS: %.2f\n", 
			(float) frames / dt * 1000.0f);
		// displayMessage(TO_STDERR | TO_CONSOLE, "timedemo FPS: %.2f (%d frames in %f seconds)\n", (float) frames / dt * 1000.0f, frames, dt / 1000.0f);
	}
	else {
		// displayMessage(TO_STDERR | TO_CONSOLE, "dt: %d, frames: %d\n", dt, frames);
		// actually, this would be a good reason to abort with a fatal error
	}

	setSettingf("speed", saveRules.speed);
	setSettingi("eraseCrashed", saveRules.eraseCrashed);
}

Callbacks timedemoCallbacks = {
	displayGame, idleTimedemo, keyTimedemo, initTimedemo, exitTimedemo,
	NULL /* mouse button */, NULL /* mouse motion */, "timedemo"
};

