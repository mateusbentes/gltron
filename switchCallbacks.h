#ifndef SWITCHCALLBACKS_H
#define SWITCHCALLBACKS_H

#include "gltron.h"

/* Global callback pointers */
extern callbacks *last_callback;
extern callbacks *current_callback;

/* Android-prefixed internal implementations */
void android_switchCallbacks(callbacks *new);
void android_updateCallbacks(void);
void android_restoreCallbacks(void);

/* Public API expected by the rest of the codebase (non-Android names) */
void switchCallbacks(callbacks *new);
void updateCallbacks(void);
void restoreCallbacks(void);
void chooseCallback(char *name);

/* Utility hit-tests and overlay drawing used by Android touch GUI */
int hit_test_back_in_viewport(int x_win, int y_win);
int hit_test_menu_item_in_viewport(int x_win, int y_win);
void draw_android_overlay(void);
int hit_btn(int x, int y, int* btn);

#endif // SWITCHCALLBACKS_H
