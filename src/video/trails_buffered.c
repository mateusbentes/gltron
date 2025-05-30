#include "video/video.h"
#include "game/game.h"
#include "video/nebu_quad.h"
#include "video/nebu_renderer_gl.h"
#include "video/trail_geometry.h"
#include "video/nebu_light.h"

/* Define Line type if not already defined */
typedef segment2 Line;

int getTrailCount() {
  int i;
  int c = 0;
  Line *l;
  for(i = 0; i < game->players; i++) {
    Data* data = &game->player[i].data;
    l = data->trails;
    fprintf(stderr, "trailcount: %d\n", data->nTrails + 1);
    while(l != data->trails + data->nTrails) {
      l++;
      c++;
    }
    c++;
  }
  fprintf(stderr, "trails: %d\n", c);
  return c;
}

void bufferPlayerBow(Player *p, QuadBuffer *qb) {
  Data *data;
  Quad *q;
  float height;
  float ex, ey, sx, sy;
  int bdist;
  float white[] = { 1.0, 1.0, 1.0, 1.0 };

  data = &p->data;
  height = data->trail_height;
  if(height <= 0) return;

  q = getNextQuad(qb);
  q->type = QUAD_COLOR;
  glShadeModel(GL_SMOOTH);

  if(PLAYER_IS_ACTIVE(p)) {
    q->type |= QUAD_TEXTURE | QUAD_TEX_MODULATE | QUAD_ALPHA_TEST;
    q->texture_id = gScreen->textures[TEX_TRAIL];
  }

  bdist = PLAYER_IS_ACTIVE(p) ? 2 : 3;

  sx = getSegmentEndX(data, 0);
  sy = getSegmentEndY(data, 0);

  ex = getSegmentEndX(data, bdist);
  ey = getSegmentEndY(data, bdist);

  /* hacked texture coordinates to avoid bleeding on some cards */
#define ONE 0.98
#define ZERO 0.02
  q_setColor4fv(q, 0, white);
  q_setTexCoord2f(q, 0, ZERO, ZERO);
  q_setVertex3f(q, 0, sx, sy, 0.0);

  q_setColor4fv(q, 1, white);
  q_setTexCoord2f(q, 1, ZERO, ONE);
  q_setVertex3f(q, 1, sx, sy, height);

  q_setColor4fv(q, 2, p->profile.pColorDiffuse);
  q_setTexCoord2f(q, 2, ONE, ONE);
  q_setVertex3f(q, 2, ex, ey, height);

  q_setColor4fv(q, 3, p->profile.pColorDiffuse);
  q_setTexCoord2f(q, 3, ONE, ZERO);
  q_setVertex3f(q, 3, ex, ey, 0.0);
}

void bufferPlayerTrail(Player *p, QuadBuffer *qb) {
  Line *ln;
  float height;
  float uv, ex, ey;
  float normal1[] = { 1.0, 0.0, 0.0 };
  float normal2[] = { 0.0, 1.0, 0.0 };
  float *normal;
  float color[4];
  float white[] = { 1.0, 1.0, 1.0, 1.0 };
  Data *data;
  Quad *q;
  int tex;

  data = &p->data;
  if(data->trail_height <= 0) return;

  tex = gScreen->textures[TEX_DECAL];

  height = data->trail_height;

  if(height < 0) return;

  /* calculate trail color and set blending modes */
  if(gSettingsCache.alpha_trails) {
    setColor4fv(p->profile.pColorAlpha);
  } else {
    setColor3fv(p->profile.pColorAlpha);
  }

  /* start drawing */
  ln = &(data->trails[0]);
  while(ln != data->trails + data->nTrails) {
    q = getNextQuad(qb);
    if (gSettingsCache.softwareRendering == 0 && 
        gSettingsCache.show_decals == 1) {
      q->type = QUAD_COLOR | QUAD_TEXTURE | QUAD_TEX_DECAL;
    } else {
      q->type = QUAD_COLOR;
    }
    q->texture_id = tex;
    if(ln->vDirection.v[1] == 0) {
      normal = normal1;
    } else {
      normal = normal2;
    }

    /* glNormal3fv(normal); */
    setNormal3fv(normal);
    setVertex3f((ln->vStart.v[0] + ln->vDirection.v[0]) / 2, 
                (ln->vStart.v[1] + ln->vDirection.v[1]) / 2, 0);
    light4fv(color);
    
    q_setColor4fv(q, 0, color);
    q_setTexCoord2f(q, 0, 0.0, 0.0);
    q_setVertex3f(q, 0, ln->vStart.v[0], ln->vStart.v[1], 0.0);

    uv = getSegmentUV(ln);

    q_setColor4fv(q, 1, color);
    q_setTexCoord2f(q, 1, uv, 0.0);
    q_setVertex3f(q, 1, ln->vStart.v[0] + ln->vDirection.v[0], 
                        ln->vStart.v[1] + ln->vDirection.v[1], 0.0);

    q_setColor4fv(q, 2, color);
    q_setTexCoord2f(q, 2, uv, 1.0);
    q_setVertex3f(q, 2, ln->vStart.v[0] + ln->vDirection.v[0], 
                        ln->vStart.v[1] + ln->vDirection.v[1], height);

    q_setColor4fv(q, 3, color);
    q_setTexCoord2f(q, 3, 0.0, 1.0);
    q_setVertex3f(q, 3, ln->vStart.v[0], ln->vStart.v[1], height);

    ln++;
  }

  if(ln->vDirection.v[1] == 0) normal = normal1;
  else normal = normal2;
  /* glNormal3fv(normal); */

  /* calculate segment color */
  setNormal3fv(normal);
  setVertex3f(ln->vStart.v[0], ln->vStart.v[1], 0);
  light4fv(color);

  q = getNextQuad(qb);
  if(gSettingsCache.softwareRendering == 0 && 
     gSettingsCache.show_decals == 1) 
    q->type = QUAD_COLOR | QUAD_TEXTURE | QUAD_TEX_DECAL;
  else q->type = QUAD_COLOR;

  q->texture_id = tex;

  q_setColor4fv(q, 0, color);
  q_setTexCoord2f(q, 0, 0.0, 0.0);
  q_setVertex3f(q, 0, ln->vStart.v[0], ln->vStart.v[1], 0.0);

  uv = getSegmentEndUV(ln, data);
  ex = getSegmentEndX(data, 1);
  ey = getSegmentEndY(data, 1);

  q_setColor4fv(q, 1, color);
  q_setTexCoord2f(q, 1, uv, 0.0);
  q_setVertex3f(q, 1, ex, ey, 0.0);

  /* uv = getSegmentUV(line); // wrong! */
  q_setColor4fv(q, 2, color);
  q_setTexCoord2f(q, 2, uv, 1.0);
  q_setVertex3f(q, 2, ex, ey, height);

  q_setColor4fv(q, 3, color);
  q_setTexCoord2f(q, 3, 0.0, 1.0);
  q_setVertex3f(q, 3, ln->vStart.v[0], ln->vStart.v[1], height);

  /* 
  printf("uv for last segment: %.3f\n");
  printf("segment lenght: %.2f\n", (ex - line->sx) + (ey - line->sy));
  */

  /* experimental trail effect */
  nebu_Video_CheckErrors("before trail");

  q = getNextQuad(qb);
  q->type = QUAD_COLOR;
    
  q_setColor4fv(q, 0, color);
  q_setVertex3f(q, 0, ex, ey, 0.0);

  q_setColor4fv(q, 3, color);
  q_setVertex3f(q, 3, ex, ey, height);

  uv = getSegmentUV(ln);
  ex = getSegmentEndX(data, 0);
  ey = getSegmentEndY(data, 0);

  memcpy(color, white, sizeof(color));

  q_setColor4fv(q, 2, color);
  q_setVertex3f(q, 2, ex, ey, height);

  q_setColor4fv(q, 1, color);
  q_setVertex3f(q, 1, ex, ey, 0.0);
}

void drawTrails(QuadBuffer *q, int *index) {
  int i;

  if(index == NULL) {
    for(i = 0; i < q->current; i++)
      renderQuad(q->quads + i);
  } else {
    for(i = 0; i < q->current; i++) {
      /* printf("drawing quad %d\n", index[i]); */
      renderQuad(q->quads + index[i] );
    }
  }
  glDisable(GL_TEXTURE_2D);
}
 
void doTrails(Player *p) {
  static QuadBuffer *q = NULL;
  int size;
  int *index;
  /* Get the camera from the player visual instead of game2 */
  Camera *cam = NULL;
  
  /* Find the player's camera from gppPlayerVisuals */
  int i;
  for(i = 0; i < gnPlayerVisuals; i++) {
    if(gppPlayerVisuals[i]->pPlayer == p) {
      cam = &gppPlayerVisuals[i]->camera;
      break;
    }
  }
  
  /* If we couldn't find the player's camera, use a default position */
  if(cam == NULL) {
    /* Use the first player's camera as a fallback */
    if(gnPlayerVisuals > 0) {
      cam = &gppPlayerVisuals[0]->camera;
    } else {
      /* If no cameras are available, we can't sort properly */
      /* Just continue without sorting */
      static Camera defaultCam;
      static int initialized = 0;
      if(!initialized) {
        memset(&defaultCam, 0, sizeof(Camera));
        initialized = 1;
      }
      cam = &defaultCam;
    }
  }
  
  if(q == NULL) {
    size = getTrailCount() + 12;
    // printf("allocating QuadBuffer, size %d\n", size);
    q = createQuadBuffer(size);
  } else {
    size = getTrailCount() + 12;
    if(size > q->size) { 
      // printf("reallocating QuadBuffer, size %d\n", size);
      freeQuadBuffer(q);
      /* we don't want to reallocate immediately, no make it a bit larger */
      q = createQuadBuffer(size + 36); 
    }
  }
  q->current = 0;
  clearState();
  if(gSettingsCache.alpha_trails) {
    /* depth sort everything */
    int i;
    for(i = 0; i < game->players; i++) {
      bufferPlayerTrail(game->player + i, q);
      bufferPlayerBow(game->player + i, q);
    }
    index = getSortedQuads(q, cam->cam);
    glEnable(GL_BLEND);
    drawTrails(q, index);
    glDisable(GL_BLEND);
    if(index != NULL) free(index);
  } else {
    /* draw non-transparent trails first (unsorted), then draw
       bows */
    int i;
    /* flat shaded, no blending */
    for(i = 0; i < game->players; i++) {
      bufferPlayerTrail(game->player + i, q);
    }

    drawTrails(q, NULL);

    for(i = 0; i < game->players; i++) {
      bufferPlayerBow(game->player + i, q);
    }
    /* bows are transparent, so sort back-to-front */
    index = getSortedQuads(q, cam->cam);
    glEnable(GL_BLEND);
    drawTrails(q, index);
    glDisable(GL_BLEND);
    if(index != NULL) free(index);
  }
  
  /* 
     printf("%d texture bounds\n", state->binds);
     printf("%d texture mod changes\n", state->mod_changes);
  */
}
