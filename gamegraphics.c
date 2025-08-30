#include "gltron.h"
#include "geom.h"
#include <string.h>
#include <math.h>
#ifdef ANDROID
#include <android/log.h>
#endif
#include <math.h>
#define M_PI 3.14159265358979323846

#ifdef ANDROID
#include <GLES2/gl2.h>
#include "shaders.h"
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

// Define LINE_D constant
#ifndef LINE_D
#define LINE_D 0.05
#endif

// Define constants for Android
#ifndef GL_LIGHTING
#define GL_LIGHTING 0x0B50
#endif

#ifndef GL_SMOOTH
#define GL_SMOOTH 0x1D01
#endif

#ifndef GL_FLAT
#define GL_FLAT 0x1D00
#endif

#ifndef GL_QUADS
#define GL_QUADS 0x0007
#endif

#ifndef GL_MODELVIEW_MATRIX
#define GL_MODELVIEW_MATRIX 0x0BA6
#endif

#ifndef GL_PROJECTION
#define GL_PROJECTION 0x1701
#endif

#ifndef GL_MODELVIEW
#define GL_MODELVIEW 0x1700
#endif

#ifndef GL_FOG
#define GL_FOG 0x0B60
#endif

#ifndef GL_LIGHT0
#define GL_LIGHT0 0x4000
#endif

#ifndef GL_POSITION
#define GL_POSITION 0x1203
#endif

// Lighting parameters
#define AMBIENT_LIGHT_R 0.2f
#define AMBIENT_LIGHT_G 0.2f
#define AMBIENT_LIGHT_B 0.2f

#define LIGHT_COLOR_R 0.8f
#define LIGHT_COLOR_G 0.8f
#define LIGHT_COLOR_B 0.8f

#define LIGHT_POS_X 5.0f
#define LIGHT_POS_Y 5.0f
#define LIGHT_POS_Z 10.0f

// Define functions for Android
#ifdef ANDROID
void multiplyMatrices(const GLfloat* a, const GLfloat* b, GLfloat* result) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            result[i * 4 + j] = 0.0f;
            for (int k = 0; k < 4; k++) {
                result[i * 4 + j] += a[i * 4 + k] * b[k * 4 + j];
            }
        }
    }
}

void createRotationMatrix(GLfloat* matrix, float angle, float x, float y, float z) {
    float c = cos(angle * M_PI / 180.0f);
    float s = sin(angle * M_PI / 180.0f);
    float t = 1.0f - c;

    // Normalize the axis
    float length = sqrt(x * x + y * y + z * z);
    if (length > 0.0f) {
        x /= length;
        y /= length;
        z /= length;
    }

    // Create the rotation matrix
    matrix[0] = t * x * x + c;
    matrix[1] = t * x * y - s * z;
    matrix[2] = t * x * z + s * y;
    matrix[3] = 0.0f;

    matrix[4] = t * x * y + s * z;
    matrix[5] = t * y * y + c;
    matrix[6] = t * y * z - s * x;
    matrix[7] = 0.0f;

    matrix[8] = t * x * z - s * y;
    matrix[9] = t * y * z + s * x;
    matrix[10] = t * z * z + c;
    matrix[11] = 0.0f;

    matrix[12] = 0.0f;
    matrix[13] = 0.0f;
    matrix[14] = 0.0f;
    matrix[15] = 1.0f;
}
#endif

// Define neigung constant
#ifndef neigung
#define neigung 25
#endif

void drawDebugTex(gDisplay *d) {
  int x = 100;
  int y = 100;

  rasonly(d);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

#ifdef ANDROID
  // For Android, we'll need to implement a different approach for drawing debug texture
  // This is a simplified version - you might need to implement a proper texture rendering method
  GLfloat vertices[] = {
    x, y, 0.0f, 0.0f,
    x + colwidth * 8, y, 0.0f, 1.0f,
    x + colwidth * 8, y + GSIZE, 0.0f, 1.0f,
    x, y + GSIZE, 0.0f, 0.0f
  };

  GLushort indices[] = {0, 1, 2, 0, 2, 3};

  // Create and bind vertex buffer
  GLuint vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  // Create and bind index buffer
  GLuint ibo;
  glGenBuffers(1, &ibo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

  // Use shader program
  GLuint shaderProgram = shader_get_basic();
  if (!shaderProgram) {
    init_shaders_android();
    shaderProgram = shader_get_basic();
    if (!shaderProgram) return;
  }
  useShaderProgram(shaderProgram);
  
  // Set identity model matrix
  setIdentityMatrix(shaderProgram, MATRIX_MODEL);

  // Set up attributes
  GLint positionLoc = glGetAttribLocation(shaderProgram, "position");
  GLint texCoordLoc = glGetAttribLocation(shaderProgram, "texCoord");

  glEnableVertexAttribArray(positionLoc);
  glVertexAttribPointer(positionLoc, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);

  glEnableVertexAttribArray(texCoordLoc);
  glVertexAttribPointer(texCoordLoc, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));

  // Bind texture
  glActiveTexture(GL_TEXTURE0);
  if (game && game->screen && game->screen->texFloor > 0) {
    glBindTexture(GL_TEXTURE_2D, game->screen->texFloor);
  } else {
    // Fallback to a default texture if not available
    GLuint defaultTex;
    glGenTextures(1, &defaultTex);
    glBindTexture(GL_TEXTURE_2D, defaultTex);
    GLfloat defaultColor[] = {1.0f, 0.0f, 0.0f, 1.0f}; // Red color
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_FLOAT, defaultColor);
  }

  // Draw
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

  // Clean up
  glDisableVertexAttribArray(positionLoc);
  glDisableVertexAttribArray(texCoordLoc);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glDeleteBuffers(1, &vbo);
  glDeleteBuffers(1, &ibo);
#else
  // For desktop OpenGL
  glColor4f(.0, 1.0, .0, 1.0);
  glRasterPos2i(x, y);
  glBitmap(colwidth * 8, GSIZE, 0, 0, 0, 0, colmap);
  glBegin(GL_LINE_LOOP);
  glVertex2i(x - 1, y - 1);
  glVertex2i(x + colwidth * 8, y - 1);
  glVertex2i(x + colwidth * 8, y + GSIZE);
  glVertex2i(x - 1, y + GSIZE);
  glEnd();
  polycount++;
#endif
}

void drawScore(Player *p, gDisplay *d) {
  char tmp[10]; /* hey, they won't reach such a score */

  sprintf(tmp, "%d", p->data->score);
  rasonly(d);

#ifdef ANDROID
  // For Android, we'll need to implement a text rendering function
  // This is a placeholder - you'll need to implement proper text rendering
  // for Android using shaders and textures
  // drawTextAndroid(5, 5, 32, tmp);

  // Temporary workaround: Use a simple colored quad for score display
  GLfloat vertices[] = {
    5, 5, 0.0f,
    5 + 32, 5, 0.0f,
    5 + 32, 5 + 32, 0.0f,
    5, 5 + 32, 0.0f
  };

  // Create and bind vertex buffer
  GLuint vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  // Use shader program
  GLuint shaderProgram = shader_get_basic();
  if (!shaderProgram) {
    init_shaders_android();
    shaderProgram = shader_get_basic();
    if (!shaderProgram) return;
  }
  useShaderProgram(shaderProgram);
  
  // Set identity model matrix
  setIdentityMatrix(shaderProgram, MATRIX_MODEL);

  // Set up attributes
  GLint positionLoc = glGetAttribLocation(shaderProgram, "position");

  glEnableVertexAttribArray(positionLoc);
  glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);

  // Set color (yellow)
  GLint colorLoc = glGetUniformLocation(shaderProgram, "color");
  glUniform4f(colorLoc, 1.0f, 1.0f, 0.2f, 1.0f);

  // Draw
  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

  // Clean up
  glDisableVertexAttribArray(positionLoc);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glDeleteBuffers(1, &vbo); /* keep program bound */
#else
  // For desktop OpenGL
  glColor4f(1.0, 1.0, 0.2, 1.0);
  drawText(5, 5, 32, tmp);
#endif
}
  
void drawFloor(gDisplay *d) {
    int j, k, l, t;

    if(game->settings->show_floor_texture) {
#ifdef ANDROID
        // Android textured floor
        if (!game || !game->screen || game->screen->texFloor == 0) {
            return;
        }

        GLuint shaderProgram = shader_get_basic();
        if (!shaderProgram) return;
        
        useShaderProgram(shaderProgram);
        setIdentityMatrix(shaderProgram, MATRIX_MODEL);

        // Setup texture - try to bind and verify
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, game->screen->texFloor);
        
        // Check if texture is valid
        GLint texWidth, texHeight;
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &texWidth);
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &texHeight);
        
        if (texWidth == 0 || texHeight == 0) {
            // Texture is not valid, fall back to line floor
            game->settings->show_floor_texture = 0;
            drawFloor(d);
            game->settings->show_floor_texture = 1;
            return;
        }

        // Set texture parameters for proper rendering
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        // Set uniforms - use simpler approach
        setAmbientLight(shaderProgram, AMBIENT_LIGHT_R, AMBIENT_LIGHT_G, AMBIENT_LIGHT_B);
        setLightColor(shaderProgram, LIGHT_COLOR_R, LIGHT_COLOR_G, LIGHT_COLOR_B);
        setLightPosition(shaderProgram, LIGHT_POS_X, LIGHT_POS_Y, LIGHT_POS_Z);

        // Use immediate mode style vertex arrays for simplicity
        l = GSIZE / 4;
        t = 5;
        
        // Simple quad rendering
        for(j = 0; j < GSIZE; j += l) {
            for(k = 0; k < GSIZE; k += l) {
                GLfloat vertices[] = {
                    // Position (x,y,z)    Texture (u,v)
                    j,     k,     0.0f,   0.0f, 0.0f,
                    j + l, k,     0.0f,   t,    0.0f,
                    j + l, k + l, 0.0f,   t,    t,
                    j,     k + l, 0.0f,   0.0f, t
                };
                
                GLushort indices[] = {0, 1, 2, 0, 2, 3};

                GLuint vbo, ebo;
                glGenBuffers(1, &vbo);
                glGenBuffers(1, &ebo);

                glBindBuffer(GL_ARRAY_BUFFER, vbo);
                glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

                GLint posLoc = glGetAttribLocation(shaderProgram, "position");
                GLint texLoc = glGetAttribLocation(shaderProgram, "texCoord");
                GLint normalLoc = glGetAttribLocation(shaderProgram, "normal");

                if (posLoc >= 0) {
                    glEnableVertexAttribArray(posLoc);
                    glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)0);
                }

                if (texLoc >= 0) {
                    glEnableVertexAttribArray(texLoc);
                    glVertexAttribPointer(texLoc, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
                }

                if (normalLoc >= 0) {
                    glVertexAttrib3f(normalLoc, 0.0f, 0.0f, 1.0f);
                }

                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

                if (posLoc >= 0) glDisableVertexAttribArray(posLoc);
                if (texLoc >= 0) glDisableVertexAttribArray(texLoc);
                
                glDeleteBuffers(1, &vbo);
                glDeleteBuffers(1, &ebo);
                
                polycount++;
            }
        }

#else
        // Desktop textured floor
        if (!game || !game->screen || game->screen->texFloor == 0) {
            return;
        }

        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, game->screen->texFloor);
        
        // Verify texture is loaded
        GLint texWidth;
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &texWidth);
        
        if (texWidth == 0) {
            glDisable(GL_TEXTURE_2D);
            // Fall back to line floor
            game->settings->show_floor_texture = 0;
            drawFloor(d);
            game->settings->show_floor_texture = 1;
            return;
        }

        // Set texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glColor4f(1.0, 1.0, 1.0, 1.0);
        
        l = GSIZE / 4;
        t = 5;
        
        for(j = 0; j < GSIZE; j += l) {
            for(k = 0; k < GSIZE; k += l) {
                glBegin(GL_QUADS);
                glNormal3f(0.0f, 0.0f, 1.0f);
                glTexCoord2f(0.0f, 0.0f); glVertex3f(j, k, 0.0f);
                glTexCoord2f(t, 0.0f);    glVertex3f(j + l, k, 0.0f);
                glTexCoord2f(t, t);       glVertex3f(j + l, k + l, 0.0f);
                glTexCoord2f(0.0f, t);    glVertex3f(j, k + l, 0.0f);
                glEnd();
                polycount++;
            }
        }
        
        glDisable(GL_TEXTURE_2D);
#endif
        
    } else {
        // Line floor
#ifdef ANDROID
        GLuint shaderProgram = shader_get_basic();
        if (!shaderProgram) return;
        
        useShaderProgram(shaderProgram);
        setIdentityMatrix(shaderProgram, MATRIX_MODEL);

        setAmbientLight(shaderProgram, AMBIENT_LIGHT_R, AMBIENT_LIGHT_G, AMBIENT_LIGHT_B);
        setLightColor(shaderProgram, LIGHT_COLOR_R, LIGHT_COLOR_G, LIGHT_COLOR_B);
        setLightPosition(shaderProgram, LIGHT_POS_X, LIGHT_POS_Y, LIGHT_POS_Z);

        // Create line vertices
        int lineCount = 0;
        for(j = 0; j <= GSIZE; j += game->settings->line_spacing) {
            lineCount += 2;
        }

        // Create vertex data
        GLfloat *vertices = malloc(lineCount * 2 * 3 * sizeof(GLfloat));
        int vertexIndex = 0;

        for(j = 0; j <= GSIZE; j += game->settings->line_spacing) {
            // Horizontal line
            vertices[vertexIndex++] = 0;     vertices[vertexIndex++] = j;     vertices[vertexIndex++] = 0;
            vertices[vertexIndex++] = GSIZE; vertices[vertexIndex++] = j;     vertices[vertexIndex++] = 0;

            // Vertical line
            vertices[vertexIndex++] = j;     vertices[vertexIndex++] = 0;     vertices[vertexIndex++] = 0;
            vertices[vertexIndex++] = j;     vertices[vertexIndex++] = GSIZE; vertices[vertexIndex++] = 0;

            polycount += 2;
        }

        // Create buffer
        GLuint vbo;
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, lineCount * 2 * 3 * sizeof(GLfloat), vertices, GL_STATIC_DRAW);

        GLint posLoc = glGetAttribLocation(shaderProgram, "position");
        GLint normalLoc = glGetAttribLocation(shaderProgram, "normal");

        if (posLoc >= 0) {
            glEnableVertexAttribArray(posLoc);
            glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
        }

        if (normalLoc >= 0) {
            glVertexAttrib3f(normalLoc, 0.0f, 0.0f, 1.0f);
        }

        // Set blue color for lines
        GLint colorLoc = glGetUniformLocation(shaderProgram, "u_color");
        if (colorLoc < 0) colorLoc = glGetUniformLocation(shaderProgram, "color");
        if (colorLoc >= 0) {
            glUniform4f(colorLoc, 0.0f, 0.0f, 1.0f, 1.0f);
        }

        // Draw lines
        glDrawArrays(GL_LINES, 0, lineCount * 2);

        if (posLoc >= 0) glDisableVertexAttribArray(posLoc);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDeleteBuffers(1, &vbo);
        free(vertices);

#else
        // Desktop line floor
        glColor3f(0.0, 0.0, 1.0);
        glBegin(GL_LINES);
        for(j = 0; j <= GSIZE; j += game->settings->line_spacing) {
            glVertex3f(0, j, 0);
            glVertex3f(GSIZE, j, 0);
            glVertex3f(j, 0, 0);
            glVertex3f(j, GSIZE, 0);
            polycount += 2;
        }
        glEnd();
#endif
    }
}

void drawTraces(Player *p, gDisplay *d, int instance) {
  line *line;
  float height;

  Data *data;
  data = p->data;
  height = data->trail_height;
  if(height > 0) {
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

#ifdef ANDROID
    // For Android, use vertex buffers for better performance
    GLfloat vertices[(data->trail - &(data->trails[0]) + 1) * 2 * 3]; // 2 vertices per line segment, 3 floats per vertex
    int vertexCount = 0;

    line = &(data->trails[0]);
    vertices[vertexCount++] = line->sx;
    vertices[vertexCount++] = line->sy;
    vertices[vertexCount++] = 0.0f;

    vertices[vertexCount++] = line->sx;
    vertices[vertexCount++] = line->sy;
    vertices[vertexCount++] = height;

    while(line != data->trail) {
      vertices[vertexCount++] = line->ex;
      vertices[vertexCount++] = line->ey;
      vertices[vertexCount++] = 0.0f;

      vertices[vertexCount++] = line->ex;
      vertices[vertexCount++] = line->ey;
      vertices[vertexCount++] = height;

      line++;
      polycount++;
    }

    vertices[vertexCount++] = line->ex;
    vertices[vertexCount++] = line->ey;
    vertices[vertexCount++] = 0.0f;

    vertices[vertexCount++] = line->ex;
    vertices[vertexCount++] = line->ey;
    vertices[vertexCount++] = height;

    // Create and bind vertex buffer
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Use shader program
    GLuint shaderProgram = shader_get_basic();
    if (!shaderProgram) {
      init_shaders_android();
      shaderProgram = shader_get_basic();
      if (!shaderProgram) return;
    }
    useShaderProgram(shaderProgram);
    
    // Set identity model matrix for floor lines
    setIdentityMatrix(shaderProgram, MATRIX_MODEL);

    // Set up attributes
    GLint positionLoc = glGetAttribLocation(shaderProgram, "position");

    glEnableVertexAttribArray(positionLoc);
    glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);

    // Draw
    glDrawArrays(GL_TRIANGLE_STRIP, 0, vertexCount / 3);

    // Clean up
    glDisableVertexAttribArray(positionLoc);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &vbo);

    if(game->settings->camType == 1) {
      GLfloat quadVertices[] = {
        data->trail->sx - LINE_D, data->trail->sy - LINE_D, 0.0f,
        data->trail->sx + LINE_D, data->trail->sy + LINE_D, 0.0f,
        data->trail->ex + LINE_D, data->trail->ey + LINE_D, 0.0f,
        data->trail->ex - LINE_D, data->trail->ey - LINE_D, 0.0f
      };

      // Create and bind vertex buffer
      GLuint quadVbo;
      glGenBuffers(1, &quadVbo);
      glBindBuffer(GL_ARRAY_BUFFER, quadVbo);
      glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

      // Use shader program
      useShaderProgram(shaderProgram);

      // Set up attributes
      glEnableVertexAttribArray(positionLoc);
      glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);

      // Draw
      glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

      // Clean up
      glDisableVertexAttribArray(positionLoc);
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glDeleteBuffers(1, &quadVbo);

      polycount++;
    }
#else
    // For desktop OpenGL
    glColor4fv(p->model->color_alpha);
    line = &(data->trails[0]);
    glBegin(GL_TRIANGLE_STRIP);
    glVertex3f(line->sx, line->sy, 0.0);
    glVertex3f(line->sx, line->sy, height);
    while(line != data->trail) {
      glVertex3f(line->ex, line->ey, 0.0);
      glVertex3f(line->ex, line->ey, height);
      line++;
      polycount++;
    }
    glVertex3f(line->ex, line->ey, 0.0);
    glVertex3f(line->ex, line->ey, height);
    polycount += 2;
    glEnd();

    if(game->settings->camType == 1) {
      //       glLineWidth(3);
      // glBegin(GL_LINES);
      glBegin(GL_QUADS);
      glVertex2f(data->trail->sx - LINE_D, data->trail->sy - LINE_D);
      glVertex2f(data->trail->sx + LINE_D, data->trail->sy + LINE_D);
      glVertex2f(data->trail->ex + LINE_D, data->trail->ey + LINE_D);
      glVertex2f(data->trail->ex - LINE_D, data->trail->ey - LINE_D);

      glEnd();
      // glLineWidth(1);
      polycount++;
    }
#endif

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  }
}

void drawCrash(float radius) {
#define CRASH_W 20
#ifdef ANDROID
  // For Android, use vertex buffers for better performance
  GLfloat vertices[] = {
    -CRASH_W, 0.0f, 0.0f, 0.0f, 0.0f,
    CRASH_W, 0.0f, 0.0f, 1.0f, 0.0f,
    CRASH_W, 0.0f, CRASH_W, 1.0f, 0.5f,
    -CRASH_W, 0.0f, CRASH_W, 0.0f, 0.5f
  };

  GLushort indices[] = {0, 1, 2, 0, 2, 3};

  // Create and bind vertex buffer
  GLuint vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  // Create and bind index buffer
  GLuint ibo;
  glGenBuffers(1, &ibo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

  // Use shader program
  GLuint shaderProgram = shader_get_basic();
  if (!shaderProgram) {
    init_shaders_android();
    shaderProgram = shader_get_basic();
    if (!shaderProgram) return;
  }
  useShaderProgram(shaderProgram);
  
  // Set identity model matrix
  setIdentityMatrix(shaderProgram, MATRIX_MODEL);

  // Set up attributes
  GLint positionLoc = glGetAttribLocation(shaderProgram, "position");
  GLint texCoordLoc = glGetAttribLocation(shaderProgram, "texCoord");

  if (positionLoc >= 0) {
    glEnableVertexAttribArray(positionLoc);
    glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), 0);
  }

  if (texCoordLoc >= 0) {
    glEnableVertexAttribArray(texCoordLoc);
    glVertexAttribPointer(texCoordLoc, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
  }

  // Set color with alpha
  GLint colorLoc = glGetUniformLocation(shaderProgram, "color");
  glUniform4f(colorLoc, 1.0f, 1.0f, 1.0f, (EXP_RADIUS_MAX - radius) / EXP_RADIUS_MAX);

  // Bind texture
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, game->screen->texCrash);

  // Draw
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

  // Clean up
  glDisableVertexAttribArray(positionLoc);
  glDisableVertexAttribArray(texCoordLoc);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glDeleteBuffers(1, &vbo);
  glDeleteBuffers(1, &ibo);
#else
  // For desktop OpenGL
  glColor4f(1.0, 1.0, 1.0, (EXP_RADIUS_MAX - radius) / EXP_RADIUS_MAX);
  /* printf("exp_r: %.2f\n", (EXP_RADIUS_MAX - radius) / EXP_RADIUS_MAX); */
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, game->screen->texCrash);
  glEnable(GL_BLEND);
  glBegin(GL_QUADS);
  glTexCoord2f(0.0, 0.0);
  glVertex3f(-CRASH_W, 0.0, 0.0);
  glTexCoord2f(1.0, 0.0);
  glVertex3f(CRASH_W, 0.0, 0.0);
  glTexCoord2f(1.0, 0.5);
  glVertex3f(CRASH_W, 0.0, CRASH_W);
  glTexCoord2f(0.0, 0.5);
  glVertex3f(-CRASH_W, 0.0, CRASH_W);
  glEnd();
  glDisable(GL_TEXTURE_2D);
  if(game->settings->show_alpha == 0) glDisable(GL_BLEND);
#endif
}

void drawCycle(Player *p) {
  float dirangles[] = { 180, 90, 0, 270 , 360, -90 };
  int time;
  int last_dir;
  float dirangle;
  Mesh *cycle;

#define turn_length 500

#ifdef ANDROID
  if (!p || !p->model || !p->model->mesh) {
    return;
  }
  cycle = p->model->mesh;
#else
  cycle = p->model->mesh;
#endif

#ifdef ANDROID
  // For Android, use matrix operations with shaders
  GLuint prog = shader_get_basic();
  if (!prog) {
    init_shaders_android();
    prog = shader_get_basic();
    if (!prog) return;
  }
  useShaderProgram(prog);

  // Build model matrix from identity (no fixed-function in GLES2)
  GLfloat modelMatrix[16] = {
    1,0,0,0,
    0,1,0,0,
    0,0,1,0,
    0,0,0,1
  };

  GLfloat translationMatrix[16] = {
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    p->data->posx, p->data->posy, 0, 1
  };
  multiplyMatrices(modelMatrix, translationMatrix, modelMatrix);

  if(game->settings->turn_cycle) {
    time = abs(p->data->turn_time - getElapsedTime());
    if(time < turn_length) {
      last_dir = p->data->last_dir;
      if(p->data->dir == 3 && last_dir == 2) last_dir = 4;
      if(p->data->dir == 2 && last_dir == 3) last_dir = 5;
      dirangle = ((turn_length - time) * dirangles[last_dir] + time * dirangles[p->data->dir]) / turn_length;
    } else dirangle = dirangles[p->data->dir];
  } else {
    dirangle = dirangles[p->data->dir];
  }

  GLfloat rotationMatrix[16];
  createRotationMatrix(rotationMatrix, dirangle, 0, 0, 1);
  multiplyMatrices(modelMatrix, rotationMatrix, modelMatrix);

  if(game->settings->turn_cycle && time < turn_length) {
    float axis = 1.0;
    if(p->data->dir < p->data->last_dir && p->data->last_dir != 3) axis = -1.0;
    else if((p->data->last_dir == 3 && p->data->dir == 2) || (p->data->last_dir == 0 && p->data->dir == 3)) axis = -1.0;
    GLfloat tiltMatrix[16];
    createRotationMatrix(tiltMatrix, neigung * sin(M_PI * time / turn_length), 0, axis, 0);
    multiplyMatrices(modelMatrix, tiltMatrix, modelMatrix);
  }

  GLfloat finalTranslationMatrix[16] = {
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    -cycle->bbox[0] / 2, -cycle->bbox[1] / 2, 0, 1
  };
  multiplyMatrices(modelMatrix, finalTranslationMatrix, modelMatrix);

  // Set the model matrix in the shader
  setModelMatrix(prog, modelMatrix);

  // Enable lighting
  glEnable(GL_LIGHTING);
  glEnable(GL_DEPTH_TEST);
  glDepthMask(GL_TRUE);

  if(p->data->exp_radius == 0)
    drawModel(cycle, MODEL_USE_MATERIAL, 0);
  else if(p->data->exp_radius < EXP_RADIUS_MAX) {
    float alpha = (float)(EXP_RADIUS_MAX - p->data->exp_radius) / (float)EXP_RADIUS_MAX;
    setMaterialAlphas(cycle, alpha);
    drawExplosion(cycle, p->data->exp_radius, MODEL_USE_MATERIAL, 0);
  }

  // Disable lighting
  glDisable(GL_LIGHTING);
  glDisable(GL_DEPTH_TEST);
  glDepthMask(GL_FALSE);

  // Clean up
#else
  // For desktop OpenGL
  glPushMatrix();
  glTranslatef(p->data->posx, p->data->posy, .0);

  if(game->settings->turn_cycle) {
    time = abs(p->data->turn_time - getElapsedTime());
    if(time < turn_length) {
      last_dir = p->data->last_dir;
      if(p->data->dir == 3 && last_dir == 2)
        last_dir = 4;
      if(p->data->dir == 2 && last_dir == 3)
        last_dir = 5;
      dirangle = ((turn_length - time) * dirangles[last_dir] +
                  time * dirangles[p->data->dir]) / turn_length;
    } else
      dirangle = dirangles[p->data->dir];
  } else {
    dirangle = dirangles[p->data->dir];
  }

  glRotatef(dirangle, 0, 0.0, 1.0);

  if(game->settings->show_crash_texture)
    if(p->data->exp_radius > 0 && p->data->exp_radius < EXP_RADIUS_MAX)
      drawCrash(p->data->exp_radius);

  if(game->settings->turn_cycle) {
    if(time < turn_length) {
      float axis = 1.0;
      if(p->data->dir < p->data->last_dir && p->data->last_dir != 3)
        axis = -1.0;
      else if((p->data->last_dir == 3 && p->data->dir == 2) ||
              (p->data->last_dir == 0 && p->data->dir == 3))
        axis = -1.0;
      glRotatef(neigung * sin(M_PI * time / turn_length),
                0.0, axis, 0.0);
    }
  }

  glTranslatef(-cycle->bbox[0] / 2, -cycle->bbox[1] / 2, .0);
  /* glTranslatef(-cycle->bbox[0] / 2, 0, .0); */
  /* glTranslatef(-cycle->bbox[0] / 2, -cycle->bbox[1], .0); */

  // Enable lighting
  glEnable(GL_LIGHTING);
  glEnable(GL_DEPTH_TEST);
  glDepthMask(GL_TRUE);

  if(p->data->exp_radius == 0)
    drawModel(cycle, MODEL_USE_MATERIAL, 0);
  else if(p->data->exp_radius < EXP_RADIUS_MAX) {
    float alpha;
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    alpha = (float)(EXP_RADIUS_MAX - p->data->exp_radius) / (float)EXP_RADIUS_MAX;
    setMaterialAlphas(cycle, alpha);
    drawExplosion(cycle, p->data->exp_radius, MODEL_USE_MATERIAL, 0);
  }

  if(game->settings->show_alpha == 0) glDisable(GL_BLEND);

  // Disable lighting
  glDisable(GL_LIGHTING);
  glDisable(GL_DEPTH_TEST);
  glDepthMask(GL_FALSE);

  glPopMatrix();
#endif
}

int playerVisible(Player *eye, Player *target) {
  float v1[3];
  float v2[3];
  float tmp[3];
  float s;
  float d;

  vsub(eye->camera->target, eye->camera->cam, v1);
  normalize(v1);
  tmp[0] = target->data->posx;
  tmp[1] = target->data->posy;
  tmp[2] = 0;
  vsub(tmp, eye->camera->cam, v2);
  normalize(v2);
  s = scalarprod(v1, v2);
  /* maybe that's not exactly correct, but I didn't notice anything */
  d = cos((game->settings->fov / 2) * 2 * M_PI / 360.0);
  /*
  printf("v1: %.2f %.2f %.2f\nv2: %.2f %.2f %.2f\ns: %.2f d: %.2f\n\n",
	 v1[0], v1[1], v1[2], v2[0], v2[1], v2[2],
	 s, d);
  */
  if(s < d)
    return 0;
  else return 1;
}
	    
void drawPlayers(Player *p) {
  int i;
  int dir;
  float l = 5.0;
  float height;

#ifdef ANDROID
  // For Android, use vertex buffers for better performance
  GLuint shaderProgram = shader_get_basic();
  if (!shaderProgram) {
    init_shaders_android();
    shaderProgram = shader_get_basic();
    if (!shaderProgram) return;
  }
  useShaderProgram(shaderProgram);

  // Set up common shader parameters
  GLint colorLoc = glGetUniformLocation(shaderProgram, "color");

  glEnable(GL_BLEND);

  // Enable lighting
  glEnable(GL_LIGHTING);

  for(i = 0; i < game->players; i++) {
    // Validate model before any use
    if (!game->player[i].model || !game->player[i].model->mesh) {
      // Still allow visibility test and drawCycle check below to skip
    } else {
      height = game->player[i].data->trail_height;
      if(height > 0) {
        // Create model matrix for this player
        GLfloat modelMatrix[16] = {
          1, 0, 0, 0,
          0, 1, 0, 0,
          0, 0, 1, 0,
          game->player[i].data->posx, game->player[i].data->posy, 0, 1
        };
        setModelMatrix(shaderProgram, modelMatrix);

        // Set color
        glUniform4fv(colorLoc, 1, game->player[i].model->color_model);

        // Create quad vertices
        dir = game->player[i].data->dir;
        GLfloat quadVertices[] = {
          0, 0, 0,
          -dirsX[dir] * l, -dirsY[dir] * l, 0,
          -dirsX[dir] * l, -dirsY[dir] * l, height,
          0, 0, height
        };

        // Create and bind vertex buffer
        GLuint vbo;
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

        // Set up attributes
        GLint positionLoc = glGetAttribLocation(shaderProgram, "position");
        glEnableVertexAttribArray(positionLoc);
        glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);

        // Draw
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        // Clean up
        glDisableVertexAttribArray(positionLoc);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDeleteBuffers(1, &vbo);

        polycount++;
      }
    }

    if(playerVisible(p, &(game->player[i]))) {
      if(game->settings->show_model && game->player[i].model && game->player[i].model->mesh)
        drawCycle(&(game->player[i]));
    }
  }

  // Disable lighting
  glDisable(GL_LIGHTING);

  // Clean up
  if(game->settings->show_alpha != 1) glDisable(GL_BLEND);
#else
  // For desktop OpenGL
  glShadeModel(GL_SMOOTH);
  glEnable(GL_BLEND);

  // Enable lighting
  glEnable(GL_LIGHTING);

  for(i = 0; i < game->players; i++) {
    height = game->player[i].data->trail_height;
    if(height > 0) {
      glPushMatrix();
      glTranslatef(game->player[i].data->posx,
                   game->player[i].data->posy,
                   0);
      /* draw Quad */
      dir = game->player[i].data->dir;
      glColor3fv(game->player[i].model->color_model);
      glBegin(GL_QUADS);
      glVertex3f(0, 0, 0);
      glColor4f(0, 0, 0, 0);
      glVertex3f(-dirsX[dir] * l, -dirsY[dir] * l, 0);
      glVertex3f(-dirsX[dir] * l, -dirsY[dir] * l, height);
      glColor3fv(game->player[i].model->color_model);
      glVertex3f(0, 0, height);
      glEnd();
      polycount++;
      glPopMatrix();
    }
    if(playerVisible(p, &(game->player[i]))) {
      if(game->settings->show_model)
        drawCycle(&(game->player[i]));
    }
  }

  // Disable lighting
  glDisable(GL_LIGHTING);

  if(game->settings->show_alpha != 1) glDisable(GL_BLEND);
  glShadeModel(GL_FLAT);
#endif
}

void drawGlow(Player *p, gDisplay *d, float dim) {
#ifdef ANDROID
  // For Android, use vertex buffers for better performance
  GLuint shaderProgram = shader_get_basic();
  if (!shaderProgram) {
    init_shaders_android();
    shaderProgram = shader_get_basic();
    if (!shaderProgram) return;
  }
  useShaderProgram(shaderProgram);

  // Set up common shader parameters
  GLint colorLoc = glGetUniformLocation(shaderProgram, "color");

  // Create model matrix for this player
  GLfloat modelMatrix[16] = {
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    p->data->posx, p->data->posy, 0, 1
  };
  setModelMatrix(shaderProgram, modelMatrix);

  // Set up glow effect
  glBlendFunc(GL_ONE, GL_ONE);
  glEnable(GL_BLEND);

  // Create vertices for glow effect
  GLfloat vertices[24]; // 6 vertices for triangle fan + 3 for triangles
  int vertexCount = 0;

  // Triangle fan vertices
  vertices[vertexCount++] = 0;
  vertices[vertexCount++] = TRAIL_HEIGHT/2;
  vertices[vertexCount++] = 0;

  for(int i = 0; i <= 5; i++) {
    float angle = (i == 5) ? 5.2 * 3.1415 / 5.0 : i * 3.1415 / 5.0;
    vertices[vertexCount++] = dim * cos(angle);
    vertices[vertexCount++] = TRAIL_HEIGHT/2 + dim * sin(angle);
    vertices[vertexCount++] = 0;
  }

  // Create and bind vertex buffer
  GLuint vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  // Set up attributes
  GLint positionLoc = glGetAttribLocation(shaderProgram, "position");
  glEnableVertexAttribArray(positionLoc);
  glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);

  // Set color
  glUniform4fv(colorLoc, 1, p->model->color_model);

  // Draw triangle fan
  glDrawArrays(GL_TRIANGLE_FAN, 0, 7);

  // Draw additional triangles
  GLfloat triangleVertices[] = {
    0, TRAIL_HEIGHT/2, 0,
    0, -TRAIL_HEIGHT/4, 0,
    dim * cos(-0.2 * 3.1415 / 5.0), TRAIL_HEIGHT/2 + dim * sin(-0.2 * 3.1415 / 5.0), 0,

    0, TRAIL_HEIGHT/2, 0,
    dim * cos(5.2 * 3.1415 / 5.0), TRAIL_HEIGHT/2 + dim * sin(5.2 * 3.1415 / 5.0), 0,
    0, -TRAIL_HEIGHT/4, 0
  };

  // Update vertex buffer for triangles
  glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVertices), triangleVertices, GL_STATIC_DRAW);

  // Draw triangles
  glDrawArrays(GL_TRIANGLES, 0, 6);

  // Clean up
  glDisableVertexAttribArray(positionLoc);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glDeleteBuffers(1, &vbo);

  polycount += 8;

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  if(game->settings->show_alpha != 1) glDisable(GL_BLEND);
#else
  // For desktop OpenGL
  float mat[4*4];

  glPushMatrix();
  glTranslatef(p->data->posx,
               p->data->posy,
               0);

  glBlendFunc(GL_ONE, GL_ONE);
  glEnable(GL_BLEND);
  glGetFloatv(GL_MODELVIEW_MATRIX, mat);
  mat[0] = mat[5] = mat[10] = 1.0;
  mat[1] = mat[2] = 0.0;
  mat[4] = mat[6] = 0.0;
  mat[8] = mat[9] = 0.0;
  glLoadMatrixf(mat);
  glBegin(GL_TRIANGLE_FAN);
  glColor3fv(p->model->color_model);

  glVertex3f(0,TRAIL_HEIGHT/2, 0);
  glColor4f(0,0,0,0.0);
  glVertex3f(dim*cos(-0.2*3.1415/5.0),
             TRAIL_HEIGHT/2+dim*sin(-0.2*3.1415/5.0), 0);
  glVertex3f(dim*cos(1.0*3.1415/5.0),
             TRAIL_HEIGHT/2+dim*sin(1.0*3.1415/5.0), 0);
  glVertex3f(dim*cos(2.0*3.1415/5.0),
             TRAIL_HEIGHT/2+dim*sin(2.0*3.1415/5.0), 0);
  glVertex3f(dim*cos(3.0*3.1415/5.0),
             TRAIL_HEIGHT/2+dim*sin(3.0*3.1415/5.0), 0);
  glVertex3f(dim*cos(4.0*3.1415/5.0),
             TRAIL_HEIGHT/2+dim*sin(4.0*3.1415/5.0), 0);
  glVertex3f(dim*cos(5.2*3.1415/5.0),
             TRAIL_HEIGHT/2+dim*sin(5.2*3.1415/5.0), 0);
  glEnd();
  polycount += 5;

  glBegin(GL_TRIANGLES);
  glColor3fv(p->model->color_model);
  glVertex3f(0,TRAIL_HEIGHT/2, 0);
  glColor4f(0,0,0,0.0);
  glVertex3f(0,-TRAIL_HEIGHT/4,0);
  glVertex3f(dim*cos(-0.2*3.1415/5.0),
             TRAIL_HEIGHT/2+dim*sin(-0.2*3.1415/5.0), 0);

  glColor3fv(p->model->color_model);
  glVertex3f(0,TRAIL_HEIGHT/2, 0);
  glColor4f(0,0,0,0.0);
  glVertex3f(dim*cos(5.2*3.1415/5.0),
             TRAIL_HEIGHT/2+dim*sin(5.2*3.1415/5.0), 0);
  glVertex3f(0,-TRAIL_HEIGHT/4,0);
  glEnd();
  polycount += 3;

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  if(game->settings->show_alpha != 1) glDisable(GL_BLEND);
  glPopMatrix();
#endif
}

void drawWalls(gDisplay *d) {
  float t = 4;

#ifdef ANDROID
  // For Android, use vertex buffers for better performance
  GLfloat vertices[] = {
    // First quad
    0.0f, 0.0f, 0.0f, t, 0.0f,
    0.0f, 0.0f, WALL_H, t, 1.0f,
    GSIZE, 0.0f, WALL_H, 0.0f, 1.0f,
    GSIZE, 0.0f, 0.0f, 0.0f, 0.0f,

    // Second quad
    GSIZE, 0.0f, 0.0f, t, 1.0f,
    GSIZE, 0.0f, WALL_H, t, 0.0f,
    GSIZE, GSIZE, WALL_H, 0.0f, 0.0f,
    GSIZE, GSIZE, 0.0f, 0.0f, 1.0f,

    // Third quad
    GSIZE, GSIZE, 0.0f, t, 1.0f,
    GSIZE, GSIZE, WALL_H, t, 0.0f,
    0.0f, GSIZE, WALL_H, 0.0f, 0.0f,
    0.0f, GSIZE, 0.0f, 0.0f, 1.0f,

    // Fourth quad
    0.0f, GSIZE, 0.0f, t, 1.0f,
    0.0f, GSIZE, WALL_H, t, 0.0f,
    0.0f, 0.0f, WALL_H, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f, 1.0f
  };

  // Create and bind vertex buffer
  GLuint vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  // Use shader program
  GLuint shaderProgram = shader_get_basic();
  if (!shaderProgram) {
    init_shaders_android();
    shaderProgram = shader_get_basic();
    if (!shaderProgram) return;
  }
  useShaderProgram(shaderProgram);
  
  // Set identity model matrix
  setIdentityMatrix(shaderProgram, MATRIX_MODEL);

  // Set up attributes
  GLint positionLoc = glGetAttribLocation(shaderProgram, "position");
  GLint texCoordLoc = glGetAttribLocation(shaderProgram, "texCoord");

  if (positionLoc >= 0) {
    glEnableVertexAttribArray(positionLoc);
    glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), 0);
  }

  if (texCoordLoc >= 0) {
    glEnableVertexAttribArray(texCoordLoc);
    glVertexAttribPointer(texCoordLoc, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
  }

  // Set color with alpha
  GLint colorLoc = glGetUniformLocation(shaderProgram, "color");
  glUniform4f(colorLoc, 1.0f, 1.0f, 1.0f, 1.0f);

  // Bind texture
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, game->screen->texWall);

  // Set blend function
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

  // Enable blending
  glEnable(GL_BLEND);

  // Enable culling (disabled for Android debug to ensure visibility)
  // glEnable(GL_CULL_FACE);

  // Draw using triangles (6 vertices per quad, 4 quads => 24 indices)
  // Build an index buffer for 4 quads (0..15 vertices)
  GLushort indices[] = {
    0,1,2, 0,2,3,
    4,5,6, 4,6,7,
    8,9,10, 8,10,11,
    12,13,14, 12,14,15
  };
  GLuint ibo;
  glGenBuffers(1, &ibo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
  glDrawElements(GL_TRIANGLES, 24, GL_UNSIGNED_SHORT, 0);

  // Clean up
  glDisableVertexAttribArray(positionLoc);
  glDisableVertexAttribArray(texCoordLoc);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glDeleteBuffers(1, &vbo);
  glDeleteBuffers(1, &ibo);

  // Disable culling
  glDisable(GL_CULL_FACE);

  // Reset blend function
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  polycount += 4;
#else
  // For desktop OpenGL
  glColor4f(1.0, 1.0, 1.0, 1.0);

  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

  glEnable(GL_CULL_FACE);

  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, game->screen->texWall);
  glBegin(GL_QUADS);
  glTexCoord2f(t, 0.0); glVertex3f(0.0, 0.0, 0.0);
  glTexCoord2f(t, 1.0); glVertex3f(0.0, 0.0, WALL_H);
  glTexCoord2f(0.0, 1.0); glVertex3f(GSIZE, 0.0, WALL_H);
  glTexCoord2f(0.0, 0.0); glVertex3f(GSIZE, 0.0, 0.0);

  glTexCoord2f(t, 1.0); glVertex3f(GSIZE, 0.0, 0.0);
  glTexCoord2f(t, 0.0); glVertex3f(GSIZE, 0.0, WALL_H);
  glTexCoord2f(0.0, 0.0); glVertex3f(GSIZE, GSIZE, WALL_H);
  glTexCoord2f(0.0, 1.0); glVertex3f(GSIZE, GSIZE, 0.0);

  glTexCoord2f(t, 1.0); glVertex3f(GSIZE, GSIZE, 0.0);
  glTexCoord2f(t, 0.0); glVertex3f(GSIZE, GSIZE, WALL_H);
  glTexCoord2f(0.0, 0.0); glVertex3f(0.0, GSIZE, WALL_H);
  glTexCoord2f(0.0, 1.0); glVertex3f(0.0, GSIZE, 0.0);

  glTexCoord2f(t, 1.0); glVertex3f(0.0, GSIZE, 0.0);
  glTexCoord2f(t, 0.0); glVertex3f(0.0, GSIZE, WALL_H);
  glTexCoord2f(0.0, 0.0); glVertex3f(0.0, 0.0, WALL_H);
  glTexCoord2f(0.0, 1.0); glVertex3f(0.0, 0.0, 0.0);

  glEnd();
  polycount += 4;

  glDisable(GL_TEXTURE_2D);

  glDisable(GL_CULL_FACE);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#endif
}

/*
void drawHelp(gDisplay *d) {
  rasonly(d);
  glColor4f(0.2, 0.2, 0.2, 0.8);
  glEnable(GL_BLEND);
  glBegin(GL_QUADS);
  glVertex2i(0,0);
  glVertex2i(d->vp_w - 1, 0);
  glVertex2i(d->vp_w - 1, d->vp_h - 1);
  glVertex2i(0, d->vp_h - 1);
  glEnd();
  if(game->settings->show_alpha != 1) glDisable(GL_BLEND);
  glColor3f(1.0, 1.0, 0.0);
  drawLines(d->vp_w, d->vp_h,
	    help, HELP_LINES, 0);
}
*/

void drawCam(Player *p, gDisplay *d) {
  int i;

#ifndef ANDROID
  if (d->fog == 1) glEnable(GL_FOG);
#endif

#ifdef ANDROID
  // For Android, use shaders for rendering
  if (!p || !p->data || !game || !game->screen) {
    return;
  }
  GLuint shaderProgram = shader_get_basic();
  if (!shaderProgram) return;
  useShaderProgram(shaderProgram);

  // Set up projection matrix
  GLfloat projectionMatrix[16];
  float aspect = (float)d->vp_w / (float)d->vp_h;
  float fov = game->settings->fov * M_PI / 180.0f;
  float f = 1.0f / tan(fov / 2.0f);

  projectionMatrix[0] = f / aspect;
  projectionMatrix[1] = 0.0f;
  projectionMatrix[2] = 0.0f;
  projectionMatrix[3] = 0.0f;

  projectionMatrix[4] = 0.0f;
  projectionMatrix[5] = f;
  projectionMatrix[6] = 0.0f;
  projectionMatrix[7] = 0.0f;

  projectionMatrix[8] = 0.0f;
  projectionMatrix[9] = 0.0f;
  projectionMatrix[10] = (GSIZE + 3.0f) / (GSIZE - 3.0f);
  projectionMatrix[11] = 1.0f;

  projectionMatrix[12] = 0.0f;
  projectionMatrix[13] = 0.0f;
  projectionMatrix[14] = -2.0f * 3.0f * GSIZE / (GSIZE - 3.0f);
  projectionMatrix[15] = 0.0f;

  setProjectionMatrix(shaderProgram, projectionMatrix);
  {
    GLfloat viewIdentity[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
    setViewMatrix(shaderProgram, viewIdentity);
  }

  // Set up view matrix
  GLfloat viewMatrix[16];

  // Camera parameters
  float camDist   = 12.0f; // distance behind the player
  float camHeight = 6.0f;  // height above the ground

  // Player position
  float playerX = p->data->posx;
  float playerY = p->data->posy;

  // Player facing direction vector (unit vector from dirsX/dirsY)
  float dirX = dirsX[p->data->dir];
  float dirY = dirsY[p->data->dir];

  // Camera position = player position - direction * distance + height in Z
  float camX = playerX - dirX * camDist;
  float camY = playerY - dirY * camDist;
  float camZ = camHeight;

  // Look-at target = in front of the player
  float lookX = playerX + dirX * 5.0f; // 5 units ahead
  float lookY = playerY + dirY * 5.0f;
  float lookZ = 0.5f; // just above ground

  // Up vector (Z-up in GLTron)
  float upX = 0.0f, upY = 0.0f, upZ = 1.0f;

  // Create view matrix
  float forward[3] = {lookX - camX, lookY - camY, lookZ - camZ};
  float length = sqrt(forward[0] * forward[0] + forward[1] * forward[1] + forward[2] * forward[2]);
  forward[0] /= length;
  forward[1] /= length;
  forward[2] /= length;

  float right[3];
  right[0] = forward[1] * upZ - forward[2] * upY;
  right[1] = forward[2] * upX - forward[0] * upZ;
  right[2] = forward[0] * upY - forward[1] * upX;
  length = sqrt(right[0] * right[0] + right[1] * right[1] + right[2] * right[2]);
  right[0] /= length;
  right[1] /= length;
  right[2] /= length;

  float up[3];
  up[0] = right[1] * forward[2] - right[2] * forward[1];
  up[1] = right[2] * forward[0] - right[0] * forward[2];
  up[2] = right[0] * forward[1] - right[1] * forward[0];

  viewMatrix[0] = right[0];
  viewMatrix[1] = up[0];
  viewMatrix[2] = -forward[0];
  viewMatrix[3] = 0.0f;

  viewMatrix[4] = right[1];
  viewMatrix[5] = up[1];
  viewMatrix[6] = -forward[1];
  viewMatrix[7] = 0.0f;

  viewMatrix[8] = right[2];
  viewMatrix[9] = up[2];
  viewMatrix[10] = -forward[2];
  viewMatrix[11] = 0.0f;

  viewMatrix[12] = -(right[0] * camX + right[1] * camY + right[2] * camZ);
  viewMatrix[13] = -(up[0] * camX + up[1] * camY + up[2] * camZ);
  viewMatrix[14] = forward[0] * camX + forward[1] * camY + forward[2] * camZ;
  viewMatrix[15] = 1.0f;

  // Set view matrix in shader
  setViewMatrix(shaderProgram, viewMatrix);

  // Draw scene; re-apply matrices before sub-draws to ensure state
  setProjectionMatrix(shaderProgram, projectionMatrix);
  setViewMatrix(shaderProgram, viewMatrix);
  if (game->settings->show_wall == 1) {
    setProjectionMatrix(shaderProgram, projectionMatrix);
    setViewMatrix(shaderProgram, viewMatrix);
    drawWalls(d);
  }

  // Draw scene
  drawFloor(d);
  if (game->settings->show_wall == 1)
    drawWalls(d);

  for (i = 0; i < game->players; i++)
    drawTraces(&(game->player[i]), d, i);


  drawPlayers(p);

  if (game->settings->show_glow == 1)
    for (i = 0; i < game->players; i++)
      if ((p != &(game->player[i])) && (game->player[i].data->speed > 0))
        drawGlow(&(game->player[i]), d, TRAIL_HEIGHT * 4);

  // Clean up /* keep program bound */
  glDisable(GL_FOG);
#else
  // For desktop OpenGL
  glColor3f(0.0, 1.0, 0.0);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(game->settings->fov, (float)d->vp_w / (float)d->vp_h, 3.0, GSIZE);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  // Camera parameters
  float camDist   = 12.0f; // distance behind the player
  float camHeight = 6.0f;  // height above the ground

  // Player position
  float playerX = p->data->posx;
  float playerY = p->data->posy;

  // Player facing direction vector (unit vector from dirsX/dirsY)
  float dirX = dirsX[p->data->dir];
  float dirY = dirsY[p->data->dir];

  // Camera position = player position - direction * distance + height in Z
  float camX = playerX - dirX * camDist;
  float camY = playerY - dirY * camDist;
  float camZ = camHeight;

  // Look-at target = in front of the player
  float lookX = playerX + dirX * 5.0f; // 5 units ahead
  float lookY = playerY + dirY * 5.0f;
  float lookZ = 0.5f; // just above ground

  // Up vector (Z-up in GLTron)
  float upX = 0.0f, upY = 0.0f, upZ = 1.0f;

  gluLookAt(camX, camY, camZ, lookX, lookY, lookZ, upX, upY, upZ);

  // Light moves with camera
  glLightfv(GL_LIGHT0, GL_POSITION, p->camera->cam);

  // Draw scene
  drawFloor(d);
  if (game->settings->show_wall == 1)
    drawWalls(d);

  for (i = 0; i < game->players; i++)
    drawTraces(&(game->player[i]), d, i);

  drawPlayers(p);

  if (game->settings->show_glow == 1)
    for (i = 0; i < game->players; i++)
      if ((p != &(game->player[i])) && (game->player[i].data->speed > 0))
        drawGlow(&(game->player[i]), d, TRAIL_HEIGHT * 4);

  glDisable(GL_FOG);
#endif
}

void drawAI(gDisplay *d) {
  char ai[] = "computer player";

  rasonly(d);

#ifdef ANDROID
  // For Android, use centralized shader for text rendering
  GLuint shaderProgram = shader_get_basic();
  if (!shaderProgram) {
    init_shaders_android();
    shaderProgram = shader_get_basic();
    if (!shaderProgram) return;
  }
  useShaderProgram(shaderProgram);

  // Set up projection matrix
  GLfloat projectionMatrix[16] = {
    2.0f / d->vp_w, 0.0f, 0.0f, 0.0f,
    0.0f, -2.0f / d->vp_h, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    -1.0f, 1.0f, 0.0f, 1.0f
  };
  setProjectionMatrix(shaderProgram, projectionMatrix);
  {
    GLfloat viewIdentity[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
    setViewMatrix(shaderProgram, viewIdentity);
  }

  // Set color (white)
  GLint colorLoc = glGetUniformLocation(shaderProgram, "color");
  glUniform4f(colorLoc, 1.0f, 1.0f, 1.0f, 1.0f);

  // Create vertices for text rendering (simplified)
  float textScale = d->vp_w / (2 * strlen(ai));
  float x = d->vp_w / 4.0f;
  float y = 10.0f;

  // For each character in the message
  for (int i = 0; i < strlen(ai); i++) {
    // Get character
    char c = ai[i];

    // Calculate texture coordinates for the character
    // This is a simplified approach - you would need a proper font texture
    float texX = (c % 16) / 16.0f;
    float texY = (c / 16) / 16.0f;
    float texWidth = 1.0f / 16.0f;
    float texHeight = 1.0f / 16.0f;

    // Create vertices for the character quad
    GLfloat vertices[] = {
      x, y, 0.0f, texX, texY,
      x + textScale, y, 0.0f, texX + texWidth, texY,
      x + textScale, y + textScale, 0.0f, texX + texWidth, texY + texHeight,
      x, y + textScale, 0.0f, texX, texY + texHeight
    };

    // Create and bind vertex buffer
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Set up attributes
    GLint positionLoc = glGetAttribLocation(shaderProgram, "position");
    GLint texCoordLoc = glGetAttribLocation(shaderProgram, "texCoord");

    glEnableVertexAttribArray(positionLoc);
    glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), 0);

    glEnableVertexAttribArray(texCoordLoc);
    glVertexAttribPointer(texCoordLoc, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));

    // Bind font texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, game->screen->texFont);

    // Draw
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    // Clean up
    glDisableVertexAttribArray(positionLoc);
    glDisableVertexAttribArray(texCoordLoc);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &vbo);

    // Move to next character position
    x += textScale;
  }

  // Clean up
#else
  // For desktop OpenGL
  glColor3f(1.0, 1.0, 1.0);
  drawText(d->vp_w / 4, 10, d->vp_w / (2 * strlen(ai)), ai);
#endif
}

void drawPause(gDisplay *display) {
  char pause[] = "Game is paused";
  char winner[] = "Player %d wins";
  char buf[100];
  char *message;
  static float d = 0;
  static float lt = 0;
  float delta;
  long now;

  now = getElapsedTime();
  delta = now - lt;
  lt = now;
  delta /= 500.0;
  d += delta;

  if(d > 2 * M_PI) {
    d -= 2 * M_PI;
  }

  if(game->pauseflag & PAUSE_GAME_FINISHED &&
     game->winner != -1) {
    message = buf;
    sprintf(message, winner, game->winner + 1);
  } else {
    message = pause;
  }

  rasonly(game->screen);

#ifdef ANDROID
  // For Android, use centralized shader for text rendering
  GLuint shaderProgram = shader_get_basic();
  if (!shaderProgram) {
    init_shaders_android();
    shaderProgram = shader_get_basic();
    if (!shaderProgram) return;
  }
  useShaderProgram(shaderProgram);

  // Set up projection matrix
  GLfloat projectionMatrix[16] = {
    2.0f / display->vp_w, 0.0f, 0.0f, 0.0f,
    0.0f, -2.0f / display->vp_h, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    -1.0f, 1.0f, 0.0f, 1.0f
  };
  setProjectionMatrix(shaderProgram, projectionMatrix);
  {
    GLfloat viewIdentity[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
    setViewMatrix(shaderProgram, viewIdentity);
  }

  // Set color (red to yellow based on d)
  GLint colorLoc = glGetUniformLocation(shaderProgram, "color");
  glUniform4f(colorLoc, 1.0f, (sin(d) + 1) / 2, (sin(d) + 1) / 2, 1.0f);

  // Create vertices for text rendering (simplified)
  float textScale = display->vp_w / (6.0f / 4.0f * strlen(message));
  float x = display->vp_w / 6.0f;
  float y = 20.0f;

  // For each character in the message
  for (int i = 0; i < strlen(message); i++) {
    // Get character
    char c = message[i];

    // Calculate texture coordinates for the character
    // This is a simplified approach - you would need a proper font texture
    float texX = (c % 16) / 16.0f;
    float texY = (c / 16) / 16.0f;
    float texWidth = 1.0f / 16.0f;
    float texHeight = 1.0f / 16.0f;

    // Create vertices for the character quad
    GLfloat vertices[] = {
      x, y, 0.0f, texX, texY,
      x + textScale, y, 0.0f, texX + texWidth, texY,
      x + textScale, y + textScale, 0.0f, texX + texWidth, texY + texHeight,
      x, y + textScale, 0.0f, texX, texY + texHeight
    };

    // Create and bind vertex buffer
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Set up attributes
    GLint positionLoc = glGetAttribLocation(shaderProgram, "position");
    GLint texCoordLoc = glGetAttribLocation(shaderProgram, "texCoord");

    glEnableVertexAttribArray(positionLoc);
    glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), 0);

    glEnableVertexAttribArray(texCoordLoc);
    glVertexAttribPointer(texCoordLoc, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));

    // Bind font texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, game->screen->texFont);

    // Draw
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    // Clean up
    glDisableVertexAttribArray(positionLoc);
    glDisableVertexAttribArray(texCoordLoc);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &vbo);

    // Move to next character position
    x += textScale;
  }

  // Show hint for touch/mouse
  if (game->settings->input_mode != 0) {
    const char* hint = "Tap to resume";
    glUniform4f(colorLoc, 1.0f, 1.0f, 1.0f, 1.0f);

    // Calculate position for hint
    float hintScale = display->vp_w / (8.0f * strlen(hint));
    float hintX = display->vp_w / 6.0f;
    float hintY = 20.0f + display->vp_h / 12.0f;

    // For each character in the hint
    for (int i = 0; i < strlen(hint); i++) {
      // Get character
      char c = hint[i];

      // Calculate texture coordinates for the character
      float texX = (c % 16) / 16.0f;
      float texY = (c / 16) / 16.0f;
      float texWidth = 1.0f / 16.0f;
      float texHeight = 1.0f / 16.0f;

      // Create vertices for the character quad
      GLfloat vertices[] = {
        hintX, hintY, 0.0f, texX, texY,
        hintX + hintScale, hintY, 0.0f, texX + texWidth, texY,
        hintX + hintScale, hintY + hintScale, 0.0f, texX + texWidth, texY + texHeight,
        hintX, hintY + hintScale, 0.0f, texX, texY + texHeight
      };

      // Create and bind vertex buffer
      GLuint vbo;
      glGenBuffers(1, &vbo);
      glBindBuffer(GL_ARRAY_BUFFER, vbo);
      glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

      // Set up attributes
      GLint positionLoc = glGetAttribLocation(shaderProgram, "position");
      GLint texCoordLoc = glGetAttribLocation(shaderProgram, "texCoord");

      glEnableVertexAttribArray(positionLoc);
      glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), 0);

      glEnableVertexAttribArray(texCoordLoc);
      glVertexAttribPointer(texCoordLoc, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));

      // Bind font texture
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, game->screen->texFont);

      // Draw
      glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

      // Clean up
      glDisableVertexAttribArray(positionLoc);
      glDisableVertexAttribArray(texCoordLoc);
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glDeleteBuffers(1, &vbo);

      // Move to next character position
      hintX += hintScale;
    }
  }

  // Clean up
#else
  // For desktop OpenGL
  glColor3f(1.0, (sin(d) + 1) / 2, (sin(d) + 1) / 2);
  drawText(display->vp_w / 6, 20,
           display->vp_w / (6.0 / 4.0 * strlen(message)), message);

  // Show hint for touch/mouse
  if (game->settings->input_mode != 0) {
    const char* hint = "Tap to resume";
    glColor3f(1.0, 1.0, 1.0);
    drawText(display->vp_w / 6, 20 + display->vp_h / 12,
             display->vp_w / (8.0 * strlen(hint)), hint);
  }
#endif
}
