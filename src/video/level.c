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
#include "video/nebu_mesh.h"

#include <string.h>
#include <math.h>  /* For sqrtf */

#include "base/nebu_debug_memory.h"

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

/* Forward declarations for functions used in video_CreateLevel */
// gltron_Mesh* createFloorMesh(void);  // Commented out to avoid compilation errors
// gltron_Mesh* createArenaMesh(void);  // Commented out to avoid compilation errors

/*
 * Implementation of loadMesh function
 * This function is referenced in loadModel but not defined
 */
gltron_Mesh* loadMesh(void) {
    printf("[video] Creating a simple mesh (stub implementation)\n");
    
    // Return NULL to avoid using undefined types
    // The calling function should handle this case
    return NULL;
}

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
        printf("[video] Loading mesh using loadMesh()\n");
        *ppMesh = loadMesh();
        if(!*ppMesh) {
            printf("[video] loadMesh() returned NULL, creating a minimal mesh\n");
            // Create a minimal mesh structure to avoid crashes
            *ppMesh = (gltron_Mesh*)malloc(sizeof(gltron_Mesh));
            if(!*ppMesh) {
                fprintf(stderr, "fatal: could not allocate memory for mesh - exiting...\n");
                nebu_assert(0); exit(1); // OK: critical, out of memory
            }
            memset(*ppMesh, 0, sizeof(gltron_Mesh));
        }
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
    printf("[video] Creating level (minimal implementation)\n");
    
    // Allocate memory for the level structure
    video_level *l = malloc(sizeof(video_level));
    if (!l) {
        fprintf(stderr, "[error] Memory allocation failed for video_level\n");
        return NULL;
    }
    memset(l, 0, sizeof(video_level));
    
    // Set up shader properties
    l->floor_shader.lit = 1;  // Enable lighting
    l->floor_shader.passes = 1;
    l->floor_shader.ridTexture = 0;
    l->floor_shader.idTexture = 0;
    l->floor_shader.fDiffuseTextureScale = 1.0f;
    
    l->arena_shader.lit = 1;  // Enable lighting
    l->arena_shader.passes = 1;
    l->arena_shader.ridTexture = 0;
    l->arena_shader.idTexture = 0;
    l->arena_shader.fDiffuseTextureScale = 1.0f;
    
    // Create a minimal floor mesh directly (without using loadModel)
    printf("[video] Creating minimal floor mesh directly\n");
    
    // Create a minimal mesh structure to avoid crashes
    l->floor = (gltron_Mesh*)malloc(sizeof(gltron_Mesh));
    if(!l->floor) {
        fprintf(stderr, "fatal: could not allocate memory for floor mesh - exiting...\n");
        free(l);
        return NULL;
    }
    memset(l->floor, 0, sizeof(gltron_Mesh));
    gpTokenCurrentFloor = 0;  // No resource token
    
    // Create a minimal arena mesh directly (without using loadModel)
    printf("[video] Creating minimal arena mesh directly\n");
    
    // Create a minimal mesh structure to avoid crashes
    l->arena = (gltron_Mesh*)malloc(sizeof(gltron_Mesh));
    if(!l->arena) {
        fprintf(stderr, "fatal: could not allocate memory for arena mesh - exiting...\n");
        free(l->floor);
        free(l);
        return NULL;
    }
    memset(l->arena, 0, sizeof(gltron_Mesh));
    gpTokenCurrentLevel = 0;  // No resource token
    
    printf("[video] Level created with minimal state\n");
    
    return l;
}

/*
// Create a floor mesh with actual geometry
gltron_Mesh* createFloorMesh(void) {
    printf("[video] Creating floor mesh\n");
    
    // Create a new mesh
    gltron_Mesh* mesh = (gltron_Mesh*)malloc(sizeof(gltron_Mesh));
    if(!mesh) {
        fprintf(stderr, "[error] Failed to allocate memory for floor mesh\n");
        return NULL;
    }
    memset(mesh, 0, sizeof(gltron_Mesh));
    
    // Set mesh flags
    mesh->flags = NEBU_MESH_POSITION | NEBU_MESH_NORMAL | NEBU_MESH_TEXCOORD0;
    
    // Create vertex buffer
    mesh->pVB = (nebu_VertexBuffer*)malloc(sizeof(nebu_VertexBuffer));
    if(!mesh->pVB) {
        fprintf(stderr, "[error] Failed to allocate vertex buffer for floor mesh\n");
        free(mesh);
        return NULL;
    }
    memset(mesh->pVB, 0, sizeof(nebu_VertexBuffer));
    
    // Set number of vertices (4 for a simple quad)
    mesh->pVB->nVertices = 4;
    
    // Allocate memory for vertex positions
    mesh->pVB->pVertices = (float*)malloc(3 * mesh->pVB->nVertices * sizeof(float));
    if(!mesh->pVB->pVertices) {
        fprintf(stderr, "[error] Failed to allocate vertex positions for floor mesh\n");
        free(mesh->pVB);
        free(mesh);
        return NULL;
    }
    
    // Allocate memory for vertex normals
    mesh->pVB->pNormals = (float*)malloc(3 * mesh->pVB->nVertices * sizeof(float));
    if(!mesh->pVB->pNormals) {
        fprintf(stderr, "[error] Failed to allocate vertex normals for floor mesh\n");
        free(mesh->pVB->pVertices);
        free(mesh->pVB);
        free(mesh);
        return NULL;
    }
    
    // Allocate memory for texture coordinates
    mesh->pVB->pTexCoords = (float**)malloc(1 * sizeof(float*));
    if(!mesh->pVB->pTexCoords) {
        fprintf(stderr, "[error] Failed to allocate texture coordinate array for floor mesh\n");
        free(mesh->pVB->pNormals);
        free(mesh->pVB->pVertices);
        free(mesh->pVB);
        free(mesh);
        return NULL;
    }
    
    mesh->pVB->pTexCoords[0] = (float*)malloc(2 * mesh->pVB->nVertices * sizeof(float));
    if(!mesh->pVB->pTexCoords[0]) {
        fprintf(stderr, "[error] Failed to allocate texture coordinates for floor mesh\n");
        free(mesh->pVB->pTexCoords);
        free(mesh->pVB->pNormals);
        free(mesh->pVB->pVertices);
        free(mesh->pVB);
        free(mesh);
        return NULL;
    }
    
    // Set up vertices for a simple quad (floor)
    float size = 100.0f;
    
    // Vertex positions (x, y, z)
    // Bottom-left
    mesh->pVB->pVertices[0] = -size;
    mesh->pVB->pVertices[1] = 0.0f;
    mesh->pVB->pVertices[2] = -size;
    
    // Bottom-right
    mesh->pVB->pVertices[3] = size;
    mesh->pVB->pVertices[4] = 0.0f;
    mesh->pVB->pVertices[5] = -size;
    
    // Top-right
    mesh->pVB->pVertices[6] = size;
    mesh->pVB->pVertices[7] = 0.0f;
    mesh->pVB->pVertices[8] = size;
    
    // Top-left
    mesh->pVB->pVertices[9] = -size;
    mesh->pVB->pVertices[10] = 0.0f;
    mesh->pVB->pVertices[11] = size;
    
    // Vertex normals (all pointing up)
    for(int i = 0; i < mesh->pVB->nVertices; i++) {
        mesh->pVB->pNormals[i*3 + 0] = 0.0f;
        mesh->pVB->pNormals[i*3 + 1] = 1.0f;
        mesh->pVB->pNormals[i*3 + 2] = 0.0f;
    }
    
    // Texture coordinates
    // Bottom-left
    mesh->pVB->pTexCoords[0][0] = 0.0f;
    mesh->pVB->pTexCoords[0][1] = 0.0f;
    
    // Bottom-right
    mesh->pVB->pTexCoords[0][2] = 1.0f;
    mesh->pVB->pTexCoords[0][3] = 0.0f;
    
    // Top-right
    mesh->pVB->pTexCoords[0][4] = 1.0f;
    mesh->pVB->pTexCoords[0][5] = 1.0f;
    
    // Top-left
    mesh->pVB->pTexCoords[0][6] = 0.0f;
    mesh->pVB->pTexCoords[0][7] = 1.0f;
    
    // Create index buffer
    mesh->nIB = 1;
    mesh->ppIB = (nebu_IndexBuffer**)malloc(mesh->nIB * sizeof(nebu_IndexBuffer*));
    if(!mesh->ppIB) {
        fprintf(stderr, "[error] Failed to allocate index buffer array for floor mesh\n");
        free(mesh->pVB->pTexCoords[0]);
        free(mesh->pVB->pTexCoords);
        free(mesh->pVB->pNormals);
        free(mesh->pVB->pVertices);
        free(mesh->pVB);
        free(mesh);
        return NULL;
    }
    
    mesh->ppIB[0] = (nebu_IndexBuffer*)malloc(sizeof(nebu_IndexBuffer));
    if(!mesh->ppIB[0]) {
        fprintf(stderr, "[error] Failed to allocate index buffer for floor mesh\n");
        free(mesh->ppIB);
        free(mesh->pVB->pTexCoords[0]);
        free(mesh->pVB->pTexCoords);
        free(mesh->pVB->pNormals);
        free(mesh->pVB->pVertices);
        free(mesh->pVB);
        free(mesh);
        return NULL;
    }
    
    // Set up indices for two triangles (6 indices total)
    mesh->ppIB[0]->nPrimitives = 2;  // Two triangles
    mesh->ppIB[0]->pIndices = (int*)malloc(3 * mesh->ppIB[0]->nPrimitives * sizeof(int));
    if(!mesh->ppIB[0]->pIndices) {
        fprintf(stderr, "[error] Failed to allocate indices for floor mesh\n");
        free(mesh->ppIB[0]);
        free(mesh->ppIB);
        free(mesh->pVB->pTexCoords[0]);
        free(mesh->pVB->pTexCoords);
        free(mesh->pVB->pNormals);
        free(mesh->pVB->pVertices);
        free(mesh->pVB);
        free(mesh);
        return NULL;
    }
    
    // First triangle (bottom-left, bottom-right, top-right)
    mesh->ppIB[0]->pIndices[0] = 0;
    mesh->ppIB[0]->pIndices[1] = 1;
    mesh->ppIB[0]->pIndices[2] = 2;
    
    // Second triangle (bottom-left, top-right, top-left)
    mesh->ppIB[0]->pIndices[3] = 0;
    mesh->ppIB[0]->pIndices[4] = 2;
    mesh->ppIB[0]->pIndices[5] = 3;
    
    printf("[video] Floor mesh created successfully\n");
    
    return mesh;
}

// Create an arena mesh with actual geometry
gltron_Mesh* createArenaMesh(void) {
    printf("[video] Creating arena mesh\n");
    
    // Create a new mesh
    gltron_Mesh* mesh = (gltron_Mesh*)malloc(sizeof(gltron_Mesh));
    if(!mesh) {
        fprintf(stderr, "[error] Failed to allocate memory for arena mesh\n");
        return NULL;
    }
    memset(mesh, 0, sizeof(gltron_Mesh));
    
    // Set mesh flags
    mesh->flags = NEBU_MESH_POSITION | NEBU_MESH_NORMAL | NEBU_MESH_TEXCOORD0;
    
    // Create vertex buffer
    mesh->pVB = (nebu_VertexBuffer*)malloc(sizeof(nebu_VertexBuffer));
    if(!mesh->pVB) {
        fprintf(stderr, "[error] Failed to allocate vertex buffer for arena mesh\n");
        free(mesh);
        return NULL;
    }
    memset(mesh->pVB, 0, sizeof(nebu_VertexBuffer));
    
    // Set number of vertices (8 for a simple box)
    mesh->pVB->nVertices = 8;
    
    // Allocate memory for vertex positions
    mesh->pVB->pVertices = (float*)malloc(3 * mesh->pVB->nVertices * sizeof(float));
    if(!mesh->pVB->pVertices) {
        fprintf(stderr, "[error] Failed to allocate vertex positions for arena mesh\n");
        free(mesh->pVB);
        free(mesh);
        return NULL;
    }
    
    // Allocate memory for vertex normals
    mesh->pVB->pNormals = (float*)malloc(3 * mesh->pVB->nVertices * sizeof(float));
    if(!mesh->pVB->pNormals) {
        fprintf(stderr, "[error] Failed to allocate vertex normals for arena mesh\n");
        free(mesh->pVB->pVertices);
        free(mesh->pVB);
        free(mesh);
        return NULL;
    }
    
    // Allocate memory for texture coordinates
    mesh->pVB->pTexCoords = (float**)malloc(1 * sizeof(float*));
    if(!mesh->pVB->pTexCoords) {
        fprintf(stderr, "[error] Failed to allocate texture coordinate array for arena mesh\n");
        free(mesh->pVB->pNormals);
        free(mesh->pVB->pVertices);
        free(mesh->pVB);
        free(mesh);
        return NULL;
    }
    
    mesh->pVB->pTexCoords[0] = (float*)malloc(2 * mesh->pVB->nVertices * sizeof(float));
    if(!mesh->pVB->pTexCoords[0]) {
        fprintf(stderr, "[error] Failed to allocate texture coordinates for arena mesh\n");
        free(mesh->pVB->pTexCoords);
        free(mesh->pVB->pNormals);
        free(mesh->pVB->pVertices);
        free(mesh->pVB);
        free(mesh);
        return NULL;
    }
    
    // Set up vertices for a simple box (arena)
    float size = 100.0f;
    float height = 10.0f;
    
    // Vertex positions (x, y, z)
    // Bottom vertices
    // 0: Bottom-left-back
    mesh->pVB->pVertices[0] = -size;
    mesh->pVB->pVertices[1] = 0.0f;
    mesh->pVB->pVertices[2] = -size;
    
    // 1: Bottom-right-back
    mesh->pVB->pVertices[3] = size;
    mesh->pVB->pVertices[4] = 0.0f;
    mesh->pVB->pVertices[5] = -size;
    
    // 2: Bottom-right-front
    mesh->pVB->pVertices[6] = size;
    mesh->pVB->pVertices[7] = 0.0f;
    mesh->pVB->pVertices[8] = size;
    
    // 3: Bottom-left-front
    mesh->pVB->pVertices[9] = -size;
    mesh->pVB->pVertices[10] = 0.0f;
    mesh->pVB->pVertices[11] = size;
    
    // Top vertices
    // 4: Top-left-back
    mesh->pVB->pVertices[12] = -size;
    mesh->pVB->pVertices[13] = height;
    mesh->pVB->pVertices[14] = -size;
    
    // 5: Top-right-back
    mesh->pVB->pVertices[15] = size;
    mesh->pVB->pVertices[16] = height;
    mesh->pVB->pVertices[17] = -size;
    
    // 6: Top-right-front
    mesh->pVB->pVertices[18] = size;
    mesh->pVB->pVertices[19] = height;
    mesh->pVB->pVertices[20] = size;
    
    // 7: Top-left-front
    mesh->pVB->pVertices[21] = -size;
    mesh->pVB->pVertices[22] = height;
    mesh->pVB->pVertices[23] = size;
    
    // Vertex normals (simplified - all pointing outward)
    // This is a simplification; in a real implementation, each vertex would have the correct normal
    for(int i = 0; i < mesh->pVB->nVertices; i++) {
        // Calculate direction from center to vertex
        float x = mesh->pVB->pVertices[i*3 + 0];
        float y = mesh->pVB->pVertices[i*3 + 1] - height/2.0f;  // Center is at half height
        float z = mesh->pVB->pVertices[i*3 + 2];
        
        // Normalize
        float length = sqrtf(x*x + y*y + z*z);
        if(length > 0.0001f) {
            mesh->pVB->pNormals[i*3 + 0] = x / length;
            mesh->pVB->pNormals[i*3 + 1] = y / length;
            mesh->pVB->pNormals[i*3 + 2] = z / length;
        } else {
            // Fallback for center vertices
            mesh->pVB->pNormals[i*3 + 0] = 0.0f;
            mesh->pVB->pNormals[i*3 + 1] = 1.0f;
            mesh->pVB->pNormals[i*3 + 2] = 0.0f;
        }
    }
    
    // Texture coordinates (simplified)
    for(int i = 0; i < mesh->pVB->nVertices; i++) {
        // Simple mapping based on vertex position
        mesh->pVB->pTexCoords[0][i*2 + 0] = (mesh->pVB->pVertices[i*3 + 0] + size) / (2.0f * size);
        mesh->pVB->pTexCoords[0][i*2 + 1] = (mesh->pVB->pVertices[i*3 + 2] + size) / (2.0f * size);
    }
    
    // Create index buffer
    mesh->nIB = 1;
    mesh->ppIB = (nebu_IndexBuffer**)malloc(mesh->nIB * sizeof(nebu_IndexBuffer*));
    if(!mesh->ppIB) {
        fprintf(stderr, "[error] Failed to allocate index buffer array for arena mesh\n");
        free(mesh->pVB->pTexCoords[0]);
        free(mesh->pVB->pTexCoords);
        free(mesh->pVB->pNormals);
        free(mesh->pVB->pVertices);
        free(mesh->pVB);
        free(mesh);
        return NULL;
    }
    
    mesh->ppIB[0] = (nebu_IndexBuffer*)malloc(sizeof(nebu_IndexBuffer));
    if(!mesh->ppIB[0]) {
        fprintf(stderr, "[error] Failed to allocate index buffer for arena mesh\n");
        free(mesh->ppIB);
        free(mesh->pVB->pTexCoords[0]);
        free(mesh->pVB->pTexCoords);
        free(mesh->pVB->pNormals);
        free(mesh->pVB->pVertices);
        free(mesh->pVB);
        free(mesh);
        return NULL;
    }
    
    // Set up indices for 12 triangles (36 indices total)
    // 6 faces * 2 triangles per face * 3 indices per triangle
    mesh->ppIB[0]->nPrimitives = 12;  // 12 triangles
    mesh->ppIB[0]->pIndices = (int*)malloc(3 * mesh->ppIB[0]->nPrimitives * sizeof(int));
    if(!mesh->ppIB[0]->pIndices) {
        fprintf(stderr, "[error] Failed to allocate indices for arena mesh\n");
        free(mesh->ppIB[0]);
        free(mesh->ppIB);
        free(mesh->pVB->pTexCoords[0]);
        free(mesh->pVB->pTexCoords);
        free(mesh->pVB->pNormals);
        free(mesh->pVB->pVertices);
        free(mesh->pVB);
        free(mesh);
        return NULL;
    }
    
    // Front wall (2 triangles)
    mesh->ppIB[0]->pIndices[0] = 2;
    mesh->ppIB[0]->pIndices[1] = 3;
    mesh->ppIB[0]->pIndices[2] = 7;
    
    mesh->ppIB[0]->pIndices[3] = 2;
    mesh->ppIB[0]->pIndices[4] = 7;
    mesh->ppIB[0]->pIndices[5] = 6;
    
    // Back wall (2 triangles)
    mesh->ppIB[0]->pIndices[6] = 0;
    mesh->ppIB[0]->pIndices[7] = 1;
    mesh->ppIB[0]->pIndices[8] = 5;
    
    mesh->ppIB[0]->pIndices[9] = 0;
    mesh->ppIB[0]->pIndices[10] = 5;
    mesh->ppIB[0]->pIndices[11] = 4;
    
    // Left wall (2 triangles)
    mesh->ppIB[0]->pIndices[12] = 0;
    mesh->ppIB[0]->pIndices[13] = 4;
    mesh->ppIB[0]->pIndices[14] = 7;
    
    mesh->ppIB[0]->pIndices[15] = 0;
    mesh->ppIB[0]->pIndices[16] = 7;
    mesh->ppIB[0]->pIndices[17] = 3;
    
    // Right wall (2 triangles)
    mesh->ppIB[0]->pIndices[18] = 1;
    mesh->ppIB[0]->pIndices[19] = 2;
    mesh->ppIB[0]->pIndices[20] = 6;
    
    mesh->ppIB[0]->pIndices[21] = 1;
    mesh->ppIB[0]->pIndices[22] = 6;
    mesh->ppIB[0]->pIndices[23] = 5;
    
    // Top (2 triangles)
    mesh->ppIB[0]->pIndices[24] = 4;
    mesh->ppIB[0]->pIndices[25] = 5;
    mesh->ppIB[0]->pIndices[26] = 6;
    
    mesh->ppIB[0]->pIndices[27] = 4;
    mesh->ppIB[0]->pIndices[28] = 6;
    mesh->ppIB[0]->pIndices[29] = 7;
    
    // Bottom (2 triangles)
    mesh->ppIB[0]->pIndices[30] = 0;
    mesh->ppIB[0]->pIndices[31] = 3;
    mesh->ppIB[0]->pIndices[32] = 2;
    
    mesh->ppIB[0]->pIndices[33] = 0;
    mesh->ppIB[0]->pIndices[34] = 2;
    mesh->ppIB[0]->pIndices[35] = 1;
    
    printf("[video] Arena mesh created successfully\n");
    
    return mesh;
}
*/