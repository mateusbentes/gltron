#include <stdio.h>
#include "base/nebu_assert.h"

#include "filesystem/path.h"
#include "video/nebu_mesh.h"
#include "video/nebu_texture2d.h"
#include "video/video_level.h"
#include "video/video.h"
#include "video/model.h"
#include "game/resource.h"
#include "base/nebu_resource.h"

#include "Nebu_scripting.h"
#include "video/nebu_renderer_gl.h"

#include <string.h>

#include "base/nebu_debug_memory.h"

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

gltron_Mesh* loadMesh(void);

void video_FreeLevel(video_level *l) {
	// TODO (important): change texture handling
	if(gpTokenCurrentFloor)
	{
		resource_Free(gpTokenCurrentFloor);
		gpTokenCurrentFloor = 0;
	}
	else
	{
		if(l->floor)
			gltron_Mesh_Free(l->floor);
	}
	if(gpTokenCurrentLevel)
	{
		resource_Free(gpTokenCurrentLevel);
		gpTokenCurrentLevel = 0;
	}
	else
	{
		if(l->arena)
			gltron_Mesh_Free(l->arena);
	}
	resource_Free(l->arena_shader.ridTexture);
	resource_Free(l->floor_shader.ridTexture);
	free(l);
}

void video_ScaleLevel(video_level *l, float fSize)
{
	nebu_assert(l->floor);
	gltron_Mesh_Scale(l->floor, fSize);

	if(l->arena)
		gltron_Mesh_Scale(l->arena, fSize);
}

void video_Shader_Geometry(gltron_Mesh *pMesh, gltron_MeshType eType, int pass)
{
	switch(pass)
	{
	case 0:
		gltron_Mesh_Draw(pMesh, eType);
		break;
	case 1:
		drawSharpEdges(pMesh);
		break;
	}
}

void video_Shader_Setup(video_level_shader* shader, int pass) {
	switch(pass)
	{
	case 0:
		if(shader->idTexture)
		{
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, shader->idTexture);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glMatrixMode(GL_TEXTURE);
			glLoadIdentity();
			glScalef(shader->fDiffuseTextureScale, shader->fDiffuseTextureScale, shader->fDiffuseTextureScale);
			glMatrixMode(GL_MODELVIEW);
		}
		else
		{
			glDisable(GL_TEXTURE_2D);
		}
		if(shader->lit)
		{
			glEnable(GL_LIGHTING);
		}
		break;
	case 1:
		glColor4f(.5, .5, .5, 1.0f);
		glPolygonOffset(1,4);
		glEnable(GL_POLYGON_OFFSET_LINE);
		break;
	}
}

void video_Shader_Cleanup(video_level_shader* shader, int pass)
{
	switch(pass)
	{
	case 0:
		if(shader->idTexture)
		{
			glMatrixMode(GL_TEXTURE);
			glLoadIdentity();
			glMatrixMode(GL_MODELVIEW);
			glDisable(GL_TEXTURE_2D);
		}
		if(shader->lit)
		{
			glDisable(GL_LIGHTING);
		}
		break;
	case 1:
		glDisable(GL_POLYGON_OFFSET_LINE);
		break;
	}
}

void video_shader_InitResources(video_level_shader *shader)
{
    if (!shader) {
        fprintf(stderr, "[error] video_shader_InitResources: NULL shader\n");
        return;
    }

    nebu_Texture2D *pTexture = NULL;
    if(shader->ridTexture)
        pTexture = (nebu_Texture2D*)resource_Get(shader->ridTexture, eRT_Texture);
    if(pTexture)
    {
        shader->idTexture = pTexture->id;
    }
    else
        shader->idTexture = 0;
}

int level_LoadTexture() {
	int rid;
	char *filename;
	int filter[] = { GL_NEAREST, GL_LINEAR, GL_LINEAR_MIPMAP_NEAREST, 
									 GL_LINEAR_MIPMAP_LINEAR };
	int wrap[] = { GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_REPEAT };
	nebu_Texture2D_meta meta;
	int result;
	// int iAnisotropy;

	meta.format = GL_RGBA;

	scripting_GetValue("min_filter");
	scripting_GetIntegerResult(&result);
	meta.min_filter = filter[result];

	scripting_GetValue("mag_filter");
	scripting_GetIntegerResult(&result);
	meta.mag_filter = filter[result];

	scripting_GetValue("wrap_s");
	scripting_GetIntegerResult(&result);
	meta.wrap_s = wrap[result];

	scripting_GetValue("wrap_t");
	scripting_GetIntegerResult(&result);
	meta.wrap_t = wrap[result];

	scripting_GetValue("anisotropic_filtering");
	if(scripting_IsNil())
	{
		meta.anisotropy = 1.0f;
		scripting_Pop();
	}
	else
	{
		scripting_GetFloatResult(&meta.anisotropy);
	}

	scripting_GetValue("file");
	scripting_GetStringResult(& filename);
	rid = resource_GetTokenMeta(filename, eRT_Texture, &meta, sizeof(nebu_Texture2D_meta));
	scripting_StringResult_Free(filename);

	return rid;
}

void level_LoadShader(video_level_shader *shader) {
	scripting_GetValue("shading");
	scripting_GetValue("lit");
	scripting_GetIntegerResult(& shader->lit);
	scripting_GetValue("passes");
	if(scripting_IsNil())
	{
		shader->passes = 1;
		scripting_Pop();
	}
	else
	{
		scripting_GetIntegerResult(&shader->passes);
	}
	scripting_GetValue("textures");
	if(scripting_IsNil())
	{
		shader->ridTexture = 0;
		shader->idTexture = 0;
		shader->fDiffuseTextureScale = 1;
	}
	else
	{
		scripting_GetValue("diffuse");
		
		shader->idTexture = 0;
		shader->ridTexture = level_LoadTexture();

		scripting_GetValue("texture_scale");
		if(!scripting_IsNil())
		{
			scripting_GetFloatResult(& shader->fDiffuseTextureScale);
		}
		else
		{
			shader->fDiffuseTextureScale = 1;
			scripting_Pop(); // texture_scale
		}

		scripting_Pop(); // diffuse
	}
	scripting_Pop(); // textures
	scripting_Pop(); // shading
}

void loadModel(gltron_Mesh **ppMesh, int *pToken)
{
	nebu_assert(!*pToken);
	nebu_assert(!*ppMesh);

	scripting_GetValue("model");
	if(scripting_IsNil())
	{
		scripting_Pop(); // model
		*ppMesh = loadMesh();
	}
	else
	{
		char *pFilename, *path;
		scripting_GetStringResult(&pFilename);
		path = getPath(PATH_DATA, pFilename);
		scripting_StringResult_Free(pFilename);
		if(!path)
		{
			fprintf(stderr, "fatal: could not find model - exiting...\n");
			nebu_assert(0); exit(1); // OK: critical, installation corrupt
		}
		*pToken = resource_GetToken(path, eRT_GLtronTriMesh);
		free(path);
		if(!*pToken)
		{
			fprintf(stderr, "fatal: could not load arena - exiting...\n");
			nebu_assert(0); exit(1); // OK: critical, installation corrupt
		}
		*ppMesh = resource_Get(*pToken, eRT_GLtronTriMesh);
	}
}

video_level* video_CreateLevel(void) {
    video_level *l = malloc(sizeof(video_level));
    if (!l) {
        fprintf(stderr, "[error] Memory allocation failed for video_level\n");
        return NULL;
    }
    memset(l, 0, sizeof(video_level));

    int iPos = scripting_StackGuardStart();
    lua_State *L = scripting_GetLuaState();
    if (!L) {
        fprintf(stderr, "[error] Lua state is NULL\n");
        free(l);
        return NULL;
    }

    // Load level script
    const char *levelScript = get_embedded_script("level.lua");
    if (levelScript) {
        if (luaL_loadbuffer(L, levelScript, strlen(levelScript), "level.lua") != 0 ||  // Use 0 for success
            lua_pcall(L, 0, 0, 0) != 0) {  // Use 0 for success
            fprintf(stderr, "[error] Failed to execute embedded level script: %s\n", lua_tostring(L, -1));
            lua_pop(L, 1);
            free(l);
            scripting_StackGuardEnd(iPos);
            return NULL;
        }
    } else if (!runScript(PATH_SCRIPTS, "level.lua")) {
        fprintf(stderr, "[error] Failed to find script: level.lua\n");
        free(l);
        scripting_StackGuardEnd(iPos);
        return NULL;
    }

    // Load floor script
    const char *floorScript = get_embedded_script("floor.lua");
    if (floorScript) {
        if (luaL_loadbuffer(L, floorScript, strlen(floorScript), "floor.lua") != 0 ||  // Use 0 for success
            lua_pcall(L, 0, 0, 0) != 0) {  // Use 0 for success
            fprintf(stderr, "[error] Failed to execute embedded floor script: %s\n", lua_tostring(L, -1));
            lua_pop(L, 1);
            free(l);
            scripting_StackGuardEnd(iPos);
            return NULL;
        }
    } else if (!runScript(PATH_SCRIPTS, "floor.lua")) {
        fprintf(stderr, "[error] Failed to find script: floor.lua\n");
        free(l);
        scripting_StackGuardEnd(iPos);
        return NULL;
    }
    loadModel(&l->floor, &gpTokenCurrentFloor);
    level_LoadShader(&l->floor_shader);

    // Load arena script
    const char *arenaScript = get_embedded_script("arena.lua");
    if (arenaScript) {
        if (luaL_loadbuffer(L, arenaScript, strlen(arenaScript), "arena.lua") != 0 ||  // Use 0 for success
            lua_pcall(L, 0, 0, 0) != 0) {  // Use 0 for success
            fprintf(stderr, "[error] Failed to execute embedded arena script: %s\n", lua_tostring(L, -1));
            lua_pop(L, 1);
            free(l);
            scripting_StackGuardEnd(iPos);
            return NULL;
        }
    } else if (!runScript(PATH_SCRIPTS, "arena.lua")) {
        fprintf(stderr, "[error] Failed to find script: arena.lua\n");
        free(l);
        scripting_StackGuardEnd(iPos);
        return NULL;
    }
    loadModel(&l->arena, &gpTokenCurrentLevel);
    level_LoadShader(&l->arena_shader);

    scripting_StackGuardEnd(iPos);
    return l;
}

enum {
	MESH_POSITION = 1,
	MESH_NORMAL = 2,
	MESH_TEXCOORD0 = 4
};

enum {
	V_POS = 0,
	V_NORMAL,
	V_TEXCOORD0
};
	
gltron_Mesh* loadMesh(void) {
	gltron_Mesh *pMesh;
	int i, j;
	int nPrimitives, nVertices, vertexformat;

	int iPos = scripting_StackGuardStart();

	scripting_GetValue("indices");
	nebu_assert(!scripting_IsNil());
	scripting_GetArraySize(&nPrimitives);
	scripting_Pop(); // indices

	scripting_GetValue("vertexformat");
	nebu_assert(!scripting_IsNil());
	scripting_GetIntegerResult(&vertexformat);

	scripting_GetValue("vertices");
	nebu_assert(!scripting_IsNil());
	scripting_GetArraySize(&nVertices);
	scripting_Pop(); // vertices

	pMesh = gltron_Mesh_Create(vertexformat, nVertices, &nPrimitives, 1); // Only one Material

	scripting_GetValue("vertices");
	for(i = 0; i < pMesh->pVB->nVertices; i++) {
		scripting_GetArrayIndex(i + 1);
		if(pMesh->pVB->vertexformat & NEBU_MESH_POSITION) {
			scripting_GetValue("pos");
			scripting_GetValue("x");
			scripting_GetFloatResult( & pMesh->pVB->pVertices[3 * i + 0] );
			scripting_GetValue("y");
			scripting_GetFloatResult( & pMesh->pVB->pVertices[3 * i + 1] );
			scripting_GetValue("z");
			scripting_GetFloatResult( & pMesh->pVB->pVertices[3 * i + 2] );
			scripting_Pop(); // pos
		}
		if(pMesh->pVB->vertexformat & NEBU_MESH_NORMAL) {
			scripting_GetValue("normal");
			scripting_GetValue("x");
			scripting_GetFloatResult( & pMesh->pVB->pNormals[3 * i + 0] );
			scripting_GetValue("y");
			scripting_GetFloatResult( & pMesh->pVB->pNormals[3 * i + 1] );
			scripting_GetValue("z");
			scripting_GetFloatResult( & pMesh->pVB->pNormals[3 * i + 2] );
			scripting_Pop(); // pos
		}
		if(pMesh->pVB->vertexformat & NEBU_MESH_TEXCOORD0) {
			scripting_GetValue("uv");
			scripting_GetValue("u");
			scripting_GetFloatResult( & pMesh->pVB->pTexCoords[0][2 * i + 0] );
			scripting_GetValue("v");
			scripting_GetFloatResult( & pMesh->pVB->pTexCoords[0][2 * i + 1] );
			scripting_Pop(); // uv
		}
		scripting_Pop(); // index i
	}
	scripting_Pop(); // vertices
	
	scripting_GetValue("indices");
	for(i = 0; i < pMesh->ppIB[0]->nPrimitives; i++) {
		scripting_GetArrayIndex(i + 1);
		for(j = 0; j < 3; j++)
		{
			scripting_GetArrayIndex(j + 1);
			scripting_GetIntegerResult( & pMesh->ppIB[0]->pIndices[3 * i + j] );
		}
		scripting_Pop(); // index i;
	}
	scripting_Pop(); // indices

	scripting_StackGuardEnd(iPos);

	return pMesh;
}
