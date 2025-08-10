#ifndef NEBU_MESH_H
#define NEBU_MESH_H


#include <SDL2/SDL.h>
#ifdef __ANDROID__
  #include <GLES2/gl2.h>
#else
  #include <GL/gl.h>
#endif

#include "base/nebu_vector.h"
enum {
	NEBU_MESH_POSITION    = 0x0001,
	NEBU_MESH_NORMAL      = 0x0002,
	NEBU_MESH_TEXCOORD0   = 0x0004,
	NEBU_MESH_TEXCOORD1   = 0x0008,
	NEBU_MESH_TEXCOORD2   = 0x0010,
	NEBU_MESH_TEXCOORD3   = 0x0020,
	NEBU_MESH_TEXCOORD4   = 0x0040,
	NEBU_MESH_TEXCOORD5   = 0x0080,
	NEBU_MESH_TEXCOORD6   = 0x0100,
	NEBU_MESH_TEXCOORD7   = 0x0200,
	NEBU_MESH_COLOR0      = 0x0400,
	NEBU_MESH_COLOR1      = 0x0800,
	NEBU_MESH_TEXCOORD_MAXCOUNT = 8,

	NEBU_MESH_FLAGS = 10
};

typedef struct {
    GLuint vboVertices; // Vertex buffer object ID
    GLuint vboIndices;  // Index buffer object ID (if using indexed rendering)
    int nVertices;      // Number of vertices
    int nPrimitives;    // Number of primitives (triangles, quads, etc.)
    int stride;         // Byte offset between consecutive vertices
    float *pVertices;   // Vertex data (position)
    float *pNormals;    // Normal data (if available)
    float *pTexCoords[NEBU_MESH_TEXCOORD_MAXCOUNT]; // Texture coordinate data (if available)
    int hasNormals;     // Flag indicating if normals are present
    int hasTexCoords;   // Flag indicating if texture coordinates are present
	int *pColor0;
	int *pColor1;
	int vertexformat;
} nebu_Mesh_VB;

typedef struct {
	int nPrimitivesPerIndex;
	int nPrimitives;
	int *pIndices;
} nebu_Mesh_IB;

typedef struct {
	nebu_Mesh_VB *pVB;
	nebu_Mesh_IB *pIB;
} nebu_Mesh;

typedef struct {
	int nTriangles;
	int *pAdjacency;
} nebu_Mesh_Adjacency;

typedef struct {
	nebu_Mesh_IB *pFrontfaces;
	nebu_Mesh_IB *pBackfaces;
	nebu_Mesh_IB *pEdges;
	// private information
	nebu_Mesh_Adjacency *pAdjacency;
	int *pDotsigns;
	vec3* pFaceNormals;
	nebu_Mesh_VB *pVB;
	nebu_Mesh_VB *pVB_Extruded;
	nebu_Mesh_IB *pIB;
	vec3 vLight;
} nebu_Mesh_ShadowInfo;

nebu_Mesh_Adjacency* nebu_Mesh_Adjacency_Create(const nebu_Mesh_VB *pVB, const nebu_Mesh_IB *pIB);
void nebu_Mesh_Adjacency_Free(nebu_Mesh_Adjacency *pAdjacency);
nebu_Mesh_ShadowInfo* nebu_Mesh_Shadow_Create(nebu_Mesh_VB *pVB, nebu_Mesh_IB *pIB);
void nebu_Mesh_Shadow_Free(nebu_Mesh_ShadowInfo* pShadowInfo);
void nebu_Mesh_Shadow_SetLight(nebu_Mesh_ShadowInfo* pShadowInfo, const vec3* vLight);
vec3* nebu_Mesh_ComputeFaceNormals(const nebu_Mesh_VB *pVB, const nebu_Mesh_IB *pIB);

nebu_Mesh_IB* nebu_Mesh_IB_Create(int nPrimitives, int nPrimitivesPerIndex);
nebu_Mesh_VB* nebu_Mesh_VB_Create(int flags, int nVertices);
nebu_Mesh* nebu_Mesh_Create(int flags, int nVertices, int nTriangles);
nebu_Mesh* nebu_Mesh_Clone(int flags, nebu_Mesh *pMesh);
void nebu_Mesh_Free(nebu_Mesh *pMesh);
void nebu_Mesh_VB_Free(nebu_Mesh_VB *pVB);
void nebu_Mesh_IB_Free(nebu_Mesh_IB *pIB);
void nebu_Mesh_ComputeNormals(nebu_Mesh *pMesh);
void nebu_Mesh_ComputeTriangleNormal(nebu_Mesh *pMesh, int triangle, float* normal);
void nebu_Mesh_VB_Scale(nebu_Mesh_VB *pVB, float fScale);
void nebu_Mesh_VB_Enable(nebu_Mesh_VB *pVB);
void nebu_Mesh_VB_Disable(nebu_Mesh_VB *pVB);
void nebu_Mesh_DrawGeometry(nebu_Mesh *pMesh);
void nebu_Mesh_VB_ComputeBBox(nebu_Mesh_VB *pVB, box3* box3);
int nebu_Mesh_Validate(nebu_Mesh *pMesh);

#endif
