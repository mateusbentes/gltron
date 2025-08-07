#include "game/game_level.h"
#include "game/game.h"
#include "filesystem/path.h"
#include <stdlib.h>
#include <stdio.h>

#ifdef USE_SCRIPTING
#include "scripting/nebu_scripting.h"
#endif

void computeBoundaries(game_level *l);

void game_FreeLevel(game_level *l) {
	int i;

	if(l->nAxis)
	{
		free(l->pAxis);
	}

	if(l->nBoundaries)
	{
		free(l->boundaries);
	}
	
	for(i = 0; i < l->nSpawnSets; i++)
	{
		game_spawnset_Free(l->ppSpawnSets[i]);
	}
	free(l->ppSpawnSets);
	free(l);
}

void game_ScaleLevel(game_level *l, float fSize)
{
	int i;
	for(i = 0; i < l->nBoundaries; i++)
	{
		segment2_Scale(&l->boundaries[i], fSize);
	}
	if(!l->spawnIsRelative)
	{
		for(i = 0; i < l->nSpawnSets; i++)
		{
			int j;
			for(j = 0; j < l->ppSpawnSets[i]->nPoints; j++)
			{
				vec2_Scale(&l->ppSpawnSets[i]->pSpawnPoints[j].vStart, fSize);
				vec2_Scale(&l->ppSpawnSets[i]->pSpawnPoints[j].vEnd, fSize);
			}
		}
	}

	box2_Scale(&l->boundingBox, fSize);
}

void computeBoundingBox(game_level *l)
{
	int i;

	box2_Init(&l->boundingBox);
	for(i = 0; i < l->nBoundaries; i++)
	{
		vec2 vEnd;
		vec2_Add(&vEnd, &l->boundaries[i].vStart, &l->boundaries[i].vDirection);
		box2_Extend(&l->boundingBox, &l->boundaries[i].vStart);
		box2_Extend(&l->boundingBox, &vEnd);
	}
}

game_spawnset* game_spawnset_Create(void) {
    game_spawnset* pSpawnSet = malloc(sizeof(game_spawnset));

#ifdef USE_SCRIPTING
    scripting_StringResult s;
    int i;

    scripting_GetValue("type");
    scripting_GetStringResult(&s);

    if(strcmp(s, "list") == 0) {
        pSpawnSet->type = eGameSpawnPoint;
    } else if(strcmp(s, "lines") == 0) {
        pSpawnSet->type = eGameSpawnLine;
    } else {
        pSpawnSet->type = eGameSpawnUndef;
    }
    scripting_StringResult_Free(s);

    scripting_GetValue("set");
    scripting_GetArraySize(&pSpawnSet->nPoints);
    pSpawnSet->pSpawnPoints = malloc(pSpawnSet->nPoints * sizeof(game_spawnpoint));

    for(i = 0; i < pSpawnSet->nPoints; i++) {
        game_spawnpoint *pSpawnPoint = pSpawnSet->pSpawnPoints + i;
        scripting_GetArrayIndex(i + 1);

        scripting_GetValue("x");
        scripting_GetFloatResult(&pSpawnPoint->vStart.v[0]);
        scripting_GetValue("y");
        scripting_GetFloatResult(&pSpawnPoint->vStart.v[1]);
        scripting_GetValue("dir");
        scripting_GetIntegerResult(&pSpawnPoint->dir);

        scripting_Pop(); // index
    }

    scripting_Pop(); // set
#else
    // Create default spawn set with 4 spawn points
    pSpawnSet->type = eGameSpawnPoint;
    pSpawnSet->nPoints = 4;
    pSpawnSet->pSpawnPoints = malloc(4 * sizeof(game_spawnpoint));
    
    // Set up 4 default spawn points
    pSpawnSet->pSpawnPoints[0].vStart.v[0] = -0.5f; pSpawnSet->pSpawnPoints[0].vStart.v[1] = -0.5f; pSpawnSet->pSpawnPoints[0].dir = 0;
    pSpawnSet->pSpawnPoints[1].vStart.v[0] = 0.5f;  pSpawnSet->pSpawnPoints[1].vStart.v[1] = -0.5f; pSpawnSet->pSpawnPoints[1].dir = 1;
    pSpawnSet->pSpawnPoints[2].vStart.v[0] = 0.5f;  pSpawnSet->pSpawnPoints[2].vStart.v[1] = 0.5f;  pSpawnSet->pSpawnPoints[2].dir = 2;
    pSpawnSet->pSpawnPoints[3].vStart.v[0] = -0.5f; pSpawnSet->pSpawnPoints[3].vStart.v[1] = 0.5f;  pSpawnSet->pSpawnPoints[3].dir = 3;
#endif

    return pSpawnSet;
}

void game_spawnset_Free(game_spawnset* pSpawnSet)
{
	free(pSpawnSet->pSpawnPoints);
	free(pSpawnSet);
}

game_level* game_CreateLevel(void) {
    int i;
    game_level* l = malloc(sizeof(game_level));

#ifdef USE_SCRIPTING
    int iHasBoundary;

    scripting_GetGlobal("level", NULL);

    scripting_GetOptional_Float("scale_factor", &l->scale_factor, 1);
    scripting_GetOptional_Int("spawn_is_relative", &l->spawnIsRelative, 1);

    scripting_GetValue("spawn");
    scripting_GetArraySize(&l->nSpawnSets);
    l->ppSpawnSets = malloc(l->nSpawnSets * sizeof(game_spawnset*));

    for(i = 0; i < l->nSpawnSets; i++) {
        scripting_GetArrayIndex(i + 1);
        l->ppSpawnSets[i] = game_spawnset_Create();
        scripting_Pop();
    }
    scripting_Pop(); // spawn

    scripting_GetValue("boundary");
    iHasBoundary = scripting_IsNil() ? 0 : 1;
    scripting_Pop();

    if(iHasBoundary) {
        scripting_GetValue("boundary");
        scripting_GetArraySize(&l->nBoundaries);
        l->boundaries = malloc(l->nBoundaries * sizeof(segment2));

        for(i = 0; i < l->nBoundaries; i++) {
            scripting_GetArrayIndex(i + 1);

            scripting_GetArrayIndex(1);
            scripting_GetValue("x");
            scripting_GetFloatResult(&l->boundaries[i].vStart.v[0]);
            scripting_GetValue("y");
            scripting_GetFloatResult(&l->boundaries[i].vStart.v[1]);
            scripting_Pop();

            scripting_GetArrayIndex(2);
            vec2 v;
            scripting_GetValue("x");
            scripting_GetFloatResult(&v.v[0]);
            scripting_GetValue("y");
            scripting_GetFloatResult(&v.v[1]);
            vec2_Sub(&l->boundaries[i].vDirection, &v, &l->boundaries[i].vStart);
            scripting_Pop();

            scripting_Pop();
        }

        scripting_Pop();
    } else {
        computeBoundaries(l);
    }

    scripting_GetValue("axis");
    scripting_GetArraySize(&l->nAxis);
    l->pAxis = malloc(l->nAxis * sizeof(vec2));

    for(i = 0; i < l->nAxis; i++) {
        scripting_GetArrayIndex(i + 1);
        scripting_GetValue("x");
        scripting_GetFloatResult(&l->pAxis[i].v[0]);
        scripting_GetValue("y");
        scripting_GetFloatResult(&l->pAxis[i].v[1]);
        scripting_Pop();
    }
    scripting_Pop();

    scripting_Pop(); // level

#else

	// Default values for boundaries and axis
    l->scale_factor = 1.0f;
    l->spawnIsRelative = 1;
    l->nSpawnSets = 1;
    l->ppSpawnSets = malloc(sizeof(game_spawnset*));
    l->ppSpawnSets[0] = game_spawnset_Create();

    l->nBoundaries = 0;
    l->boundaries = NULL;

    // Initialize with default axis (4 directions: right, up, left, down)
    l->nAxis = 4;
    l->pAxis = malloc(4 * sizeof(vec2));
    l->pAxis[0].v[0] = 1.0f; l->pAxis[0].v[1] = 0.0f;  // Right
    l->pAxis[1].v[0] = 0.0f; l->pAxis[1].v[1] = 1.0f;  // Up
    l->pAxis[2].v[0] = -1.0f; l->pAxis[2].v[1] = 0.0f; // Left
    l->pAxis[3].v[0] = 0.0f; l->pAxis[3].v[1] = -1.0f; // Down
#endif

    computeBoundingBox(l);
    return l;
}

void computeBoundaries(game_level *l)
{
	// Implementation for computing boundaries from level model
	// This is a stub - you'll need to implement this based on your requirements
	l->nBoundaries = 0;
	l->boundaries = NULL;
}

/* Changed return type from int to void to match header */
void game_LoadLevel(void) {
    char *filename;

    if(!game2) {
        fprintf(stderr, "Error: game2 is NULL in game_LoadLevel()\n");
        return;
    }

    if(game2->level) {
        game_UnloadLevel();
    }

#ifdef USE_SCRIPTING
    scripting_GetGlobal("settings", "current_level", NULL);
    scripting_GetStringResult(&filename);

    if(filename) {
        char *path = getPath(PATH_LEVEL, filename);
        scripting_StringResult_Free(filename);

        if(path) {
            if(scripting_RunFile(path)) {
                fprintf(stderr, "Error loading level '%s'\n", path);
                free(path);
                path = getPath(PATH_LEVEL, "default.lua");
                if(path) {
                    scripting_RunFile(path);
                }
            }
            free(path);
        }
    }

    game2->level = game_CreateLevel();

    if(game2->level->scale_factor != 1.0f) {
        game_ScaleLevel(game2->level, game2->level->scale_factor);
    }
#else
    fprintf(stderr, "Warning: USE_SCRIPTING not defined. Skipping game_LoadLevel.\n");
    game2->level = NULL;
#endif
}

void game_UnloadLevel(void) {
    if(game2 && game2->level) {
        game_FreeLevel(game2->level);
        game2->level = NULL;
    }
}
