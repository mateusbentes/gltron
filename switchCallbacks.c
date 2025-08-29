#include "gltron.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

callbacks *last_callback = 0;
callbacks *current_callback = 0;

void switchCallbacks(callbacks *new) {
  last_callback = current_callback;
  current_callback = new;

#ifndef ANDROID
  glutIdleFunc(new->idle);
  glutDisplayFunc(new->display);
  glutKeyboardFunc(new->keyboard);
  glutSpecialFunc(new->special);
  /* register mouse handlers depending on mode */
  /* always register reshape */
  glutReshapeFunc(onReshape);
  if (new == &guiCallbacks) {
    glutMouseFunc(mouseGui);
    glutMotionFunc(motionGui);
    glutPassiveMotionFunc(motionGui);
  } else if (new == &gameCallbacks) {
    glutMouseFunc(mouseGame);
    glutMotionFunc(motionGame);
    glutPassiveMotionFunc(motionGame);
  } else if (new == &pauseCallbacks) {
    glutMouseFunc(mousePause);
    glutMotionFunc(motionPause);
    glutPassiveMotionFunc(motionPause);
  }
#endif
  lasttime = getElapsedTime();

  /* printf("callbacks registred\n"); */
  (new->init)();
  (new->initGL)();
  /* printf("callback init's completed\n"); */
}
  
void updateCallbacks() {
  /* called when the window is recreated */
#ifndef ANDROID
  glutIdleFunc(current_callback->idle);
  glutDisplayFunc(current_callback->display);
  glutKeyboardFunc(current_callback->keyboard);
  glutSpecialFunc(current_callback->special);
  if (current_callback == &guiCallbacks) {
    glutMouseFunc(mouseGui);
    glutMotionFunc(motionGui);
    glutPassiveMotionFunc(motionGui);
  } else if (current_callback == &gameCallbacks) {
    glutMouseFunc(mouseGame);
    glutMotionFunc(motionGame);
    glutPassiveMotionFunc(motionGame);
  } else if (current_callback == &pauseCallbacks) {
    glutMouseFunc(mousePause);
    glutMotionFunc(motionPause);
    glutPassiveMotionFunc(motionPause);
  }
#endif
  lasttime = getElapsedTime();

  fprintf(stderr, "restoring callbacks\n");
}

void restoreCallbacks() {
  if(last_callback == 0) {
    fprintf(stderr, "no last callback present, exiting\n");
    exit(1);
  }
  switchCallbacks(last_callback);
}

void chooseCallback(char *name) {
  /* maintain a table of names of callbacks */
  /* lets hardcode the names for all known modes in here */

/* TODO(3): incorporate model stuff */
  /*
  if(strcmp(name, "chooseModel") ==  0) {
    fprintf(stderr, "change callbacks to chooseModel\n");
    switchCallbacks(&chooseModelCallbacks);
  }
  */
  if(strcmp(name, "gui") == 0) {
    /* fprintf(stderr, "change callbacks to gui\n"); */
    switchCallbacks(&guiCallbacks);
  }
}
