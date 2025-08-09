#include "base/nebu_math.h"
#include "video/nebu_mesh.h"
#include "video/nebu_renderer_gl.h"
#include <SDL2/SDL.h>
#ifdef __ANDROID__
  #include <GLES2/gl2.h>
#else
  #include <GL/gl.h>
#endif
#include <string.h>
#include <stdlib.h>

// --- Modern Mesh Vertex Buffer ---
typedef struct {
    GLuint vbo_vertices;
    GLuint vbo_normals;
    GLuint vbo_colors;
    GLuint vbo_texcoords[NEBU_MESH_TEXCOORD_MAXCOUNT];
    GLuint ibo_indices;
    int nVertices;
    int nIndices;
    int vertexformat;
} MeshGLBuffers;

// --- Helper: Normalize a vector ---
static void normalize(float *v)
{
    float fSqrLength = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
    if(fSqrLength != 0)
    {
        float fLength = (float) sqrtf(fSqrLength); 
        v[0] /= fLength;
        v[1] /= fLength;
        v[2] /= fLength;
    }
}

// --- Upload mesh data to GPU buffers ---
void nebu_MeshGL_Upload(nebu_Mesh *pMesh, MeshGLBuffers *glbuf)
{
    // Vertices
    glGenBuffers(1, &glbuf->vbo_vertices);
    glBindBuffer(GL_ARRAY_BUFFER, glbuf->vbo_vertices);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * pMesh->pVB->nVertices, pMesh->pVB->pVertices, GL_STATIC_DRAW);

    // Normals
    if (pMesh->pVB->vertexformat & NEBU_MESH_NORMAL) {
        glGenBuffers(1, &glbuf->vbo_normals);
        glBindBuffer(GL_ARRAY_BUFFER, glbuf->vbo_normals);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * pMesh->pVB->nVertices, pMesh->pVB->pNormals, GL_STATIC_DRAW);
    }

    // Colors
    if (pMesh->pVB->vertexformat & NEBU_MESH_COLOR0) {
        glGenBuffers(1, &glbuf->vbo_colors);
        glBindBuffer(GL_ARRAY_BUFFER, glbuf->vbo_colors);
        glBufferData(GL_ARRAY_BUFFER, sizeof(int) * pMesh->pVB->nVertices, pMesh->pVB->pColor0, GL_STATIC_DRAW);
    }

    // Texcoords
    for (int i = 0; i < NEBU_MESH_TEXCOORD_MAXCOUNT; i++) {
        if (pMesh->pVB->vertexformat & (NEBU_MESH_TEXCOORD0 << i)) {
            glGenBuffers(1, &glbuf->vbo_texcoords[i]);
            glBindBuffer(GL_ARRAY_BUFFER, glbuf->vbo_texcoords[i]);
            glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 2 * pMesh->pVB->nVertices, pMesh->pVB->pTexCoords[i], GL_STATIC_DRAW);
        }
    }

    // Indices
    glGenBuffers(1, &glbuf->ibo_indices);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glbuf->ibo_indices);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * pMesh->pIB->nPrimitives * pMesh->pIB->nPrimitivesPerIndex, pMesh->pIB->pIndices, GL_STATIC_DRAW);

    glbuf->nVertices = pMesh->pVB->nVertices;
    glbuf->nIndices = pMesh->pIB->nPrimitives * pMesh->pIB->nPrimitivesPerIndex;
    glbuf->vertexformat = pMesh->pVB->vertexformat;
}

// --- Modern mesh draw using attribute pointers ---
void nebu_Mesh_DrawGeometry_Modern(nebu_Mesh *pMesh, MeshGLBuffers *glbuf, GLint aPosition, GLint aNormal, GLint aColor, GLint aTexCoord)
{
    glBindBuffer(GL_ARRAY_BUFFER, glbuf->vbo_vertices);
    glVertexAttribPointer(aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(aPosition);

    if ((glbuf->vertexformat & NEBU_MESH_NORMAL) && aNormal >= 0) {
        glBindBuffer(GL_ARRAY_BUFFER, glbuf->vbo_normals);
        glVertexAttribPointer(aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(aNormal);
    }

    if ((glbuf->vertexformat & NEBU_MESH_COLOR0) && aColor >= 0) {
        glBindBuffer(GL_ARRAY_BUFFER, glbuf->vbo_colors);
        glVertexAttribPointer(aColor, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, 0);
        glEnableVertexAttribArray(aColor);
    }

    // Only one texcoord set for simplicity
    if ((glbuf->vertexformat & NEBU_MESH_TEXCOORD0) && aTexCoord >= 0) {
        glBindBuffer(GL_ARRAY_BUFFER, glbuf->vbo_texcoords[0]);
        glVertexAttribPointer(aTexCoord, 2, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(aTexCoord);
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glbuf->ibo_indices);
    glDrawElements(GL_TRIANGLES, glbuf->nIndices, GL_UNSIGNED_INT, 0);

    glDisableVertexAttribArray(aPosition);
    if ((glbuf->vertexformat & NEBU_MESH_NORMAL) && aNormal >= 0)
        glDisableVertexAttribArray(aNormal);
    if ((glbuf->vertexformat & NEBU_MESH_COLOR0) && aColor >= 0)
        glDisableVertexAttribArray(aColor);
    if ((glbuf->vertexformat & NEBU_MESH_TEXCOORD0) && aTexCoord >= 0)
        glDisableVertexAttribArray(aTexCoord);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

// --- The rest of the mesh math and allocation code remains unchanged ---
void nebu_Mesh_ComputeTriangleNormal(nebu_Mesh *pMesh, int triangle, float* normal)
{
    float v1[3], v2[3];
    int a, b, c, i;

    a = pMesh->pIB->pIndices[3 * triangle + 0];
    b = pMesh->pIB->pIndices[3 * triangle + 1];
    c = pMesh->pIB->pIndices[3 * triangle + 2];
    for(i = 0; i < 3; i++)
    {
        v1[i] = pMesh->pVB->pVertices[3 * b + i] - pMesh->pVB->pVertices[3 * a + i];
        v2[i] = pMesh->pVB->pVertices[3 * c + i] - pMesh->pVB->pVertices[3 * a + i];
    }
    normal[0] = v1[1] * v2[2] - v1[2] * v2[1];
    normal[1] = v1[2] * v2[0] - v1[0] * v2[2];
    normal[2] = v1[0] * v2[1] - v1[1] * v2[0];
}

void nebu_Mesh_ComputeNormals(nebu_Mesh *pMesh)
{
    int i, j;

    if(!pMesh->pVB->pNormals)
        pMesh->pVB->pNormals = (float*) malloc( 3 * sizeof(float) * pMesh->pVB->nVertices );
    pMesh->pVB->vertexformat |= NEBU_MESH_NORMAL;

    memset(pMesh->pVB->pNormals, 0, 3 * sizeof(float) * pMesh->pVB->nVertices);
    for(i = 0; i < pMesh->pIB->nPrimitives; i++)
    {
        float normal[3];
        nebu_Mesh_ComputeTriangleNormal(pMesh, i, normal);

        for(j = 0; j < 3; j++)
        {
            int vertex = pMesh->pIB->pIndices[3 * i + j];
            pMesh->pVB->pNormals[3 * vertex + 0] += normal[0];
            pMesh->pVB->pNormals[3 * vertex + 1] += normal[1];
            pMesh->pVB->pNormals[3 * vertex + 2] += normal[2];
        }
    }
    for(i = 0; i < pMesh->pVB->nVertices; i++)
    {
        normalize(pMesh->pVB->pNormals + 3 * i);
    }
}

// --- Allocation and free functions remain unchanged ---
nebu_Mesh_IB* nebu_Mesh_IB_Create(int nPrimitives, int nPrimitivesPerIndex)
{
    nebu_Mesh_IB *pIB = (nebu_Mesh_IB*) malloc(sizeof(nebu_Mesh_IB));
    pIB->pIndices = (int*) malloc(nPrimitives * nPrimitivesPerIndex * sizeof(int));
    pIB->nPrimitives = nPrimitives;
    pIB->nPrimitivesPerIndex = nPrimitivesPerIndex;
    return pIB;
}

void nebu_Mesh_IB_Free(nebu_Mesh_IB *pIB)
{
    free(pIB->pIndices);
    free(pIB);
}

void nebu_Mesh_VB_Free(nebu_Mesh_VB *pVB)
{
    int i;

    if(pVB->vertexformat & NEBU_MESH_POSITION)
        free(pVB->pVertices);
    if(pVB->vertexformat & NEBU_MESH_NORMAL)
        free(pVB->pNormals);
    if(pVB->vertexformat & NEBU_MESH_COLOR0)
        free(pVB->pColor0);
    if(pVB->vertexformat & NEBU_MESH_COLOR1)
        free(pVB->pColor1);

    for(i = 0; i < NEBU_MESH_TEXCOORD_MAXCOUNT; i++)
    {
        if(pVB->vertexformat & (NEBU_MESH_TEXCOORD0 << i) &&
            pVB->pTexCoords[i])
            free(pVB->pTexCoords[i]);
    }
    free(pVB);
}

void nebu_Mesh_Free(nebu_Mesh *pMesh)
{
    nebu_Mesh_IB_Free(pMesh->pIB);
    nebu_Mesh_VB_Free(pMesh->pVB);
    free(pMesh);
}

nebu_Mesh_VB* nebu_Mesh_VB_Create(int flags, int nVertices)
{
    int i;

    nebu_Mesh_VB* pVB = (nebu_Mesh_VB*) malloc(sizeof(nebu_Mesh_VB));
    if (!pVB) return NULL;

    memset(pVB, 0, sizeof(nebu_Mesh_VB));  // zero all pointers and fields

    pVB->nVertices = nVertices;
    pVB->vertexformat = flags;

    if(flags & NEBU_MESH_POSITION)
        pVB->pVertices = (float*) malloc(3 * sizeof(float) * nVertices);

    if(flags & NEBU_MESH_NORMAL)
        pVB->pNormals = (float*) malloc(3 * sizeof(float) * nVertices);

    if(flags & NEBU_MESH_COLOR0)
        pVB->pColor0 = (int*) malloc(sizeof(int) * nVertices);

    if(flags & NEBU_MESH_COLOR1)
        pVB->pColor1 = (int*) malloc(sizeof(int) * nVertices);

    for(i = 0; i < NEBU_MESH_TEXCOORD_MAXCOUNT; i++)
    {
        if(flags & (NEBU_MESH_TEXCOORD0 << i))
            pVB->pTexCoords[i] = (float*) malloc(2 * sizeof(float) * nVertices);
    }

    return pVB;
}

nebu_Mesh* nebu_Mesh_Create(int flags, int nVertices, int nTriangles)
{
    nebu_Mesh *pMesh = (nebu_Mesh*) malloc(sizeof(nebu_Mesh));
    pMesh->pVB = nebu_Mesh_VB_Create(flags, nVertices);
    pMesh->pIB = nebu_Mesh_IB_Create(nTriangles, 3);
    return pMesh;
}

void nebu_Mesh_VB_ComputeBBox(nebu_Mesh_VB *pVB, box3* box)
{
    box3_Compute(box, (vec3*)pVB->pVertices, pVB->nVertices);
}
