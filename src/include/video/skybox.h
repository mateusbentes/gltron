// skybox.h
#ifndef SKYBOX_H
#define SKYBOX_H

#include "video/model.h" // gltron_Mesh

typedef struct {
    gltron_Mesh *skyboxMesh;
    // Other Skybox members...
} Skybox;

void enableSkyboxTexture(void);
void disableSkyboxTexture(void);
void drawSkybox(Skybox *skybox);

#endif /* SKYBOX_H */
