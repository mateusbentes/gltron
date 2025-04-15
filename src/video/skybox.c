// skybox.c
#include "video/video.h"
#include "game/game.h"
#include "configuration/settings.h"
#include "video/nebu_renderer_gl.h"
#include "video/skybox.h"

static void bindSkyboxTexture(int index) {
    glBindTexture(GL_TEXTURE_2D, gScreen->textures[TEX_SKYBOX0 + index]);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
}

void drawSkybox(Skybox *skybox) {
    /* these are the values for y == up, x == front */
    /*
    float sides[6][4][3] = {
        { { 1, -1, -1 }, { 1, -1, 1 }, { 1, 1, 1 },  { 1, 1, -1 } }, // front
        { { 1, 1, -1 }, { 1, 1, 1 }, { -1, 1, 1 }, { -1, 1, -1 } }, // top
        { { -1, -1, -1 }, { 1, -1, -1 },  { 1, 1, -1 }, { -1, 1, -1 } }, // left
        { { 1, -1, 1 }, { -1, -1, 1 }, { -1, 1, 1 }, { 1, 1, 1 } }, // right
        { { -1, -1, -1 }, { 1, -1, -1 }, { 1, -1, 1 }, { -1, -1, 1 } }, // bottom
        { { -1, -1, 1 }, { -1, -1, -1 }, { -1, 1, -1 }, { -1, 1, a1 } } // back
        };
    */

    /* these values are for z == up, x == front */
    float sides[6][12] = {
        { 1, 1, -1,
          1, -1, -1,
          1, 1, 1,
          1, -1, 1 }, /* front */
        { 1, 1, 1,
         -1, 1, 1,
         1, -1, 1,
         -1, -1, 1 }, /* top */
        { -1, 1, -1,
          1, 1, -1,
          -1, 1, 1,
          1, 1, 1 }, /* left */
        { 1, -1, -1,
        -1, -1, -1,
        1, -1, 1,
        -1, -1, 1 }, /* right */
        { -1, 1, -1,
          -1, -1, -1,
          1, 1, -1,
          1, -1, -1 }, /* bottom */
        { -1, -1, -1,
          -1, 1, -1,
          -1, -1, 1,
          -1, 1, 1 } /* back */
    };

    float uv[4][2] = { 0, 0, 1, 0, 0, 1, 1, 1 };
    int i;

    if (!gSettingsCache.show_skybox)
        return;

    glEnable(GL_TEXTURE_2D);
    glColor4f(1.0, 1.0, 1.0, 1.0f);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, 0, uv);
    glPushMatrix();
    glScalef(1.0f, 1.0f, 1.0f);

    for (i = 0; i < 6; i++) {
        bindSkyboxTexture(i);
        glVertexPointer(3, GL_FLOAT, 0, sides[i]);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
}


gltron_Mesh* loadSkyboxMesh(void) {
    // Implement the logic to load the skybox mesh
    // This is a placeholder implementation
    gltron_Mesh* mesh = (gltron_Mesh*)malloc(sizeof(gltron_Mesh));
    if (!mesh) {
        fprintf(stderr, "[FATAL] Failed to allocate memory for skybox mesh\n");
        exit(EXIT_FAILURE);
    }

    // Initialize the mesh with appropriate data
    // This is just a placeholder; you need to implement the actual loading logic
    // For example, you might load the mesh from a file or generate it programmatically
    // Example: Initialize with dummy data
    mesh->vertices = NULL; // Replace with actual vertex data
    mesh->numVertices = 0; // Replace with actual number of vertices
    mesh->indices = NULL; // Replace with actual index data
    mesh->numIndices = 0; // Replace with actual number of indices

    // Initialize other members as needed
    mesh->pVB = NULL;
    mesh->pSI = NULL;
    mesh->ppIB = NULL;
    mesh->nMaterials = 0;
    mesh->ppMaterials = NULL;
    mesh->bIsBBoxValid = 0;
    mesh->skyboxMesh = NULL; // Assuming this is a placeholder

	// Return the loaded mesh
	return mesh;
}
