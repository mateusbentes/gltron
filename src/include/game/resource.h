#ifndef RESOURCE_H
#define RESOURCE_H

#include "video/nebu_2d.h"

// Add this enum for resource types
typedef enum {
    RESOURCE_TEXTURE = 1,
    RESOURCE_FONT,
    RESOURCE_2D,
    RESOURCE_GLTRON_TRI_MESH,
    RESOURCE_GLTRON_QUAD_MESH
} ResourceType;

typedef enum EResourceType
{
    eRT_Surface,
    eRT_2d,
    eRT_Texture,
    eRT_Font,
    eRT_GLtronTriMesh,
    eRT_GLtronQuadMesh
} EResourceType;

/* --- Resource Token Definition --- */
typedef struct resource_token {
    EResourceType type;     // Resource type
    void *data;             // Pointer to the actual resource data
    int id;                 // Unique identifier or handle
    const char *name;       // Resource name or path
    // Add more fields as needed for your resource system
} resource_token;

/* --- Resource Management API --- */
void resource_Init(void);
void resource_Shutdown(void);
int resource_LoadTexture(const char* filename, int type);

resource_token *resource_GetToken(const char *path, EResourceType type);
void resource_Free(resource_token *token);

#endif /* RESOURCE_H */
