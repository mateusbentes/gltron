#ifndef GAME_LEVEL_H
#define GAME_LEVEL_H

#include "base/nebu_vector.h"
#include "base/nebu_util.h"
#include "base/nebu_math.h"

typedef enum {
	eGameSpawnUndef = 0,
	eGameSpawnPoint,
	eGameSpawnLine
} eGameSpawnType;

typedef struct {
	vec2 vStart;
	vec2 vEnd; // unused for eGameSpawnLine
	int n;
	int dir;
} game_spawnpoint;

typedef struct {
	eGameSpawnType type;
	int nPoints;
	game_spawnpoint *pSpawnPoints;
} game_spawnset;

typedef struct {
	int nBoundaries;
	segment2 *boundaries;
	int nAxis;
	vec2 *pAxis;
	float scale_factor;
	int spawnIsRelative;
	box2 boundingBox;
	int nSpawnSets;
	game_spawnset **ppSpawnSets;
} game_level;

void game_FreeLevel(game_level *l);
void game_ScaleLevel(game_level *l, float fSize);
game_level* game_CreateLevel(void);
game_spawnset* game_spawnset_Create(void);
void game_spawnset_Free(game_spawnset* pSpawnSet);
void computeBoundingBox(game_level *l);
void computeBoundaries(game_level *l);

/* Changed return type from int to void to match implementation */
void game_LoadLevel(void);
void game_UnloadLevel(void);

#endif
