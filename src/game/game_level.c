#include "game/game_level.h"
#include "game/game.h"
#include "filesystem/path.h"
#include "scripting/nebu_scripting.h"
#include <stdlib.h>
#include <stdio.h>

/* Forward declaration of computeBoundaries to avoid warning */
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

game_spawnset* game_spawnset_Create(void)
{
	scripting_StringResult s;
	int i;

	game_spawnset* pSpawnSet = (game_spawnset*) malloc(sizeof(game_spawnset));

	// get spawnset type
	scripting_GetValue("type");
	scripting_GetStringResult(&s);
	if(strcmp(s, "list") == 0) { pSpawnSet->type = eGameSpawnPoint; }
	else if(strcmp(s, "lines") == 0) { pSpawnSet->type = eGameSpawnLine; }
	else { pSpawnSet->type = eGameSpawnUndef; }
	scripting_StringResult_Free(s);

	scripting_GetValue("set");

	scripting_GetArraySize(&pSpawnSet->nPoints);
	pSpawnSet->pSpawnPoints = malloc(pSpawnSet->nPoints * sizeof(game_spawnpoint));

	// Spawn points are relative to the bounding box of the floor
	for(i = 0; i < pSpawnSet->nPoints; i++)
	{
		game_spawnpoint *pSpawnPoint = pSpawnSet->pSpawnPoints + i;

		scripting_GetArrayIndex(i + 1);

		switch(pSpawnSet->type)
		{
		case eGameSpawnPoint:
			scripting_GetValue("x");
			scripting_GetFloatResult(&pSpawnPoint->vStart.v[0]);
			scripting_GetValue("y");
			scripting_GetFloatResult(&pSpawnPoint->vStart.v[1]);
			scripting_GetValue("dir");
			scripting_GetIntegerResult(&pSpawnPoint->dir);
			break;
		case eGameSpawnLine:
			scripting_GetValue("vStart");
			scripting_GetValue("x");
			scripting_GetFloatResult(&pSpawnPoint->vStart.v[0]);
			scripting_GetValue("y");
			scripting_GetFloatResult(&pSpawnPoint->vStart.v[1]);
			scripting_Pop(); // vStart

			scripting_GetValue("vEnd");
			scripting_GetValue("x");
			scripting_GetFloatResult(&pSpawnPoint->vEnd.v[0]);
			scripting_GetValue("y");
			scripting_GetFloatResult(&pSpawnPoint->vEnd.v[1]);
			scripting_Pop(); // vEnd

			scripting_GetValue("n");
			scripting_GetIntegerResult(&pSpawnPoint->n);
			if(pSpawnPoint->n == 0)
			{
				// TODO: find a different limit somehow?
				pSpawnPoint->n = 999;
			}

			scripting_GetValue("dir");
			scripting_GetIntegerResult(&pSpawnPoint->dir);
			break;
		default:
			break;
		}

		scripting_Pop(); // index i
	}

	scripting_Pop(); // set

	return pSpawnSet;
}

void game_spawnset_Free(game_spawnset* pSpawnSet)
{
	free(pSpawnSet->pSpawnPoints);
	free(pSpawnSet);
}

game_level* game_CreateLevel(void) {
	int i;
	game_level* l;
	int iHasBoundary;

	l = malloc(sizeof(game_level));
	scripting_GetGlobal("level", NULL);
	
	// get scale factor, if present
	scripting_GetOptional_Float("scale_factor", &l->scale_factor, 1);
	// are spawn points relative?
	scripting_GetOptional_Int("spawn_is_relative", &l->spawnIsRelative, 1);

	// get number of spawnpoints
	scripting_GetValue("spawn");
	scripting_GetArraySize(&l->nSpawnSets);

	l->ppSpawnSets = malloc(l->nSpawnSets * sizeof(game_spawnset*));

	for(i = 0; i < l->nSpawnSets; i++)
	{
		scripting_GetArrayIndex(i + 1);
		l->ppSpawnSets[i] = game_spawnset_Create();
		scripting_Pop(); // index
	}
	scripting_Pop(); // spawn
	
	// TODO: in testing

	// two possibilities:
	// 1) The level contains boundary, vertices & indices fields,
	//    so we can load everything from there
	// 2) There's a 'model' field, and the boundaries	are the triangle edges
	//    without an adjacency

	scripting_GetValue("boundary");
	iHasBoundary = scripting_IsNil() ? 0 : 1;
	scripting_Pop();

	if(iHasBoundary)
	{
		// store boundary from lua
		scripting_GetValue("boundary");

		// get number of boundary segments
		scripting_GetArraySize(&l->nBoundaries);
		// copy boundaries into segments
		l->boundaries = malloc(l->nBoundaries * sizeof(segment2));
		for(i = 0; i < l->nBoundaries; i++)
		{
			scripting_GetArrayIndex(i + 1);
			
			scripting_GetArrayIndex(1);
			scripting_GetValue("x");
			scripting_GetFloatResult(&l->boundaries[i].vStart.v[0]);
			scripting_GetValue("y");
			scripting_GetFloatResult(&l->boundaries[i].vStart.v[1]);
			scripting_Pop(); // index 0
			
			scripting_GetArrayIndex(2);
			{
				vec2 v;
				scripting_GetValue("x");
				scripting_GetFloatResult(&v.v[0]);
				scripting_GetValue("y");
				scripting_GetFloatResult(&v.v[1]);
				vec2_Sub(&l->boundaries[i].vDirection, &v, &l->boundaries[i].vStart);
			}
			scripting_Pop(); // index 1
		
			scripting_Pop(); // index i
		}
		scripting_Pop(); // boundary
	}		
	else
	{
		// compute boundaries from level model
		computeBoundaries(l);
	}

	// load movement directions
	// store boundary from lua
	scripting_GetValue("axis");

	// get number of movement directions
	scripting_GetArraySize(&l->nAxis);
	// copy movement directions
	l->pAxis = malloc(l->nAxis * sizeof(vec2));
	for(i = 0; i < l->nAxis; i++)
	{
		scripting_GetArrayIndex(i + 1);
		
		scripting_GetValue("x");
		scripting_GetFloatResult(&l->pAxis[i].v[0]);
		scripting_GetValue("y");
		scripting_GetFloatResult(&l->pAxis[i].v[1]);
			
		scripting_Pop(); // index i
	}
	scripting_Pop(); // axis

	scripting_Pop(); // level

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
    
    /* Check if game2 is NULL */
    if(!game2) {
        fprintf(stderr, "Error: game2 is NULL in game_LoadLevel()\n");
        return;
    }
    
    /* Free any existing level */
    if(game2->level) {
        game_UnloadLevel();
    }
    
    /* Load level from script */
    scripting_GetGlobal("settings", "current_level", NULL);
    scripting_GetStringResult(&filename);
    
    if(filename) {
        char *path = getPath(PATH_LEVEL, filename);
        scripting_StringResult_Free(filename);
        
        if(path) {
            if(scripting_RunFile(path)) {
                fprintf(stderr, "Error loading level '%s'\n", path);
                free(path);
                /* Load a default level if the specified one fails */
                path = getPath(PATH_LEVEL, "default.lua");
                if(path) {
                    scripting_RunFile(path);
                }
            }
            free(path);
        }
    }
    
    /* Create level from script data */
    game2->level = game_CreateLevel();
    
    /* Scale level if needed */
    if(game2->level->scale_factor != 1.0f) {
        game_ScaleLevel(game2->level, game2->level->scale_factor);
    }
}

void game_UnloadLevel(void) {
    if(game2 && game2->level) {
        game_FreeLevel(game2->level);
        game2->level = NULL;
    }
}
