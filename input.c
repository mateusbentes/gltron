#include "gltron.h"
#ifdef ANDROID
#include "switchCallbacks.h"
#endif

#define KEYBOARD

void keyGame(unsigned char k, int x, int y) {
  /* Validate game state before processing input */
  if (!game) {
    fprintf(stderr, "keyGame: game is NULL!\n");
    return;
  }

  /* Check if game is paused or finished */
  if (game->pauseflag == PAUSE_GAME_FINISHED) {
    /* Only allow certain keys when game is finished */
    if (k == 27 || k == ' ' || k == 'q') {
      /* Continue with these keys */
    } else {
      /* Ignore other keys when game is finished */
      return;
    }
  }

  switch (k) {
  case 'q': exit(0); break;
  case 27: 
#ifdef ANDROID
    /* Update timing before switching callbacks */
    extern int getElapsedTime(void);
    extern int lasttime;
    lasttime = getElapsedTime();
    android_switchCallbacks(&guiCallbacks);
#else
    switchCallbacks(&guiCallbacks);
#endif
    break;
    /* steering player 0 */
  case 'a': case 'A': 
    /* Check if player exists and is alive */
    if (game->player[0].data && game->player[0].data->speed > 0) {
      turn(game->player[0].data, 3); 
    }
    break;
  case 's': case 'S': 
    /* Check if player exists and is alive */
    if (game->player[0].data && game->player[0].data->speed > 0) {
      turn(game->player[0].data, 1); 
    }
    break;
    /* steering player 1 */
  case 'k': case 'K': 
    if (game->player[1].data && game->player[1].data->speed > 0) {
      turn(game->player[1].data, 3); 
    }
    break;
  case 'l': case 'L': 
    if (game->player[1].data && game->player[1].data->speed > 0) {
      turn(game->player[1].data, 1); 
    }
    break;
    /* steering player 2 */
  case '5': 
    if (game->player[2].data && game->player[2].data->speed > 0) {
      turn(game->player[2].data, 3); 
    }
    break;
  case '6': 
    if (game->player[2].data && game->player[2].data->speed > 0) {
      turn(game->player[2].data, 1); 
    }
    break;
    /* steering player 3 */
    /* cursor keys in specialKey() */
  case ' ': 
#ifdef ANDROID
    /* Update timing before switching callbacks */
    extern int getElapsedTime(void);
    extern int lasttime;
    lasttime = getElapsedTime();
    android_switchCallbacks(&pauseCallbacks);
#else
    switchCallbacks(&pauseCallbacks);
#endif
    break;
    /* case 9: glutIdleFunc(0); break; */
  default: fprintf(stderr, "key %d is not bound\n", k);
  }
}

void specialGame(int key, int x, int y) {
  /* Validate game state before processing input */
  if (!game) {
    fprintf(stderr, "specialGame: game is NULL!\n");
    return;
  }

  /* Check if game is paused or finished */
  if (game->pauseflag == PAUSE_GAME_FINISHED) {
    /* Ignore special keys when game is finished */
    return;
  }

#ifdef ANDROID
  // Android-specific key handling
  switch(key) {
  case 102: // Android equivalent for F1
    if (game && game->settings) {
      defaultDisplay(0);
    }
    break;
  case 103: // Android equivalent for F2
    if (game && game->settings) {
      defaultDisplay(1);
    }
    break;
  case 104: // Android equivalent for F3
    if (game && game->settings) {
      defaultDisplay(2);
    }
    break;
  case 109: // Android equivalent for F10
    if (game && game->settings) {
      game->settings->camType = (game->settings->camType + 1) % CAM_COUNT;
      for(int i = 0; i < game->players; i++) {
        if (game->player[i].camera) {
          game->player[i].camera->camType = game->settings->camType;
        }
      }
    }
    break;
  case 114: // Android equivalent for F5
    if (game && game->settings) {
      saveSettings();
    }
    break;
  }
#else
  // Desktop GLUT key handling
  switch(key) {
  case GLUT_KEY_F1:
    if (game && game->settings) {
      defaultDisplay(0);
    }
    break;
  case GLUT_KEY_F2:
    if (game && game->settings) {
      defaultDisplay(1);
    }
    break;
  case GLUT_KEY_F3:
    if (game && game->settings) {
      defaultDisplay(2);
    }
    break;
  case GLUT_KEY_F10:
    if (game && game->settings) {
      game->settings->camType = (game->settings->camType + 1) % CAM_COUNT;
      for(int i = 0; i < game->players; i++) {
        if (game->player[i].camera) {
          game->player[i].camera->camType = game->settings->camType;
        }
      }
    }
    break;
  case GLUT_KEY_F5:
    if (game && game->settings) {
      saveSettings();
    }
    break;
  }
#endif
}


void parse_args(int argc, char *argv[]) {
  /* Validate game state before processing args */
  if (!game || !game->settings) {
    fprintf(stderr, "parse_args: game or settings is NULL!\n");
    return;
  }

  int i;
  while(argc--) {
    /* Validate argument pointer */
    if (!argv[argc]) {
      fprintf(stderr, "parse_args: argv[%d] is NULL!\n", argc);
      continue;
    }

    if(argv[argc][0] == '-') {
      i = 0;
      while(argv[argc][++i] != 0) {
        switch(argv[argc][i]) {
        case 'k': game->settings->erase_crashed = 1; break;
        case 'f': game->settings->fast_finish = 1; break;
        case 'b': game->settings->show_alpha = 0; break;
        case 'm': game->settings->show_model = 0; break;
        case 'x': game->settings->show_crash_texture = 0; break;
        case 'F': game->settings->show_fps = 0; break;
        case 't': game->settings->show_floor_texture = 0; break;
        case 'c': game->settings->show_ai_status = 0; break;
        case 'g': game->settings->show_glow = 0; break;
        case 'w': game->settings->show_wall = 0; break;
        case 'C': game->settings->show_ai_status = 1; break;
          /* case 'v': game->settings->screenSaver = 1; break; */
        case 'i': game->settings->windowMode = 1; break;
        case 'M': game->settings->mouse_warp = 1; break;
        case '1': /* default is -2 */
          game->settings->width = 320;
          game->settings->height = 240;
	  break;
	case '2': 
	  game->settings->width = 640;
	  game->settings->height = 480;
	  break;
	case '3': 
	  game->settings->width = 800;
	  game->settings->height = 600;
	  break;
	case '4': 
	  game->settings->width = 1024;
	  game->settings->height = 768;
	  break;
	case 's':
	  game->settings->playSound = 0;
	  break;
	case 'h':
	default:
	  printf("Usage: %s [-FftwbghcCsk1234]\n\n", argv[0]);
	  printf("Options:\n\n");
	  printf("-k\terase crashed players (like in the movie)\n");
	  printf("-f\tfast finish after human has crashed\n");
	  printf("-F\tdon't display FPS counter\n");
	  printf("-t\tdon't display floor texture, use lines instead"
		 "(huge speed gain)\n");
	  printf("-w\tdon't display walls (speed gain)\n");
	  printf("-b\tdon't use blending (speed gain)\n");
	  printf("-m\tdon't show lightcycle (speed gain)\n");
	  printf("-x\tdon't show crash texture (speed gain)\n");
	  printf("-g\tdon't show glows (small speed gain)\n");
	  printf("-c\tdon't show ai status\n");
	  printf("-C\tshow ai status (default: on)\n");
	  printf("-1\tSet resolution to 320x240\n");
	  printf("-2\tSet resolution to 640x480 (default)\n");
	  printf("-3\tSet resolution to 800x600\n");
	  printf("-4\tSet resolution to 1024x768\n");
	  printf("-s\tDon't play sound\n");
	  /* printf("-v\tStart in demo/screensaver mode\n"); */
	  /* printf("-i\tforce startup in a window\n"); */
	  printf("-M\tcapture mouse (useful for Voodoo1/2 owners)");
	  printf("-h\tthis help\n");
	  exit(1);
	}
      }
    }
  }
}
