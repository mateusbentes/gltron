#include "gltron.h"
#ifdef ANDROID
#include "shaders.h"
#include <GLES2/gl2.h>
#endif

void drawMeshPart(MeshPart* meshpart, int flag) {
  int i, j;
  int type, c;
  float *normal;
  float *vertex;

  for(i = 0; i < meshpart->nFaces; i++) {
    c = *(meshpart->facesizes + i);
    type = 0;

#ifdef ANDROID
    // OpenGL ES doesn't support GL_POLYGON, so we'll use GL_TRIANGLE_FAN instead
    if(c > 4) type = GL_TRIANGLE_FAN;
    if(c == 4) type = GL_TRIANGLE_FAN;  // Use triangle fan for quads
    if(c == 3) type = GL_TRIANGLES;
    if(flag & 1) type = GL_LINE_LOOP;
#else
    if(c > 4) type = GL_POLYGON;
    if(c == 4) type = GL_QUADS;
    if(c == 3) type = GL_TRIANGLES;
    if(flag & 1) type = GL_LINE_LOOP;
#endif

    if(type != 0) {
#ifdef ANDROID
      // For Android, we need to use vertex arrays or VBOs
      // This is a simplified version - you might want to use VBOs for better performance
      GLfloat vertices[MODEL_FACESIZE * 3];
      GLfloat normals[MODEL_FACESIZE * 3];

      for(j = 0; j < c; j++) {
        normal = meshpart->normals + 3 * (i * MODEL_FACESIZE + j);
        vertex = meshpart->vertices + 3 * (i * MODEL_FACESIZE + j);

        normals[j*3] = normal[0];
        normals[j*3+1] = normal[1];
        normals[j*3+2] = normal[2];

        vertices[j*3] = vertex[0];
        vertices[j*3+1] = vertex[1];
        vertices[j*3+2] = vertex[2];
      }

      // Set up vertex attributes
      glEnableVertexAttribArray(0);  // Position
      glEnableVertexAttribArray(1);  // Normal

      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vertices);
      glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, normals);

      // Draw the geometry
      if(type == GL_TRIANGLE_FAN) {
        glDrawArrays(GL_TRIANGLE_FAN, 0, c);
      } else {
        glDrawArrays(type, 0, c);
      }

      // Disable attributes
      glDisableVertexAttribArray(0);
      glDisableVertexAttribArray(1);
#else
      // Desktop OpenGL - use immediate mode
      glBegin(type);
      for(j = 0; j < c; j++) {
        normal = meshpart->normals + 3 * (i * MODEL_FACESIZE + j);
        glNormal3fv(normal);
        vertex = meshpart->vertices + 3 * (i * MODEL_FACESIZE + j);
        glVertex3fv(vertex);
      }
      glEnd();
#endif
    }
  }
}

void drawExplosionPart(MeshPart* meshpart, float radius, int flag) {
  int i, j;
  int type, c;
  float *normal;
  float *vertex;

#define EXP_VECTORS 10
  float vectors[][3] = {
    { 0.03, -0.06, -0.07 },
    { 0.04, 0.08, -0.03 },
    { 0.10, -0.04, -0.07 },
    { 0.06, -0.09, -0.10 },
    { -0.03, -0.05, 0.02 },
    { 0.07, 0.08, -0.00 },
    { 0.01, -0.04, 0.10 },
    { -0.01, -0.07, 0.09 },
    { 0.01, -0.01, -0.09 },
    { -0.04, 0.04, 0.02 }
  };

  for(i = 0; i < meshpart->nFaces; i++) {
    c = *(meshpart->facesizes + i);
    type = 0;

#ifdef ANDROID
    if(c > 4) type = GL_TRIANGLE_FAN;
    if(c == 4) type = GL_TRIANGLE_FAN;
    if(c == 3) type = GL_TRIANGLES;
    if(flag & 1) type = GL_LINE_LOOP;
#else
    if(c > 4) type = GL_POLYGON;
    if(c == 4) type = GL_QUADS;
    if(c == 3) type = GL_TRIANGLES;
    if(flag & 1) type = GL_LINE_LOOP;
#endif

    if(type != 0) {
#ifdef ANDROID
      // For Android, we need to use vertex arrays or VBOs
      GLfloat vertices[MODEL_FACESIZE * 3];
      GLfloat normals[MODEL_FACESIZE * 3];

      // Calculate translation based on normal and explosion vector
      normal = meshpart->normals + 3 * (i * MODEL_FACESIZE);
      float tx = radius * (normal[0] + vectors[i % EXP_VECTORS][0]);
      float ty = radius * (normal[1] + vectors[i % EXP_VECTORS][1]);
      float tz = radius * (normal[2] + vectors[i % EXP_VECTORS][2]);

      // Set up model matrix with translation
      GLfloat modelMatrix[16] = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        tx, ty, tz, 1
      };

      // Set the model matrix in the shader
      extern GLuint shaderProgram; // Assuming this is defined elsewhere
      GLint modelMatrixLoc = glGetUniformLocation(shaderProgram, "modelMatrix");
      glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, modelMatrix);

      // Prepare vertex data
      for(j = 0; j < c; j++) {
        normal = meshpart->normals + 3 * (i * MODEL_FACESIZE + j);
        vertex = meshpart->vertices + 3 * (i * MODEL_FACESIZE + j);

        normals[j*3] = normal[0];
        normals[j*3+1] = normal[1];
        normals[j*3+2] = normal[2];

        vertices[j*3] = vertex[0];
        vertices[j*3+1] = vertex[1];
        vertices[j*3+2] = vertex[2];
      }

      // Set up vertex attributes
      glEnableVertexAttribArray(0);  // Position
      glEnableVertexAttribArray(1);  // Normal

      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vertices);
      glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, normals);

      // Draw the geometry
      if(type == GL_TRIANGLE_FAN) {
        glDrawArrays(GL_TRIANGLE_FAN, 0, c);
      } else {
        glDrawArrays(type, 0, c);
      }

      // Disable attributes
      glDisableVertexAttribArray(0);
      glDisableVertexAttribArray(1);

      // Reset model matrix
      GLfloat identityMatrix[16] = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
      };
      glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, identityMatrix);
#else
      // Desktop OpenGL - use immediate mode
      glPushMatrix();
      normal = meshpart->normals + 3 * (i * MODEL_FACESIZE);
      glTranslatef(radius * (normal[0] + vectors[i % EXP_VECTORS][0]),
                   radius * (normal[1] + vectors[i % EXP_VECTORS][1]),
                   radius * (normal[2] + vectors[i % EXP_VECTORS][2]));

      glBegin(type);
      for(j = 0; j < c; j++) {
        normal = meshpart->normals + 3 * (i * MODEL_FACESIZE + j);
        glNormal3fv(normal);
        vertex = meshpart->vertices + 3 * (i * MODEL_FACESIZE + j);
        glVertex3fv(vertex);
      }
      glEnd();
      glPopMatrix();
#endif
    }
  }
}

void printColor(float *values, int count) {
  int i;
  printf("color: ");
  for(i = 0; i < count; i++)
    printf("%.2f", *(values + i));
  printf("\n");
}

void drawModel(Mesh *mesh, int mode, int flag) {
  int i;

  for(i = 0; i < mesh->nMaterials; i++) {
    /* set materials */
    if(mode & MODEL_USE_MATERIAL) {
#ifdef ANDROID
      // For Android, we need to set material properties in the shader
      extern GLuint shaderProgram; // Assuming this is defined elsewhere

      // Set ambient color
      GLint ambientLoc = glGetUniformLocation(shaderProgram, "material.ambient");
      glUniform4fv(ambientLoc, 1, (mesh->materials + i)->ambient);

      // Set diffuse color
      GLint diffuseLoc = glGetUniformLocation(shaderProgram, "material.diffuse");
      glUniform4fv(diffuseLoc, 1, (mesh->materials + i)->diffuse);

      // Set specular color
      GLint specularLoc = glGetUniformLocation(shaderProgram, "material.specular");
      glUniform4fv(specularLoc, 1, (mesh->materials + i)->specular);
#else
      // Desktop OpenGL - use immediate mode
      glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,
                   (mesh->materials + i)->ambient);

      glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,
                   (mesh->materials + i)->diffuse);

      glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,
                   (mesh->materials + i)->specular);
#endif
    }
    drawMeshPart(mesh->meshparts + i, flag);
  }
}

void drawExplosion(Mesh *mesh, float radius, int mode, int flag) {
  int i;
  for (i = 0; i < mesh->nMaterials; i++) {
#ifdef ANDROID
    GLuint shaderProgram = shader_get_basic();
    if (!shaderProgram) continue;
    useShaderProgram(shaderProgram);

    // Set per-material colors as uniforms if present in shader
    GLint ambientLoc = glGetUniformLocation(shaderProgram, "material.ambient");
    if (ambientLoc >= 0) glUniform4fv(ambientLoc, 1, (mesh->materials + i)->ambient);
    GLint diffuseLoc = glGetUniformLocation(shaderProgram, "material.diffuse");
    if (diffuseLoc >= 0) glUniform4fv(diffuseLoc, 1, (mesh->materials + i)->diffuse);
    GLint specularLoc = glGetUniformLocation(shaderProgram, "material.specular");
    if (specularLoc >= 0) glUniform4fv(specularLoc, 1, (mesh->materials + i)->specular);
#else
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,  (mesh->materials + i)->ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,  (mesh->materials + i)->diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, (mesh->materials + i)->specular);
#endif
    drawExplosionPart(mesh->meshparts + i, radius, flag);
  }
}
