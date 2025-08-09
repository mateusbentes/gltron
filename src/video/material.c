#include "video/model.h"
#include "filesystem/path.h"
#include "video/nebu_renderer_gl.h"
#include <stdio.h>
#include <string.h>
#include "base/nebu_assert.h"
#include "zlib.h"
#include "base/nebu_debug_memory.h"

#define MAX_MATERIALS 200
#define BUF_SIZE 120

void loadDefaultMaterial(gltron_Mesh_Material *pMaterial) {
    float ambient[] = { 0.2f, 0.2f, 0.2f, 1.0 };
    float diffuse[] = { 1.0, 1.0, 0.0, 1.0 };
    float specular[] = { 1.0, 1.0, 1.0, 1.0 };
    float shininess = 2;

    memcpy(pMaterial->ambient, ambient, sizeof(ambient));
    memcpy(pMaterial->diffuse, diffuse, sizeof(diffuse));
    memcpy(pMaterial->specular, specular, sizeof(specular));
    pMaterial->shininess = shininess;
    pMaterial->flags = eMT_Ambient | eMT_Diffuse | eMT_Specular | eMT_Shininess;
    pMaterial->name = NULL;
}

void readMaterialLibraryFromFile(char *filename, gltron_Mesh *pMesh) {
    gltron_Mesh_Material *pMaterials;
    int iMaterial;
    char buf[BUF_SIZE];
    char name[BUF_SIZE];
    char *path;
    gzFile f;
    int i;

    path = getPath(PATH_DATA, filename);
    if (path == NULL) {
        fprintf(stderr, "** could not obtain path to file '%s'\n", filename);
        return;
    }

    if ((f = gzopen(path, "r")) == 0) {
        fprintf(stderr, "** could not open file '%s'\n", filename);
        free(path);
        return;
    }
    free(path);

    pMaterials = malloc(MAX_MATERIALS * sizeof(gltron_Mesh_Material));
    iMaterial = -1;

    while (gzgets(f, buf, sizeof(buf))) {
        switch (buf[0]) {
        case '#': break;
        case 'n':
            iMaterial++;
            if (iMaterial >= MAX_MATERIALS) {
                fprintf(stderr, "Overflowing material buffer\n");
                nebu_assert(0); exit(1);
            }
            loadDefaultMaterial(pMaterials + iMaterial);
            sscanf(buf, "newmtl %s ", name);
            pMaterials[iMaterial].name = malloc(strlen(name) + 1);
            strncpy(pMaterials[iMaterial].name, name, strlen(name) + 1);
            break;
        default:
            nebu_assert(iMaterial >= 0);
            if (sscanf(buf, " Ka %f %f %f ",
                pMaterials[iMaterial].ambient + 0,
                pMaterials[iMaterial].ambient + 1,
                pMaterials[iMaterial].ambient + 2) == 3) {
                // nothing
            } else if (sscanf(buf, " Kd %f %f %f ",
                pMaterials[iMaterial].diffuse + 0,
                pMaterials[iMaterial].diffuse + 1,
                pMaterials[iMaterial].diffuse + 2) == 3) {
                // nothing
            } else if (sscanf(buf, " Ks %f %f %f ",
                pMaterials[iMaterial].specular + 0,
                pMaterials[iMaterial].specular + 1,
                pMaterials[iMaterial].specular + 2) == 3) {
                // nothing
            } else if (sscanf(buf, " Ns %f ",
                &(pMaterials[iMaterial].shininess)) == 1) {
                // nothing
            } else {
                // Unparsed property
            }
        }
    }

    pMesh->nMaterials = iMaterial + 1;
    pMesh->ppMaterials = (gltron_Mesh_Material**)
        malloc(pMesh->nMaterials * sizeof(gltron_Mesh_Material*));

    for (i = 0; i < pMesh->nMaterials; i++) {
        pMesh->ppMaterials[i] = gltron_Mesh_Material_Create(pMaterials[i].name, 0);
        gltron_Mesh_Material_Copy(pMesh->ppMaterials[i], pMaterials + i);
        free(pMaterials[i].name);
    }
    free(pMaterials);
}

void readMaterialLibrary(char *buf, gltron_Mesh *pMesh) {
    char filename[BUF_SIZE];
    if (sscanf(buf, " mtllib %s ", filename) != 1) {
        fprintf(stderr, "*** failing parsing filename from %s\n", buf);
        return;
    }
    readMaterialLibraryFromFile(filename, pMesh);
}

void setMaterial(char *buf, gltron_Mesh *pMesh, int *iGroup) {
    char name[BUF_SIZE];
    int i;
    if (sscanf(buf, " usemtl %s ", name) == 1) {
        for (i = 0; i < pMesh->nMaterials; i++) {
            if (strstr(pMesh->ppMaterials[i]->name, name) == pMesh->ppMaterials[i]->name) {
                *iGroup = i;
                return;
            }
        }
        fprintf(stderr, "*** error: material %s not found\n", name);
    } else {
        fprintf(stderr, "*** error: can't parse material line %s\n", buf);
    }
}

void gltron_Mesh_Material_SetAlpha(gltron_Mesh *pMesh, float fAlpha) {
    int i;
    for (i = 0; i < pMesh->nMaterials; i++) {
        pMesh->ppMaterials[i]->diffuse[3] = fAlpha;
    }
}

void gltron_Mesh_Material_SetColor(gltron_Mesh *pMesh, char *name, ColorType eType, float pColor[4]) {
    int i;
    for (i = 0; i < pMesh->nMaterials; i++) {
        if (name == NULL || strstr(pMesh->ppMaterials[i]->name, name) == pMesh->ppMaterials[i]->name) {
            switch (eType) {
            case eAmbient:
                memcpy(pMesh->ppMaterials[i]->ambient, pColor, sizeof(float[4]));
                break;
            case eDiffuse:
                memcpy(pMesh->ppMaterials[i]->diffuse, pColor, sizeof(float[4]));
                break;
            case eSpecular:
                memcpy(pMesh->ppMaterials[i]->specular, pColor, sizeof(float[4]));
                break;
            }
        }
    }
}

// Modernized: Instead of glMaterial*, upload material properties to shader uniforms
void gltron_Mesh_Material_ApplyToShader(gltron_Mesh_Material *pMat, GLuint shaderProgram) {
    // Uniform names must match those in your shader
    GLint locAmbient = glGetUniformLocation(shaderProgram, "uMaterialAmbient");
    GLint locDiffuse = glGetUniformLocation(shaderProgram, "uMaterialDiffuse");
    GLint locSpecular = glGetUniformLocation(shaderProgram, "uMaterialSpecular");
    GLint locShininess = glGetUniformLocation(shaderProgram, "uMaterialShininess");

    if (locAmbient >= 0)
        glUniform4fv(locAmbient, 1, pMat->ambient);
    if (locDiffuse >= 0)
        glUniform4fv(locDiffuse, 1, pMat->diffuse);
    if (locSpecular >= 0)
        glUniform4fv(locSpecular, 1, pMat->specular);
    if (locShininess >= 0)
        glUniform1f(locShininess, pMat->shininess);
}
