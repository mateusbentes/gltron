#include "base/nebu_system.h"
#include "base/nebu_callbacks.h"
#include "input/nebu_input_system.h"
#include "video/nebu_video_system.h"
#include "base/nebu_util.h"

#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>

#include "base/nebu_debug_memory.h"

/* platform specific code to obtain current time in millis */
#include <sys/time.h>
static int getElapsedTime(void) {
  struct timeval tp;
  static int last = 0;
  int now;
  gettimeofday(&tp, NULL);
  now = (int)(tp.tv_sec * 1000 + tp.tv_usec / 1000);
  if(last == 0)
    last = now;
  int dt = now - last;
  last = now;
  return dt;
}

static int return_code = -1;
static int redisplay = 0;
static int idle = 1;
static int fps_last = 0;
static int fps_dt = 0;
static int fps_frames = 0;

static unsigned int lastFrame = 0;
static unsigned int dt = 0;
static unsigned int isSinceLastFrame = 0;

/* Pointer to the current callback structure */
Callbacks *current = 0;

void nebu_System_SetCallbacks(Callbacks *cb) {
  if (!cb) {
    fprintf(stderr, "[error] nebu_System_SetCallbacks: NULL callbacks provided\n");
    return;
  }

  current = cb;

  /* Print debug info about the callbacks being set */
  fprintf(stderr, "[debug] nebu_System_SetCallbacks: setting callbacks '%s'\n",
          current->name ? current->name : "unnamed");
}

void nebu_System_Exit() {
  SDL_Quit();
  exit(0);
}

void nebu_System_ExitLoop(int value) {
  idle = 0;
  return_code = value;
}

void nebu_System_PostRedisplay() {
  redisplay = 1;
}

void nebu_Init(void) {
  /* video initialized in Window.c */
  /* initialize SDL timer */
  if(SDL_Init(SDL_INIT_TIMER) < 0) {
    fprintf(stderr, "Couldn't initialize SDL timer: %s\n", SDL_GetError());
    exit(1); /* OK: critical, no visual */
  }
  /* setup keyboard */
  nebu_Input_Init();
}

void nebu_Time_SetCurrentFrameTime(unsigned t) {
  dt = t - lastFrame;
  lastFrame = t;
  isSinceLastFrame = 0;
}

unsigned int nebu_Time_GetElapsed() {
  return lastFrame;
}

unsigned int nebu_Time_GetElapsedSinceLastFrame() {
  return isSinceLastFrame;
}

int nebu_Time_GetTimeForLastFrame() {
  return dt;
}

void nebu_Time_FrameDelay(unsigned int ms) {
  unsigned int start, end;
  start = SDL_GetTicks();
  do {
    SDL_Delay(1);
    end = SDL_GetTicks();
    isSinceLastFrame = end - start;
  } while(end - start < ms);
}

void nebu_System_Sleep(int ms) {
  SDL_Delay(ms);
}
