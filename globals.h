#ifndef GLOBALS_H
#define GLOBALS_H

// Include necessary headers
#include "model.h"
#include "fonttex.h"
#include "menu.h"
#include "sgi_texture.h"
#include "gltron.h"

// Forward declarations for types used in globals
typedef struct Game Game;
typedef struct gDisplay gDisplay;
typedef struct Player Player;
typedef struct settings_int settings_int;
typedef struct settings_float settings_float;

// Declare global variables as extern
extern Game main_game;
extern Game *game;  // game is a pointer to Game structure
extern fonttex *ftx;
extern int fontID;
extern Menu** pMenuList;
extern Menu* pRootMenu;
extern float camAngle;
extern unsigned char* colmap;
extern int colwidth;
extern int dirsX[];
extern int dirsY[];
extern int lasttime;
extern double dt;
extern settings_int *si;
extern int si_count;
extern settings_float *sf;
extern int sf_count;
extern int polycount;
extern float colors_alpha[][4];
extern float colors_trail[][4];
extern float colors_model[][4];
extern int vp_max[];
extern float vp_x[3][4];
extern float vp_y[3][4];
extern float vp_w[3][4];
extern float vp_h[3][4];
extern char *help[];

#endif
