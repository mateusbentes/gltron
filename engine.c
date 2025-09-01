#include <math.h>
#include <string.h>
#include "gltron.h"
#include "globals.h"
#include "shaders.h"
#include <math.h>
#define M_PI 3.14159265358979323846

#ifdef ANDROID
#include <GLES/gl.h>
#include <GLES/glext.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

void setCol(int x, int y) {
  int offset, mask;
  if(x < 0 || x > GSIZE - 1 || y < 0 || y > GSIZE - 1) {
    printf("setCol: %d %d is out of range!\n", x, y);
    return;
  }
  offset = x / 8 + y * colwidth;
  mask = 128 >> (x % 8);
  *(colmap + offset) |= mask;
}

void clearCol(int x, int y) {
  int offset, mask;
  if(x < 0 || x > GSIZE - 1 || y < 0 || y > GSIZE - 1) {
    printf("clearCol: %d %d is out of range!\n", x, y);
    return;
  }
  offset = x / 8 + y * colwidth;
  mask = 128 >> (x % 8);
  *(colmap + offset) &= !mask;
}

int getCol(int x, int y) {
  int offset, mask;
  if(x < 0 || x > GSIZE - 1 || y < 0 || y > GSIZE - 1)
    return -1;
  offset = x / 8 + y * colwidth;
  mask = 128 >> (x % 8);
  return *(colmap + offset) & mask;
}

void turn(Data* data, int direction) {
  line *new;

  if(data->speed > 0) { /* only allow turning when in-game */
    data->trail->ex = data->posx;
    data->trail->ey = data->posy;

    /* smooth turning */
    data->last_dir = data->dir;
    data->turn_time = getElapsedTime();

    data->dir = (data->dir + direction) % 4;

    new = (data->trail) + 1;
    new->ex = new->sx = data->trail->ex;
    new->ey = new->sy = data->trail->ey;

    data->trail = new;
  }
}


void initDisplay(gDisplay *d, int type, int p, int onScreen) {
  int field;
  field = game->screen->vp_w / 32;
  d->h = game->screen->h;
  d->w = game->screen->w;
  d->vp_x = vp_x[type][p] * field;
  d->vp_y = vp_y[type][p] * field;
  d->vp_w = vp_w[type][p] * field;
  d->vp_h = vp_h[type][p] * field;
  d->blending = 1;
  d->fog = 0;
  d->wall = 1;
  d->onScreen = onScreen;
}  

void changeDisplay() {
  int i;
  for(i = 0; i < game->players; i++)
    game->player[i].display->onScreen = 0;
  for(i = 0; i < vp_max[game->settings->display_type]; i++)
       initDisplay(game->player[ game->settings->content[i] ].display, 
		   game->settings->display_type, i, 1);
}

void initGame() { /* called when game mode is entered */
}

void initGameStructures() { /* called only once */
  /* init game screen */
  /* init players. for each player: */
  /*   init model */
  /*   init display */
  /*   init ai */
  /*   create data */
  /*   create camera */

  gDisplay *d;
  int i, j;
  /* int onScreen; */
  /* Data *data; */
  /* Camera *c; */
  /* Model *m; */
  AI *ai;
  Player *p;
  char *path;

  game->winner = -1;
  game->screen = (gDisplay*) malloc(sizeof(gDisplay));
  d = game->screen;
  d->h = game->settings->height; d->w = game->settings->width;
  d->vp_x = 0; d->vp_y = 0;
  d->vp_w = d->w; d->vp_h = d->h;
  d->blending = 1;
  d->fog = 0;
  d->wall = 1;
  d->onScreen = -1;

  game->players = PLAYERS;
  for(i = 0; i < game->players; i++) {
    p = &(game->player[i]);
    p->model = (Model*) malloc(sizeof(Model));
    p->display = (gDisplay*) malloc(sizeof(gDisplay));
    p->ai = (AI*) malloc(sizeof(AI));
    p->data = (Data*) malloc(sizeof(Data));
    p->camera = (Camera*) malloc(sizeof(Camera));

    // init model & display & ai

    // load player mesh, currently only one type
    path = getFullPath("t-u-low.obj");
    // path = getFullPath("tron-med.obj");
    if(path != 0)
      // model size == CYCLE_HEIGHT
      p->model->mesh = loadModel(path, CYCLE_HEIGHT, 1);
    else {
      printf("fatal: could not load model - exiting...\n");
      exit(1);
    }
    
    free(path);

    /* copy contents from colors_a[] to model struct */
    for(j = 0; j < 4; j++) {
      p->model->color_alpha[j] = colors_alpha[i][j];
      p->model->color_trail[j] = colors_trail[i][j];
      p->model->color_model[j] = colors_model[i][j];
    }
    // set material 0 to color_model
    setMaterialAmbient(p->model->mesh, 0, p->model->color_model);
    setMaterialDiffuse(p->model->mesh, 0, p->model->color_model);

    ai = p->ai;
    ai->active = (i == 0 && game->settings->screenSaver == 0) ? -1 : 1;
    ai->tdiff = 0;
    ai->moves = 0;
    ai->danger = 0;
  }

  changeDisplay();
  initData();
}

void initData() {
  /* for each player */
  /*   init camera (if any) */
  /*   init data */
  /*   reset ai (if any) */
  int i;
  Camera *cam;
  Data *data;
  AI *ai;
  Model *model;

  for(i = 0; i < game->players; i++) {
    data = game->player[i].data;
    cam = game->player[i].camera;
    ai = game->player[i].ai;
    model = game->player[i].model;

    setMaterialAlphas(model->mesh, 1.0);

#ifdef ANDROID
    // Force chase cam (Mike-cam) for Android for better mobile experience
    cam->camType = 1;
#else
    cam->camType = game->settings->camType;
#endif
    cam->target[0] = data->posx;
    cam->target[1] = data->posy;
    cam->target[2] = 0;

    cam->cam[0] = data->posx + CAM_CIRCLE_DIST;
    cam->cam[1] = data->posy;
    cam->cam[2] = CAM_CIRCLE_Z;

    data->posx = GSIZE / 2 + GSIZE / 4 *
      cos ( (float) (i * 2 * M_PI) / (float) game->players );
    data->posy = GSIZE / 2 + GSIZE / 4 * 
      sin ( (float) (i * 2 * M_PI) / (float) game->players );

    data->dir = rand() & 3;
    data->last_dir = data->dir;
    data->turn_time = 0;

    data->speed = game->settings->speed;
    data->trail_height = TRAIL_HEIGHT;
    data->trail = data->trails;
    data->exp_radius = 0;

    data->trail->sx = data->trail->ex = data->posx;
    data->trail->sy = data->trail->ey = data->posy;

    ai->tdiff = 0;
    ai->moves = 0;
    ai->danger = 0;
  }

  game->running = game->players; /* everyone is alive */
  game->winner = -1;
  /* colmap */
  colwidth = (GSIZE + 7) / 8;
  if(colmap == NULL) colmap = (unsigned char*) malloc(colwidth * GSIZE);
  for(i = 0; i < colwidth * GSIZE; i++)
    *(colmap + i) = 0;

  lasttime = getElapsedTime();
#ifdef ANDROID
  // Start unpaused on Android for immediate gameplay
  game->pauseflag = 0;
#else
  game->pauseflag = 0;
#endif
}


int colldetect(float sx, float sy, float ex, float ey, int dir, int *x, int *y) {
  if(getenv("TRON_NO_COLL")) return 0;
  *x = (int) sx;
  *y = (int) sy;

  while(*x != (int) ex || *y != (int) ey) {
    *x += dirsX[dir];
    *y += dirsY[dir];
    if(getCol(*x, *y)) {
      /* check if x/y are in bounds and correct it */
      if(*x < 0) *x = 0;
      if(*x >= GSIZE) *x = GSIZE -1; 
      if(*y < 0) *y = 0;
      if(*y >= GSIZE) *y = GSIZE -1; 
      return 1;
    }
  }
  return 0;
}

void doTrail(line *t, void(*mark)(int, int)) {	  
  int x, y, ex, ey, dx, dy;

  x = (t->sx < t->ex) ? t->sx : t->ex;
  y = (t->sy < t->ey) ? t->sy : t->ey;
  ex = (t->sx > t->ex) ? t->sx : t->ex;
  ey = (t->sy > t->ey) ? t->sy : t->ey;
  dx = (x == ex) ? 0 : 1;
  dy = (y == ey) ? 0 : 1;
  if(dx == 0 && dy == 0) {
    mark(x, y);
  } else 
    while(x <= ex && y <= ey) {
      mark(x, y);
      x += dx;
      y += dy;
    }
}

void fixTrails() {
  int i;
  Data *d;
  line *t;
  for(i = 0; i < game->players; i++) {
    d = game->player[i].data;
    if(d->speed > 0) {
      t = &(d->trails[0]);
      while(t != d->trail) {
	doTrail(t, setCol);
	t++;
      }
      doTrail(t, setCol);
    }
  }
}

void clearTrails(Data *data) {
  line *t = &(data->trails[0]);
  while(t != data->trail) {
    doTrail(t, clearCol);
    t++;
  }
  doTrail(t, clearCol);
}

void chaseCamMove() {
  int i;
  Camera *cam;
  Data *data;
  float dest[3];
  float dcamx;
  float dcamy;
  float d;

  for(i = 0; i < game->players; i++) {

    cam = game->player[i].camera;
    data = game->player[i].data;

    switch(cam->camType) {
    case 0: /* Andi-cam */
      cam->cam[0] = data->posx + CAM_CIRCLE_DIST * COS(camAngle);
      cam->cam[1] = data->posy + CAM_CIRCLE_DIST * SIN(camAngle);
      cam->cam[2] = CAM_CIRCLE_Z;
      cam->target[0] = data->posx;
      cam->target[1] = data->posy;
      cam->target[2] = B_HEIGHT;
      break;
    
    case 1: // Mike-cam (classic GLTron chase camera)
      // Look at the player's position
      cam->target[0] = data->posx + dirsX[data->dir] * 5.0f; // look ahead in the driving direction
      cam->target[1] = data->posy + dirsY[data->dir] * 5.0f;
      cam->target[2] = B_HEIGHT;

      // Desired camera position behind the player
      dest[0] = data->posx - CAM_FOLLOW_DIST * dirsX[data->dir];
      dest[1] = data->posy - CAM_FOLLOW_DIST * dirsY[data->dir];
      dest[2] = CAM_CIRCLE_Z;

      // Smooth interpolation toward desired position
      d = sqrtf((dest[0] - cam->cam[0]) * (dest[0] - cam->cam[0]) +
              (dest[1] - cam->cam[1]) * (dest[1] - cam->cam[1]));
      if (d != 0) {
        dcamx = (float)dt * CAM_FOLLOW_SPEED * (dest[0] - cam->cam[0]) / d;
        dcamy = (float)dt * CAM_FOLLOW_SPEED * (dest[1] - cam->cam[1]) / d;

        if ((dest[0] - cam->cam[0] > 0 && dest[0] - cam->cam[0] < dcamx) ||
            (dest[0] - cam->cam[0] < 0 && dest[0] - cam->cam[0] > dcamx)) {
            cam->cam[0] = dest[0];
        } else cam->cam[0] += dcamx;

        if ((dest[1] - cam->cam[1] > 0 && dest[1] - cam->cam[1] < dcamy) ||
            (dest[1] - cam->cam[1] < 0 && dest[1] - cam->cam[1] > dcamy)) {
            cam->cam[1] = dest[1];
        } else cam->cam[1] += dcamy;
      }

      // Always keep the camera at fixed height
      cam->cam[2] = CAM_CIRCLE_Z;
      break;

    case 2: /* 1st person */
#define H 3
      cam->target[0] = data->posx + dirsX[data->dir];
      cam->target[1] = data->posy + dirsY[data->dir];
      cam->target[2] = H;

      cam->cam[0] = data->posx;
      cam->cam[1] = data->posy;
      cam->cam[2] = H + 0.5;
      break;
    }
  }
}

void idleGame( void ) {
  int i, j;
  int loop; 

  /* Apply any pending display changes right away in game loop */
  applyDisplaySettingsDeferred();
  /* If a change was just applied, force the viewport reset for game */
  forceViewportResetIfNeededForGame();

#ifdef SOUND
  soundIdle();
#endif

  if(game->settings->fast_finish == 1) {
    loop = FAST_FINISH;
    for(i = 0; i < game->players; i++)
      if(game->player[i].ai->active != 1 &&
	 game->player[i].data->exp_radius < EXP_RADIUS_MAX)
	 /* game->player[i].data->speed > 0) */
	loop = 1;
  } else loop = 1;

  // Ensure we don't have huge time jumps on first frame
  if(getElapsedTime() - lasttime > 1000) {
    lasttime = getElapsedTime() - 20; // Reset to reasonable delta
  }
  if(getElapsedTime() - lasttime < 10 && loop == 1) return;
  timediff();
  for(j = 0; j < loop; j++) {
    if(loop == FAST_FINISH)
      dt = 20;
    movePlayers();

    /* do AI */
    for(i = 0; i < game->players; i++)
      if(game->player[i].ai != NULL)
	if(game->player[i].ai->active == 1)
	  doComputer(&(game->player[i]), game->player[i].data);
  }

  /* chase-cam movement here */
  camMove();
  chaseCamMove();
#ifndef ANDROID
  glutPostRedisplay();
#else
  /* Android: frame rendering should be requested by the app's loop */
#endif
}

void defaultDisplay(int n) {
  game->settings->display_type = n;
  game->settings->content[0] = 0;
  game->settings->content[1] = 1;
  game->settings->content[2] = 2;
  game->settings->content[3] = 3;
  changeDisplay();
}

void initGameScreen() {
  gDisplay *d;
  d = game->screen;
  d->h = game->settings->height; d->w = game->settings->width;
  d->vp_x = 0; d->vp_y = 0;
  d->vp_w = d->w; d->vp_h = d->h;
}

void cycleDisplay(int p) {
  int q;
  q = (game->settings->content[p] + 1) % game->players;
  while(q != game->settings->content[p]) {
    if(game->player[q].display->onScreen == 0)
      game->settings->content[p] = q;
    else q = (q + 1) % game->players;
  }
  changeDisplay();
}

void resetScores() {
  int i;
  for(i = 0; i < game->players; i++)
    game->player[i].data->score = 0;
}

void movePlayers() {
  int i, j;
  float newx, newy;
  int x, y;
  int col;
  int winner;
  Data *data;

  /* do movement and collision */
  for(i = 0; i < game->players; i++) {
    data = game->player[i].data;
    if(data->speed > 0) { /* still alive */
      newx = data->posx + dt / 100 * data->speed * dirsX[data->dir];
      newy = data->posy + dt / 100 * data->speed * dirsY[data->dir];
      
      if((int)data->posx != newx || (int)data->posy != newy) {
	/* collision-test here */
	/* boundary-test here */
	col = colldetect(data->posx, data->posy, newx, newy,
			 data->dir, &x, &y);
	if (col) {
#ifdef SOUND
#ifdef ANDROID
	  sb_play_sample("game_crash.wav");
#else
	  playSampleEffect(crash_sfx);
#endif
#endif
	  /* set endpoint to collision coordinates */
	  newx = x;
	  newy = y;
	  /* update scores; */
	  if(game->settings->screenSaver != 1)
	  for(j = 0; j < game->players; j++) {
	    if(j != i && game->player[j].data->speed > 0)
	      game->player[j].data->score++;
	  }
	  data->speed = SPEED_CRASHED;
	}

	/* now draw marks in the bitfield */
	x = (int) data->posx;
	y = (int) data->posy;
	while(x != (int)newx ||
	      y != (int)newy ) {
	  x += dirsX[data->dir];
	  y += dirsY[data->dir];
	  setCol(x, y);
	}
	data->trail->ex = data->posx = newx;
	data->trail->ey = data->posy = newy;

	if(col && game->settings->erase_crashed == 1) {
	  clearTrails(data);
	  fixTrails(); /* clearTrails does too much... */
	}
      }
    } else { /* do trail countdown && explosion */
      if(data->exp_radius < EXP_RADIUS_MAX)
	data->exp_radius += (float)dt * EXP_RADIUS_DELTA;
      else if (data->speed == SPEED_CRASHED) {
	data->speed = SPEED_GONE;
	game->running--;
	if(game->running <= 1) { /* all dead, find survivor */
	  for(winner = 0; winner < game->players; winner++)
	    if(game->player[winner].data->speed > 0) break;
	  game->winner = (winner == game->players) ? -1 : winner;
	  printf("winner: %d\n", winner);
#ifdef ANDROID
	  extern callbacks pauseCallbacks;
	  android_switchCallbacks(&pauseCallbacks);
#else
	  switchCallbacks(&pauseCallbacks);
#endif
	  /* screenSaverCheck(0); */
	  game->pauseflag = PAUSE_GAME_FINISHED;
	}
      }
      if(game->settings->erase_crashed == 1 && data->trail_height > 0)
	data->trail_height -= (float)(dt * TRAIL_HEIGHT) / 1000;
    }
  }
}

void timediff() {
  int t;
  t = getElapsedTime();
  dt = t - lasttime;
  lasttime = t;
}

void camMove() {
#ifdef ANDROID
  // On Android, camera movement is handled by chaseCamMove()
  // This function is only for legacy compatibility
  return;
#else
  // Get the active player (assuming player 0 is the main player)
  Data *activePlayer = game->player[0].data;

  // Camera parameters
  float camHeight = 5.0f;   // Vertical distance from the player
  float camDist   = 10.0f;  // Distance behind the player

  // Player position
  float playerX = activePlayer->posx;
  float playerY = activePlayer->posy;

  // Get player direction unit vector from dirsX/Y
  float dirX = dirsX[activePlayer->dir];
  float dirY = dirsY[activePlayer->dir];

  // Place camera BEHIND the player
  float camX = playerX - camDist * dirX;
  float camY = playerY - camDist * dirY;
  float camZ = camHeight;

  // Look-at target = player position (centered at ground height)
  float lookX = playerX;
  float lookY = playerY;
  float lookZ = 0.0f;

  // Up vector
  float upX = 0.0f;
  float upY = 0.0f;
  float upZ = 1.0f;   // In GLTron, Z is up, not Y
#endif

#ifdef ANDROID
  // OpenGL ES 2.0 implementation
  GLfloat modelViewMatrix[16];

  // Initialize the model-view matrix
  memset(modelViewMatrix, 0, sizeof(modelViewMatrix));
  modelViewMatrix[0] = 1.0f; // Scale X
  modelViewMatrix[5] = 1.0f; // Scale Y
  modelViewMatrix[10] = 1.0f; // Scale Z
  modelViewMatrix[15] = 1.0f; // Translation W

  // Calculate the view matrix
  float forward[3] = {lookX - camX, lookY - camY, lookZ - camZ};
  float right[3];
  float up[3] = {upX, upY, upZ};

  // Normalize the forward vector
  float length = sqrtf(forward[0] * forward[0] + forward[1] * forward[1] + forward[2] * forward[2]);
  if (length > 0) {
    forward[0] /= length;
    forward[1] /= length;
    forward[2] /= length;
  }

  // Calculate the right vector
  right[0] = forward[1] * up[2] - forward[2] * up[1];
  right[1] = forward[2] * up[0] - forward[0] * up[2];
  right[2] = forward[0] * up[1] - forward[1] * up[0];

  // Normalize the right vector
  length = sqrtf(right[0] * right[0] + right[1] * right[1] + right[2] * right[2]);
  if (length > 0) {
    right[0] /= length;
    right[1] /= length;
    right[2] /= length;
  }

  // Calculate the up vector
  up[0] = right[1] * forward[2] - right[2] * forward[1];
  up[1] = right[2] * forward[0] - right[0] * forward[2];
  up[2] = right[0] * forward[1] - right[1] * forward[0];

  // Set the view matrix
  modelViewMatrix[0] = right[0];
  modelViewMatrix[1] = up[0];
  modelViewMatrix[2] = -forward[0];
  modelViewMatrix[4] = right[1];
  modelViewMatrix[5] = up[1];
  modelViewMatrix[6] = -forward[1];
  modelViewMatrix[8] = right[2];
  modelViewMatrix[9] = up[2];
  modelViewMatrix[10] = -forward[2];
  modelViewMatrix[12] = -(right[0] * camX + right[1] * camY + right[2] * camZ);
  modelViewMatrix[13] = -(up[0] * camX + up[1] * camY + up[2] * camZ);
  modelViewMatrix[14] = forward[0] * camX + forward[1] * camY + forward[2] * camZ;

  // Apply the model-view matrix via shader uniform (GLES2 path)
  GLuint sp = shader_get_basic();
  if (sp) {
    useShaderProgram(sp);
    setViewMatrix(sp, (float*)modelViewMatrix);
  }
#else
  // Standard OpenGL implementation
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  // Set up the camera position and orientation
  gluLookAt(camX, camY, camZ,  // Camera position
            lookX, lookY, lookZ,  // Look at point
            upX, upY, upZ);  // Up vector
#endif
}
