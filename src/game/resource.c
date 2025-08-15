#include <GL/gl.h>
#include "game/resource.h"
#include "base/nebu_util.h"
#include "base/nebu_resource.h"
#include "video/nebu_font.h"
#include "video/nebu_texture2d.h"
#include "filesystem/nebu_filesystem.h"
#include "base/nebu_assert.h"
#include <string.h>

// TODO: move user type load functions somewhere else
#include "filesystem/path.h"
#include "video/model.h"

#include "base/nebu_debug_memory.h"

// Forward declaration of resource_Get function
void* resource_Get(int token, int type);

// Forward declaration of findToken function
resource_token* findToken(int token);

// Function to load a texture
int resource_LoadTexture(const char* filename, int type) {
    printf("[resource] Loading texture %s of type %d\n", filename, type);

    // Get the full path to the texture file
    char* path = nebu_FS_GetPath_WithFilename(PATH_ART, filename);
    if (!path) {
        fprintf(stderr, "[resource] Failed to locate texture file: %s\n", filename);
        return -1;
    }

    // Load the texture using the appropriate loader
    void* textureData = nebu_Texture2D_Load(path, NULL);
    free(path);

    if (!textureData) {
        fprintf(stderr, "[resource] Failed to load texture: %s\n", filename);
        return -1;
    }

    // Get the texture ID from the loaded data
    nebu_Texture2D* texture = (nebu_Texture2D*)textureData;
    int textureId = texture->id;

    // Store the texture in the resource system
    // Create a non-const copy of the filename
    char* filenameCopy = strdup(filename);
    if (!filenameCopy) {
        fprintf(stderr, "[resource] Failed to allocate memory for filename copy\n");
        nebu_Texture2D_Free(texture);
        return -1;
    }

    int token = resource_GetTokenMeta(filenameCopy, type, NULL, 0);
    free(filenameCopy);

    if (token == 0) {
        fprintf(stderr, "[resource] Failed to create resource token for texture: %s\n", filename);
        nebu_Texture2D_Free(texture);
        return -1;
    }

    // Store the texture data in the resource system
    resource_token* pToken = findToken(token);
    if (!pToken) {
        fprintf(stderr, "[resource] Failed to find resource token for texture: %s\n", filename);
        nebu_Texture2D_Free(texture);
        return -1;
    }

    // Assign the texture data to the resource token
    pToken->data = textureData;
    pToken->type = type;
    pToken->id = textureId;
    pToken->name = filename;

    printf("[resource] Texture %s loaded with ID %d\n", filename, textureId);
    return textureId;
}

void* getTriMesh(char *filename, void *dummy)
{
    return gltron_Mesh_LoadFromFile(filename, TRI_MESH);
}

void* getQuadMesh(char *filename, void *dummy)
{
    return gltron_Mesh_LoadFromFile(filename, QUAD_MESH);
}

static void releaseMesh(void *pData)
{
    gltron_Mesh_Free((gltron_Mesh*)pData);
}

void* get2d(char *filename, void *dummy)
{
    char *path;
    path = nebu_FS_GetPath_WithFilename(PATH_ART, filename);
    if(path)
    {
        void *pData = nebu_2d_LoadPNG(path, 0);
        free(path);
        return pData;
    }
    else
    {
        fprintf(stderr, "failed to locate %s", filename);
        nebu_assert(0); exit(1); // installation corrupt
    }
}

static void release2d(void *pData)
{
    nebu_2d_Free((nebu_2d*)pData);
}

void* getFont(char *filename, void *dummy)
{
    return nebu_Font_Load(filename, 16, 16, 32, 96);
}

static void releaseFont(void *pData)
{
    nebu_Font_Free(pData);
}

void* getTexture(char *filename, void *meta)
{
    char *path;
    path = nebu_FS_GetPath_WithFilename(PATH_ART, filename);
    if(path)
    {
        void *pData = nebu_Texture2D_Load(path, (nebu_Texture2D_meta*) meta);
        free(path);
        return pData;
    }
    else
    {
        fprintf(stderr, "failed to locate %s", filename);
        nebu_assert(0); exit(1); // installation corrupt
    }
}

static void releaseTexture(void *pData)
{
    nebu_Texture2D_Free((nebu_Texture2D*)pData);
}

void resource_Init()
{
    resource_RegisterHandler(eRT_GLtronTriMesh, getTriMesh, releaseMesh);
    resource_RegisterHandler(eRT_GLtronQuadMesh, getQuadMesh, releaseMesh);
    resource_RegisterHandler(eRT_2d, get2d, release2d);
    resource_RegisterHandler(eRT_Font, getFont, releaseFont);
    resource_RegisterHandler(eRT_Texture, getTexture, releaseTexture);
}

void resource_Shutdown(void)
{
    resource_UnregisterHandler(eRT_GLtronTriMesh);
    resource_UnregisterHandler(eRT_GLtronQuadMesh);
    resource_UnregisterHandler(eRT_2d);
    resource_UnregisterHandler(eRT_Font);
    resource_UnregisterHandler(eRT_Texture);
}

void resource_GetTextureDimensions(int textureId, int* width, int* height) {
    printf("[resource] Getting dimensions for texture %d\n", textureId);

    // Get the texture data using the correct resource type
    void* textureData = resource_Get(textureId, RESOURCE_TEXTURE);

    if (!textureData) {
        printf("[resource] Error: Texture %d not found\n", textureId);
        *width = 0;
        *height = 0;
        return;
    }

    // Cast the texture data to the appropriate type
    nebu_Texture2D* texture = (nebu_Texture2D*)textureData;

    // Get the texture dimensions using OpenGL functions
    GLint texWidth = 0;
    GLint texHeight = 0;

    // Bind the texture to get its dimensions
    glBindTexture(GL_TEXTURE_2D, texture->id);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &texWidth);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &texHeight);

    // Set the output parameters
    *width = texWidth;
    *height = texHeight;

    printf("[resource] Texture %d dimensions: %dx%d\n", textureId, *width, *height);
}
