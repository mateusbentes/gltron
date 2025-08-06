#ifndef NEBU_MESH_H
#define NEBU_MESH_H

// Android stub for nebu_mesh.h

typedef struct {
    float* vertices;
    float* normals;
    float* texcoords;
    unsigned int* indices;
    int num_vertices;
    int num_indices;
} nebu_Mesh;

typedef struct {
    int dummy;
} nebu_Mesh_ShadowInfo;

typedef struct {
    int dummy;
} nebu_Mesh_IB;

typedef struct {
    int dummy;
} nebu_Mesh_VB;

// Basic mesh functions
nebu_Mesh* nebu_Mesh_Create(void);
void nebu_Mesh_Destroy(nebu_Mesh* mesh);
void nebu_Mesh_Draw(nebu_Mesh* mesh);

#endif // NEBU_MESH_H

