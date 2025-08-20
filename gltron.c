/*
  gltron 0.50 beta
  Copyright (C) 1999 by Andreas Umbach <marvin@dataway.ch>
*/

#include "gltron.h"

/* todo: define the globals where I need them */
/* declare them only in gltron.h */

#include "globals.h"

int getElapsedTime(void) {
#ifdef WIN32
	return timeGetTime();
#else
  return glutGet(GLUT_ELAPSED_TIME);
#endif
}

void mouseWarp() {
  glutWarpPointer(game->screen->w / 2, game->screen->h / 2);
}

void drawGame() {
  GLint i;
  gDisplay *d;
  Player *p;

  polycount = 0;
  glClearColor(.0, .0, .0, .0);
  glDepthMask(GL_TRUE);
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glDepthMask(GL_FALSE);

  for(i = 0; i < vp_max[ game->settings->display_type]; i++) {
    p = &(game->player[ game->settings->content[i] ]);
    if(p->display->onScreen == 1) {
      d = p->display;
      glViewport(d->vp_x, d->vp_y, d->vp_w, d->vp_h);
      drawCam(p, d);
      drawScore(p, d);
      if(game->settings->show_ai_status)
	if(p->ai->active == 1)
	  drawAI(d);
    }
  }

  if(game->settings->show_2d > 0)
    drawDebugTex(game->screen);
  if(game->settings->show_fps)
    drawFPS(game->screen);

  /*
  if(game->settings->show_help == 1)
    drawHelp(game->screen);
  */

  /* printf("%d polys\n", polycount); */
}
void displayGame() {
  /* Ensure viewports match window size at draw time after any display change */
  forceViewportResetIfNeededForGame();
  drawGame();
  if(game->settings->mouse_warp)
    mouseWarp();
  glutSwapBuffers();
}

void initCustomLights() {
  float col[] = { .77, .77, .77, 1.0 };
  float dif[] =  { 0.4, 0.4, 0.4, 1};
  float amb[] = { 0.25, 0.25, 0.25, 1};

  glEnable(GL_LIGHT0);
  glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
  glLightfv(GL_LIGHT0, GL_SPECULAR, col);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
}

void initGLGame() {
  printf("OpenGL Info: '%s'\n%s - %s\n", glGetString(GL_VENDOR),
	 glGetString(GL_RENDERER), glGetString(GL_VERSION));

  glShadeModel( GL_FLAT ); /* ugly debug mode */


  if(game->settings->show_alpha) 
    glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  
  glFogf(GL_FOG_START, 50.0);
  glFogf(GL_FOG_END, 100.0);
  glFogf(GL_FOG_MODE, GL_LINEAR);
  glFogf(GL_FOG_DENSITY, 0.1);
  glDisable(GL_FOG);


  /* TODO(3): incorporate model stuff */
  /* initLightAndMaterial(); */
  initCustomLights();

  glDepthMask(GL_FALSE);
  glDisable(GL_DEPTH_TEST);
}

int initWindow() {
  int win_id;
  /* char buf[20]; */

  glutInitWindowSize(game->settings->width, game->settings->height);
  glutInitWindowPosition(0, 0);

  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);

  /*
  sprintf(buf, "%dx%d:16", game->settings->width, game->settings->height);
  glutGameModeString(buf);

  if(glutGameModeGet(GLUT_GAME_MODE_POSSIBLE) &&
     !game->settings->windowMode) {
     win_id = glutEnterGameMode();
  */
    /* check glutGameMode results */
  /*
    printf("Glut game mode status\n");
    printf("  active: %d\n", glutGameModeGet( GLUT_GAME_MODE_ACTIVE));
    printf("  possible: %d\n", glutGameModeGet( GLUT_GAME_MODE_POSSIBLE));
    printf("  width: %d\n", glutGameModeGet( GLUT_GAME_MODE_WIDTH));
    printf("  height: %d\n", glutGameModeGet( GLUT_GAME_MODE_HEIGHT));
    printf("  depth: %d\n", glutGameModeGet( GLUT_GAME_MODE_PIXEL_DEPTH));
    printf("  refresh: %d\n", glutGameModeGet( GLUT_GAME_MODE_REFRESH_RATE));
    printf("  changed display: %d\n",
	   glutGameModeGet( GLUT_GAME_MODE_DISPLAY_CHANGED));

  } else
  */
    win_id = glutCreateWindow("gltron");
  if (win_id < 0) {
    printf("could not create window...exiting\n");
    exit(1);
  }
  return win_id;
}

void shutdownDisplay(gDisplay *d) {
  deleteTextures();
  deleteFonts();
  glutDestroyWindow(d->win_id);
  printf("window destroyed\n");
}

static int g_pending_display_apply = 0;
static int g_just_applied_display_change = 0;
static int g_apply_cooldown_frames = 0;

void requestDisplayApply() {
  g_pending_display_apply = 1;
}

void forceViewportResetIfNeededForGui() {
  if (!g_just_applied_display_change) return;
  int w = glutGet(GLUT_WINDOW_WIDTH);
  int h = glutGet(GLUT_WINDOW_HEIGHT);
  if (w > 0 && h > 0) {
    glViewport(0, 0, w, h);
    guiProjection(w, h);
    printf("GUI viewport/projection reset to %dx%d after display change.\n", w, h);
  }
  g_just_applied_display_change = 0;
}

void forceViewportResetIfNeededForGame() {
  if (!g_just_applied_display_change) return;
  int w = glutGet(GLUT_WINDOW_WIDTH);
  int h = glutGet(GLUT_WINDOW_HEIGHT);
  if (w > 0 && h > 0) {
    glViewport(0, 0, w, h);
    initGameScreen();
    changeDisplay();
    printf("Game viewport reset to %dx%d after display change.\n", w, h);
  }
  g_just_applied_display_change = 0;
}

void applyDisplaySettingsDeferred() {
  if (!g_pending_display_apply) return;
  g_pending_display_apply = 0;

  if (!game || !game->settings) return;
  /* Ensure we have a window; if not, create one */
  if (!game->screen) {
    game->screen = (gDisplay*) malloc(sizeof(gDisplay));
  }
  if (game->screen->win_id <= 0) {
    setupDisplay(game->screen);
  } else {
    /* Make current */
    glutSetWindow(game->screen->win_id);
  }

  /* Apply windowed/fullscreen and resolution safely */
  if (game->settings->fullscreen) {
    /* Ensure we are current and then enter fullscreen */
    if (glutGetWindow() <= 0) {
      setupDisplay(game->screen);
      glutSetWindow(game->screen->win_id);
    }
    glutFullScreen();
    /* Query actual window size and update settings */
    int w = glutGet(GLUT_WINDOW_WIDTH);
    int h = glutGet(GLUT_WINDOW_HEIGHT);
    if (w > 0 && h > 0) {
      game->settings->width = w;
      game->settings->height = h;
    }
    /* Ensure screen matches settings */
    if (game->screen) {
      game->screen->w = game->settings->width;
      game->screen->h = game->settings->height;
    }
    /* Reinitialize screen and viewports after entering fullscreen with updated size */
    initGameScreen();
    changeDisplay();
    updateCallbacks();
    printf("Applied fullscreen via deferred apply. Window size: %dx%d.\n", w, h);
    /* Persist new size for next start */
    saveSettings();
  } else {
    /* Windowed: rebuild to requested resolution */
    printf("Applying windowed mode %dx%d via deferred apply.\n", game->settings->width, game->settings->height);
    shutdownDisplay(game->screen);
    setupDisplay(game->screen);
    initGameScreen();
    changeDisplay();
    updateCallbacks();
  }
  /* Mark to force viewport/projection reset on next frame and skip one draw */
  g_just_applied_display_change = 1;
  g_apply_cooldown_frames = 1;
}

void onReshape(int w, int h) {
  if (!game || !game->settings || !game->screen) return;
  if (game->screen->win_id > 0) glutSetWindow(game->screen->win_id);
  game->settings->width = w;
  game->settings->height = h;
  game->screen->w = w;
  game->screen->h = h;
  initGameScreen();
  changeDisplay();
  glViewport(0, 0, w, h);
  printf("onReshape applied: %dx%d.\n", w, h);
}

void setupDisplay(gDisplay *d) {
  printf("trying to create window\n");
  d->win_id = initWindow();
  printf("window created\n");
  /* printf("win_id is %d\n", d->win_id); */

  printf("loading fonts...\n");
  initFonts();
  printf("loading textures...\n");
  initTexture(game->screen);

}

int main( int argc, char *argv[] ) {
  char *path;

#ifdef __FreeBSD__
  fpsetmask(0);
#endif

  glutInit(&argc, argv);

  path = getFullPath("settings.txt");
  if(path != 0)
    initMainGameSettings(path); /* reads defaults from ~/.gltronrc */
  else {
    printf("fatal: could not settings.txt, exiting...\n");
    exit(1);
  }

  parse_args(argc, argv);

  /* sound */

#ifdef SOUND
  printf("initializing sound\n");
  initSound();
  path = getFullPath("gltron.it");
  if(path == 0 || loadSound(path)) 
    printf("error trying to load sound\n");
  else {
    if(game->settings->playSound) {
      playSound();
      free(path);
    }
  }
#endif

  printf("loading menu\n");
  path = getFullPath("menu.txt");
  if(path != 0)
    pMenuList = loadMenuFile(path);
  else {
    printf("fatal: could not load menu.txt, exiting...\n");
    exit(1);
  }
  printf("menu loaded\n");
  free(path);

  initGameStructures();
  resetScores();

  initData();

  setupDisplay(game->screen);
  switchCallbacks(&guiCallbacks);
  switchCallbacks(&guiCallbacks);

  glutMainLoop();

  return 0;
}

callbacks gameCallbacks = { 
  displayGame,
  idleGame,
  keyGame,
  specialGame,
  initGame,
  initGLGame
};






