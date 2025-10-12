#include "gltron.h"
#include "geom.h"
#include <string.h>
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include <GL/gl.h>
#include <GL/glu.h>

#ifndef GL_TEXTURE_WIDTH
#define GL_TEXTURE_WIDTH 0x1000
#endif

#ifndef GL_TEXTURE_HEIGHT
#define GL_TEXTURE_HEIGHT 0x1001
#endif

// Define LINE_D constant
#ifndef LINE_D
#define LINE_D 0.05
#endif

// Define constants for Android
#ifndef GL_LIGHTING
#define GL_LIGHTING 0x0B50
#endif

#ifndef GL_SMOOTH
#define GL_SMOOTH 0x1D01
#endif

#ifndef GL_FLAT
#define GL_FLAT 0x1D00
#endif

#ifndef GL_QUADS
#define GL_QUADS 0x0007
#endif

#ifndef GL_MODELVIEW_MATRIX
#define GL_MODELVIEW_MATRIX 0x0BA6
#endif

#ifndef GL_PROJECTION
#define GL_PROJECTION 0x1701
#endif

#ifndef GL_MODELVIEW
#define GL_MODELVIEW 0x1700
#endif

#ifndef GL_FOG
#define GL_FOG 0x0B60
#endif

#ifndef GL_LIGHT0
#define GL_LIGHT0 0x4000
#endif

#ifndef GL_POSITION
#define GL_POSITION 0x1203
#endif

// Lighting parameters
#define AMBIENT_LIGHT_R 0.2f
#define AMBIENT_LIGHT_G 0.2f
#define AMBIENT_LIGHT_B 0.2f

#define LIGHT_COLOR_R 0.8f
#define LIGHT_COLOR_G 0.8f
#define LIGHT_COLOR_B 0.8f

#define LIGHT_POS_X 5.0f
#define LIGHT_POS_Y 5.0f
#define LIGHT_POS_Z 10.0f

// Define neigung constant
#ifndef neigung
#define neigung 25
#endif

void drawDebugTex(gDisplay *d) {
  int x = 100;
  int y = 100;

  rasonly(d);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  // For desktop OpenGL
  glColor4f(.0, 1.0, .0, 1.0);
  glRasterPos2i(x, y);
  glBitmap(colwidth * 8, GSIZE, 0, 0, 0, 0, colmap);
  glBegin(GL_LINE_LOOP);
  glVertex2i(x - 1, y - 1);
  glVertex2i(x + colwidth * 8, y - 1);
  glVertex2i(x + colwidth * 8, y + GSIZE);
  glVertex2i(x - 1, y + GSIZE);
  glEnd();
  polycount++;
}

void drawScore(Player *p, gDisplay *d) {
  char tmp[10]; /* hey, they won't reach such a score */

  sprintf(tmp, "%d", p->data->score);
  rasonly(d);

  // For desktop OpenGL
  glColor4f(1.0, 1.0, 0.2, 1.0);
  drawText(5, 5, 32, tmp);
}
  
void drawFloor(gDisplay *d) {
    int j, k, l, t;

    if(game->settings->show_floor_texture) {
        // Desktop textured floor
        if (!game || !game->screen || game->screen->texFloor == 0) {
            return;
        }

        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, game->screen->texFloor);
        
        // Verify texture is loaded
        GLint texWidth;
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &texWidth);
        
        if (texWidth == 0) {
            glDisable(GL_TEXTURE_2D);
            // Fall back to line floor
            game->settings->show_floor_texture = 0;
            drawFloor(d);
            game->settings->show_floor_texture = 1;
            return;
        }

        // Set texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glColor4f(1.0, 1.0, 1.0, 1.0);
        
        l = GSIZE / 4;
        t = 5;
        
        for(j = 0; j < GSIZE; j += l) {
            for(k = 0; k < GSIZE; k += l) {
                glBegin(GL_QUADS);
                glNormal3f(0.0f, 0.0f, 1.0f);
                glTexCoord2f(0.0f, 0.0f); glVertex3f(j, k, 0.0f);
                glTexCoord2f(t, 0.0f);    glVertex3f(j + l, k, 0.0f);
                glTexCoord2f(t, t);       glVertex3f(j + l, k + l, 0.0f);
                glTexCoord2f(0.0f, t);    glVertex3f(j, k + l, 0.0f);
                glEnd();
                polycount++;
            }
        }
        
        glDisable(GL_TEXTURE_2D);
        
    } else {
        // Line floor
        // Desktop line floor
        glColor3f(0.0, 0.0, 1.0);
        glBegin(GL_LINES);
        for(j = 0; j <= GSIZE; j += game->settings->line_spacing) {
            glVertex3f(0, j, 0);
            glVertex3f(GSIZE, j, 0);
            glVertex3f(j, 0, 0);
            glVertex3f(j, GSIZE, 0);
            polycount += 2;
        }
        glEnd();
    }
}

void drawTraces(Player *p, gDisplay *d, int instance) {
  line *line;
  float height;

  Data *data;
  data = p->data;
  height = data->trail_height;
  if(height > 0) {
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    // For desktop OpenGL
    glColor4fv(p->model->color_alpha);
    line = &(data->trails[0]);
    glBegin(GL_TRIANGLE_STRIP);
    glVertex3f(line->sx, line->sy, 0.0);
    glVertex3f(line->sx, line->sy, height);
    while(line != data->trail) {
      glVertex3f(line->ex, line->ey, 0.0);
      glVertex3f(line->ex, line->ey, height);
      line++;
      polycount++;
    }
    glVertex3f(line->ex, line->ey, 0.0);
    glVertex3f(line->ex, line->ey, height);
    polycount += 2;
    glEnd();

    if(game->settings->camType == 1) {
      //       glLineWidth(3);
      // glBegin(GL_LINES);
      glBegin(GL_QUADS);
      glVertex2f(data->trail->sx - LINE_D, data->trail->sy - LINE_D);
      glVertex2f(data->trail->sx + LINE_D, data->trail->sy + LINE_D);
      glVertex2f(data->trail->ex + LINE_D, data->trail->ey + LINE_D);
      glVertex2f(data->trail->ex - LINE_D, data->trail->ey - LINE_D);

      glEnd();
      // glLineWidth(1);
      polycount++;
    }

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  }
}

void drawCrash(float radius) {
#define CRASH_W 20
  // For desktop OpenGL
  glColor4f(1.0, 1.0, 1.0, 1.0);

  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

  glEnable(GL_CULL_FACE);

  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, game->screen->texWall);
  glBegin(GL_QUADS);
  glTexCoord2f(0.0, 0.0); glVertex3f(0.0, 0.0, 0.0);
  glTexCoord2f(0.0, 1.0); glVertex3f(0.0, 0.0, WALL_H);
  glTexCoord2f(0.0, 1.0); glVertex3f(GSIZE, 0.0, WALL_H);
  glTexCoord2f(0.0, 0.0); glVertex3f(GSIZE, 0.0, 0.0);

  glTexCoord2f(0.0, 1.0); glVertex3f(GSIZE, 0.0, 0.0);
  glTexCoord2f(1.0, 0.0); glVertex3f(GSIZE, 0.0, WALL_H);
  glTexCoord2f(0.0, 0.0); glVertex3f(GSIZE, GSIZE, WALL_H);
  glTexCoord2f(0.0, 1.0); glVertex3f(GSIZE, GSIZE, 0.0);

  glTexCoord2f(0.0, 1.0); glVertex3f(GSIZE, GSIZE, 0.0);
  glTexCoord2f(1.0, 0.0); glVertex3f(GSIZE, GSIZE, WALL_H);
  glTexCoord2f(0.0, 0.0); glVertex3f(0.0, GSIZE, WALL_H);
  glTexCoord2f(0.0, 1.0); glVertex3f(0.0, GSIZE, 0.0);

  glTexCoord2f(0.0, 1.0); glVertex3f(0.0, GSIZE, 0.0);
  glTexCoord2f(1.0, 0.0); glVertex3f(0.0, GSIZE, WALL_H);
  glTexCoord2f(0.0, 0.0); glVertex3f(0.0, 0.0, WALL_H);
  glTexCoord2f(0.0, 1.0); glVertex3f(0.0, 0.0, 0.0);

  glEnd();
  polycount += 4;

  glDisable(GL_TEXTURE_2D);

  glDisable(GL_CULL_FACE);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void drawCycle(Player *p) {
  float dirangles[] = { 180, 90, 0, 270 , 360, -90 };
  int time = 0;
  int last_dir;
  float dirangle;
  Mesh *cycle;

#define turn_length 500

  cycle = p->model->mesh;

  // Desktop OpenGL code remains unchanged
  glPushMatrix();
  glTranslatef(p->data->posx, p->data->posy, .0);

  if(game->settings->turn_cycle) {
    time = abs(p->data->turn_time - getElapsedTime());
    if(time < turn_length) {
      last_dir = p->data->last_dir;
      if(p->data->dir == 3 && last_dir == 2)
        last_dir = 4;
      if(p->data->dir == 2 && last_dir == 3)
        last_dir = 5;
      dirangle = ((turn_length - time) * dirangles[last_dir] +
                  time * dirangles[p->data->dir]) / turn_length;
    } else
      dirangle = dirangles[p->data->dir];
  } else {
    dirangle = dirangles[p->data->dir];
  }

  glRotatef(dirangle, 0, 0.0, 1.0);

  if(game->settings->show_crash_texture)
    if(p->data->exp_radius > 0 && p->data->exp_radius < EXP_RADIUS_MAX)
      drawCrash(p->data->exp_radius);

  if(game->settings->turn_cycle) {
    if(time < turn_length) {
      float axis = 1.0;
      if(p->data->dir < p->data->last_dir && p->data->last_dir != 3)
        axis = -1.0;
      else if((p->data->last_dir == 3 && p->data->dir == 2) ||
              (p->data->last_dir == 0 && p->data->dir == 3))
        axis = -1.0;
      glRotatef(neigung * sin(M_PI * time / turn_length),
                0.0, axis, 0.0);
    }
  }

  glTranslatef(-cycle->bbox[0] / 2, -cycle->bbox[1] / 2, .0);

  // Enable lighting
  glEnable(GL_LIGHTING);
  glEnable(GL_DEPTH_TEST);
  glDepthMask(GL_TRUE);

  if(p->data->exp_radius == 0)
    drawModel(cycle, MODEL_USE_MATERIAL, 0);
  else if(p->data->exp_radius < EXP_RADIUS_MAX) {
    float alpha;
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    alpha = (float)(EXP_RADIUS_MAX - p->data->exp_radius) / (float)EXP_RADIUS_MAX;
    setMaterialAlphas(cycle, alpha);
    drawExplosion(cycle, p->data->exp_radius, MODEL_USE_MATERIAL, 0);
  }

  if(game->settings->show_alpha == 0) glDisable(GL_BLEND);

  // Disable lighting
  glDisable(GL_LIGHTING);
  glDisable(GL_DEPTH_TEST);
  glDepthMask(GL_FALSE);

  glPopMatrix();
}

int playerVisible(Player *eye, Player *target) {
  float v1[3];
  float v2[3];
  float tmp[3];
  float s;
  float d;

  vsub(eye->camera->target, eye->camera->cam, v1);
  normalize(v1);
  tmp[0] = target->data->posx;
  tmp[1] = target->data->posy;
  tmp[2] = 0;
  vsub(tmp, eye->camera->cam, v2);
  normalize(v2);
  s = scalarprod(v1, v2);
  /* maybe that's not exactly correct, but I didn't notice anything */
  d = cos((game->settings->fov / 2) * 2 * M_PI / 360.0);
  /*
  printf("v1: %.2f %.2f %.2f\nv2: %.2f %.2f %.2f\ns: %.2f d: %.2f\n\n",
	 v1[0], v1[1], v1[2], v2[0], v2[1], v2[2],
	 s, d);
  */
  if(s < d)
    return 0;
  else return 1;
}

void drawPlayers(Player *p) {
  int i;
  int dir;
  float l = 5.0;
  float height;

  // For desktop OpenGL
  glShadeModel(GL_SMOOTH);
  glEnable(GL_BLEND);

  // Enable lighting
  /* no fixed-function lighting on GLES2 */

  for(i = 0; i < game->players; i++) {
    height = game->player[i].data->trail_height;
    if(height > 0) {
      glPushMatrix();
      glTranslatef(game->player[i].data->posx,
                   game->player[i].data->posy,
                   0);
      /* draw Quad */
      dir = game->player[i].data->dir;
      glColor3fv(game->player[i].model->color_model);
      glBegin(GL_QUADS);
      glVertex3f(0, 0, 0);
      glColor4f(0, 0, 0, 0);
      glVertex3f(-dirsX[dir] * l, -dirsY[dir] * l, 0);
      glVertex3f(-dirsX[dir] * l, -dirsY[dir] * l, height);
      glColor3fv(game->player[i].model->color_model);
      glVertex3f(0, 0, height);
      glEnd();
      polycount++;
      glPopMatrix();
    }
    if(playerVisible(p, &(game->player[i]))) {
      if(game->settings->show_model)
        drawCycle(&(game->player[i]));
    }
  }

  // Disable lighting
  /* no fixed-function lighting on GLES2 */

  if(game->settings->show_alpha != 1) glDisable(GL_BLEND);
  glShadeModel(GL_FLAT);
}

void drawGlow(Player *p, gDisplay *d, float dim) {
  // For desktop OpenGL
  float mat[4*4];

  glPushMatrix();
  glTranslatef(p->data->posx,
               p->data->posy,
               0);

  glBlendFunc(GL_ONE, GL_ONE);
  glEnable(GL_BLEND);
  glGetFloatv(GL_MODELVIEW_MATRIX, mat);
  mat[0] = mat[5] = mat[10] = 1.0;
  mat[1] = mat[2] = 0.0;
  mat[4] = mat[6] = 0.0;
  mat[8] = mat[9] = 0.0;
  glLoadMatrixf(mat);
  glBegin(GL_TRIANGLE_FAN);
  glColor3fv(p->model->color_model);

  glVertex3f(0,TRAIL_HEIGHT/2, 0);
  glColor4f(0,0,0,0.0);
  glVertex3f(dim*cos(-0.2*3.1415/5.0),
             TRAIL_HEIGHT/2+dim*sin(-0.2*3.1415/5.0), 0);
  glVertex3f(dim*cos(1.0*3.1415/5.0),
             TRAIL_HEIGHT/2+dim*sin(1.0*3.1415/5.0), 0);
  glVertex3f(dim*cos(2.0*3.1415/5.0),
             TRAIL_HEIGHT/2+dim*sin(2.0*3.1415/5.0), 0);
  glVertex3f(dim*cos(3.0*3.1415/5.0),
             TRAIL_HEIGHT/2+dim*sin(3.0*3.1415/5.0), 0);
  glVertex3f(dim*cos(4.0*3.1415/5.0),
             TRAIL_HEIGHT/2+dim*sin(4.0*3.1415/5.0), 0);
  glVertex3f(dim*cos(5.2*3.1415/5.0),
             TRAIL_HEIGHT/2+dim*sin(5.2*3.1415/5.0), 0);
  glEnd();
  polycount += 5;

  glBegin(GL_TRIANGLES);
  glColor3fv(p->model->color_model);
  glVertex3f(0,TRAIL_HEIGHT/2, 0);
  glColor4f(0,0,0,0.0);
  glVertex3f(0,-TRAIL_HEIGHT/4,0);
  glVertex3f(dim*cos(-0.2*3.1415/5.0),
             TRAIL_HEIGHT/2+dim*sin(-0.2*3.1415/5.0), 0);

  glColor3fv(p->model->color_model);
  glVertex3f(0,TRAIL_HEIGHT/2, 0);
  glColor4f(0,0,0,0.0);
  glVertex3f(dim*cos(5.2*3.1415/5.0),
             TRAIL_HEIGHT/2+dim*sin(5.2*3.1415/5.0), 0);
  glVertex3f(0,-TRAIL_HEIGHT/4,0);
  glEnd();
  polycount += 3;

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  if(game->settings->show_alpha != 1) glDisable(GL_BLEND);
  glPopMatrix();
}

void drawWalls(gDisplay *d) {
  float t = 4;  // Texture repeat factor

  // For desktop OpenGL
  glColor4f(1.0, 1.0, 1.0, 1.0);

  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

  glEnable(GL_CULL_FACE);

  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, game->screen->texWall);
  glBegin(GL_QUADS);
  glTexCoord2f(0.0, 0.0); glVertex3f(0.0, 0.0, 0.0);
  glTexCoord2f(0.0, 1.0); glVertex3f(0.0, 0.0, WALL_H);
  glTexCoord2f(0.0, 1.0); glVertex3f(GSIZE, 0.0, WALL_H);
  glTexCoord2f(0.0, 0.0); glVertex3f(GSIZE, 0.0, 0.0);

  glTexCoord2f(0.0, 1.0); glVertex3f(GSIZE, 0.0, 0.0);
  glTexCoord2f(1.0, 0.0); glVertex3f(GSIZE, 0.0, WALL_H);
  glTexCoord2f(0.0, 0.0); glVertex3f(GSIZE, GSIZE, WALL_H);
  glTexCoord2f(0.0, 1.0); glVertex3f(GSIZE, GSIZE, 0.0);

  glTexCoord2f(0.0, 1.0); glVertex3f(GSIZE, GSIZE, 0.0);
  glTexCoord2f(1.0, 0.0); glVertex3f(GSIZE, GSIZE, WALL_H);
  glTexCoord2f(0.0, 0.0); glVertex3f(0.0, GSIZE, WALL_H);
  glTexCoord2f(0.0, 1.0); glVertex3f(0.0, GSIZE, 0.0);

  glTexCoord2f(0.0, 1.0); glVertex3f(0.0, GSIZE, 0.0);
  glTexCoord2f(1.0, 0.0); glVertex3f(0.0, GSIZE, WALL_H);
  glTexCoord2f(0.0, 0.0); glVertex3f(0.0, 0.0, WALL_H);
  glTexCoord2f(0.0, 1.0); glVertex3f(0.0, 0.0, 0.0);

  glEnd();
  polycount += 4;

  glDisable(GL_TEXTURE_2D);

  glDisable(GL_CULL_FACE);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

/*
void drawHelp(gDisplay *d) {
  rasonly(d);
  glColor4f(0.2, 0.2, 0.2, 0.8);
  glEnable(GL_BLEND);
  glBegin(GL_QUADS);
  glVertex2i(0,0);
  glVertex2i(d->vp_w - 1, 0);
  glVertex2i(d->vp_w - 1, d->vp_h - 1);
  glVertex2i(0, d->vp_h - 1);
  glEnd();
  if(game->settings->show_alpha != 1) glDisable(GL_BLEND);
  glColor3f(1.0, 1.0, 0.0);
  drawLines(d->vp_w, d->vp_h,
	    help, HELP_LINES, 0);
}
*/

void drawCam(Player *p, gDisplay *d) {
  int i;

  if (d->fog == 1) glEnable(GL_FOG);

  // For desktop OpenGL
  glColor3f(0.0, 1.0, 0.0);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(game->settings->fov, (float)d->vp_w / (float)d->vp_h, 3.0, GSIZE);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  // Camera parameters
  float camDist   = 12.0f; // distance behind the player
  float camHeight = 6.0f;  // height above the ground

  // Player position
  float playerX = p->data->posx;
  float playerY = p->data->posy;

  // Player facing direction vector (unit vector from dirsX/dirsY)
  float dirX = dirsX[p->data->dir];
  float dirY = dirsY[p->data->dir];

  // Camera position = player position - direction * distance + height in Z
  float camX = playerX - dirX * camDist;
  float camY = playerY - dirY * camDist;
  float camZ = camHeight;

  // Look-at target = in front of the player
  float lookX = playerX + dirX * 5.0f; // 5 units ahead
  float lookY = playerY + dirY * 5.0f;
  float lookZ = 0.5f; // just above ground

  // Up vector (Z-up in GLTron)
  float upX = 0.0f, upY = 0.0f, upZ = 1.0f;

  gluLookAt(camX, camY, camZ, lookX, lookY, lookZ, upX, upY, upZ);

  // Light moves with camera
  glLightfv(GL_LIGHT0, GL_POSITION, p->camera->cam);

  // Draw scene
  drawFloor(d);
  if (game->settings->show_wall == 1)
    drawWalls(d);

  for (i = 0; i < game->players; i++)
    drawTraces(&(game->player[i]), d, i);

  drawPlayers(p);

  if (game->settings->show_glow == 1)
    for (i = 0; i < game->players; i++)
      if ((p != &(game->player[i])) && (game->player[i].data->speed > 0))
        drawGlow(&(game->player[i]), d, TRAIL_HEIGHT * 4);

  glDisable(GL_FOG);
}

void drawAI(gDisplay *d) {
  char ai[] = "computer player";

  rasonly(d);

  // For desktop OpenGL
  glColor3f(1.0, 1.0, 1.0);
  drawText(d->vp_w / 4, 10, d->vp_w / (2 * strlen(ai)), ai);
}

void drawPause(gDisplay *display) {
  char pause[] = "Game is paused";
  char winner[] = "Player %d wins";
  char buf[100];
  char *message;
  static float d = 0;
  static float lt = 0;
  float delta;
  long now;

  now = getElapsedTime();
  delta = now - lt;
  lt = now;
  delta /= 500.0;
  d += delta;

  if(d > 2 * M_PI) {
    d -= 2 * M_PI;
  }

  if(game->pauseflag & PAUSE_GAME_FINISHED &&
     game->winner != -1) {
    message = buf;
    sprintf(message, winner, game->winner + 1);
  } else {
    message = pause;
  }

  rasonly(game->screen);

  // For desktop OpenGL
  glColor3f(1.0, (sin(d) + 1) / 2, (sin(d) + 1) / 2);
  drawText(display->vp_w / 6, 20,
           display->vp_w / (6.0 / 4.0 * strlen(message)), message);

  // Show hint for touch/mouse
  if (game->settings->input_mode != 0) {
    const char* hint = "Tap to resume";
    glColor3f(1.0, 1.0, 1.0);
    drawText(display->vp_w / 6, 20 + display->vp_h / 12,
             display->vp_w / (8.0 * strlen(hint)), hint);
  }
}
