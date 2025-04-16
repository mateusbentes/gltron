// skybox.c
#include "video/video.h"
#include "game/game.h"
#include "configuration/settings.h"
#include "video/nebu_renderer_gl.h"
#include "video/skybox.h"
#include <lib3ds/file.h>
#include <lib3ds/mesh.h>
#include <stdio.h>
#include <stdlib.h>

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


gltron_Mesh* loadSkyboxMesh(const char* filename) {
    // Load the skybox mesh using lib3ds
    Lib3dsFile *file = lib3ds_file_load(filename);
    if (!file) {
        fprintf(stderr, "[FATAL] Failed to load skybox mesh from file: %s\n", filename);
        exit(EXIT_FAILURE);
    }

    // Find the mesh named "skybox_mesh_name"
    Lib3dsMesh *lib3ds_mesh = NULL;
    for (Lib3dsMesh *m = file->meshes; m; m = m->next) {
        if (strcmp(m->name, "skybox_mesh_name") == 0) {
            lib3ds_mesh = m;
            break;
        }
    }

    if (!lib3ds_mesh) {
        fprintf(stderr, "[FATAL] Failed to find skybox mesh named 'skybox_mesh_name' in file: %s\n", filename);
        lib3ds_file_free(file);
        exit(EXIT_FAILURE);
    }

    // Create a new gltron_Mesh structure
    gltron_Mesh* mesh = (gltron_Mesh*)malloc(sizeof(gltron_Mesh));
    if (!mesh) {
        fprintf(stderr, "[FATAL] Failed to allocate memory for skybox mesh\n");
        lib3ds_file_free(file);
        exit(EXIT_FAILURE);
    }

    // Initialize the mesh with data from the 3DS file
    mesh->vertices = (float*)malloc(lib3ds_mesh->points * 3 * sizeof(float));
    mesh->numVertices = lib3ds_mesh->points;
    mesh->indices = (unsigned short*)malloc(lib3ds_mesh->faces * 3 * sizeof(unsigned short));
    mesh->numIndices = lib3ds_mesh->faces * 3;

    if (!mesh->vertices || !mesh->indices) {
        fprintf(stderr, "[FATAL] Failed to allocate memory for mesh vertices or indices\n");
        free(mesh->vertices);
        free(mesh->indices);
        free(mesh);
        lib3ds_file_free(file);
        exit(EXIT_FAILURE);
    }

    // Copy vertex data
    for (int i = 0; i < lib3ds_mesh->points; i++) {
        mesh->vertices[i * 3] = lib3ds_mesh->pointL[i].pos[0];
        mesh->vertices[i * 3 + 1] = lib3ds_mesh->pointL[i].pos[1];
        mesh->vertices[i * 3 + 2] = lib3ds_mesh->pointL[i].pos[2];
    }

    // Copy index data
    for (int i = 0; i < lib3ds_mesh->faces; i++) {
        mesh->indices[i * 3] = lib3ds_mesh->faceL[i].points[0];
        mesh->indices[i * 3 + 1] = lib3ds_mesh->faceL[i].points[1];
        mesh->indices[i * 3 + 2] = lib3ds_mesh->faceL[i].points[2];
    }

    // Initialize other members as needed
    mesh->pVB = NULL;
    mesh->pSI = NULL;
    mesh->ppIB = NULL;
    mesh->nMaterials = 0;
    mesh->ppMaterials = NULL;
    mesh->bIsBBoxValid = 0;
    mesh->skyboxMesh = NULL; // Assuming this is a placeholder

    // Free the lib3ds file
    lib3ds_file_free(file);

    return mesh;
}
