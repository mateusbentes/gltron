#include "gltron.h"
#include "geom.h"
#include <string.h>
#include <math.h>
#ifdef ANDROID
#include <android/log.h>
#endif
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Forward declarations for ordering
#ifdef ANDROID
void drawGlow(Player *p, gDisplay *d, float dim);
#endif

#ifdef ANDROID
#include <GLES2/gl2.h>
#include "shaders.h"
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#ifndef GL_TEXTURE_WIDTH
#define GL_TEXTURE_WIDTH 0x1000
#endif

#ifndef GL_TEXTURE_HEIGHT
#define GL_TEXTURE_HEIGHT 0x1001
#endif

// Define LINE_D constant
#ifndef LINE_D
#define LINE_D 0.05
#endif

// Define constants for Android
#ifndef GL_LIGHTING
#define GL_LIGHTING 0x0B50
#endif

#ifdef ANDROID
// Helper function to check OpenGL errors
// Defined in graphics.c, declared in gltron.h as extern
// Small helpers to keep shader state consistent
static inline GLuint ensure_basic_shader_bound() {
  GLuint prog = shader_get_basic();
  if (!prog) {
    init_shaders_android();
    prog = shader_get_basic();
    if (!prog) {
#ifdef ANDROID
      __android_log_print(ANDROID_LOG_ERROR, "GLTron", "Failed to initialize shaders!");
#endif
      return 0;
    }
  }
  if (prog) useShaderProgram(prog);
  return prog;
}

static inline void ensure2D(GLuint prog, int w, int h) {
  if (!prog) return;
  // 2D mode flag and ortho
  setRenderMode2D(prog, 1);
  GLfloat proj[16] = {
    2.0f / (GLfloat)w, 0, 0, 0,
    0, -2.0f / (GLfloat)h, 0, 0,
    0, 0, 1, 0,
    -1, 1, 0, 1
  };
  setProjectionMatrix(prog, proj);
  GLfloat identity[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
  setViewMatrix(prog, identity);
  setModelMatrix(prog, identity);
}

static inline void ensure3D(GLuint prog) {
  if (!prog) return;
  setRenderMode2D(prog, 0);
}
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
  // For Android, implement debug texture rendering with proper vertex format
  // Define reasonable defaults if not available
  float width = 64.0f;   // Default width for debug texture
  float height = 32.0f;  // Default height for debug texture
  
  // Use actual values if available
  if (game && game->screen) {
    // Adjust size based on viewport
    width = game->screen->vp_w / 10.0f;
    height = game->screen->vp_h / 20.0f;
  }
  
  __android_log_print(ANDROID_LOG_INFO, "GLTron", "drawDebugTex called: x=%d, y=%d, w=%.1f, h=%.1f", x, y, width, height);
  
  // Create vertices with 3D position (x,y,z) and 2D texture coords (u,v)
  GLfloat vertices[] = {
    // Position (x,y,z)           // TexCoord (u,v)
    (float)x,         (float)y,          0.0f,   0.0f, 0.0f,
    (float)x + width, (float)y,          0.0f,   1.0f, 0.0f,
    (float)x + width, (float)y + height, 0.0f,   1.0f, 1.0f,
    (float)x,         (float)y + height, 0.0f,   0.0f, 1.0f
  };

  __android_log_print(ANDROID_LOG_INFO, "GLTron", "drawDebugTex vertices bounds: minX=%d, maxX=%.1f, minY=%d, maxY=%.1f", 
                      x, (float)x + width, y, (float)y + height);

  GLushort indices[] = {0, 1, 2, 0, 2, 3};

  // Create and bind vertex buffer
  GLuint vbo = 0;
  glGenBuffers(1, &vbo);
  if (vbo == 0) {
    checkGLError("glGenBuffers");
    __android_log_print(ANDROID_LOG_ERROR, "GLTron", "Failed to create VBO for debug texture");
    return;
  }
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  // Create and bind index buffer
  GLuint ibo = 0;
  glGenBuffers(1, &ibo);
  if (ibo == 0) {
    checkGLError("glGenBuffers for ibo");
    __android_log_print(ANDROID_LOG_ERROR, "GLTron", "Failed to create EBO for debug texture");
    glDeleteBuffers(1, &vbo);
    return;
  }
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

  // Use shader program
  GLuint shaderProgram = ensure_basic_shader_bound();
  if (!shaderProgram) {
    __android_log_print(ANDROID_LOG_ERROR, "GLTron", "Failed to bind shader for debug texture");
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ibo);
    return;
  }
  ensure2D(shaderProgram, game->screen->vp_w, game->screen->vp_h);
  __android_log_print(ANDROID_LOG_INFO, "GLTron", "Shader bound for debug texture: program=%u", shaderProgram);

  // Set up attributes
  GLint positionLoc = glGetAttribLocation(shaderProgram, "position");
  GLint texCoordLoc = glGetAttribLocation(shaderProgram, "texCoord");
  __android_log_print(ANDROID_LOG_INFO, "GLTron", "Attribute locations for debug texture: position=%d, texCoord=%d", positionLoc, texCoordLoc);

  if (positionLoc >= 0) {
    glEnableVertexAttribArray(positionLoc);
    // Position is 3 floats, stride is 5 floats (3 pos + 2 tex)
    glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), 0);
  } else {
    __android_log_print(ANDROID_LOG_ERROR, "GLTron", "Position attribute not found for debug texture");
  }

  if (texCoordLoc >= 0) {
    glEnableVertexAttribArray(texCoordLoc);
    // Texture coords are 2 floats, starting after 3 position floats
    glVertexAttribPointer(texCoordLoc, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
  } else {
    __android_log_print(ANDROID_LOG_INFO, "GLTron", "Texture coordinate attribute not found for debug texture");
  }

  // Set color for the debug texture
  setColor(shaderProgram, 0.0f, 1.0f, 0.0f, 1.0f); // Green color like desktop

  // Bind texture
  glActiveTexture(GL_TEXTURE0);
  GLuint texToUse = 0;
  
  if (game && game->screen && game->screen->texFloor > 0) {
    texToUse = game->screen->texFloor;
    glBindTexture(GL_TEXTURE_2D, texToUse);
    __android_log_print(ANDROID_LOG_INFO, "GLTron", "Bound floor texture for debug: ID=%u", texToUse);
  } else {
    // Create a simple checkerboard pattern for debug
    static GLuint debugTex = 0;
    if (debugTex == 0) {
      unsigned char pattern[4*4*4]; // 4x4 RGBA texture
      for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
          int idx = (i * 4 + j) * 4;
          unsigned char val = ((i + j) % 2) ? 255 : 0;
          pattern[idx] = val;     // R
          pattern[idx+1] = 255;   // G
          pattern[idx+2] = val;   // B
          pattern[idx+3] = 255;   // A
        }
      }
      glGenTextures(1, &debugTex);
      glBindTexture(GL_TEXTURE_2D, debugTex);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 4, 4, 0, GL_RGBA, GL_UNSIGNED_BYTE, pattern);
    }
    glBindTexture(GL_TEXTURE_2D, debugTex);
    texToUse = debugTex;
    __android_log_print(ANDROID_LOG_INFO, "GLTron", "Created debug checkerboard texture: ID=%u", debugTex);
  }
  
  setTexture(shaderProgram, 0);

  // Draw
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
  __android_log_print(ANDROID_LOG_INFO, "GLTron", "Debug texture drawn");

  // Clean up
  if (positionLoc >= 0) glDisableVertexAttribArray(positionLoc);
  if (texCoordLoc >= 0) glDisableVertexAttribArray(texCoordLoc);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glDeleteBuffers(1, &vbo);
  glDeleteBuffers(1, &ibo);
  
  polycount++;
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
  // For Android, render score text using font texture
  GLuint shaderProgram = ensure_basic_shader_bound();
  if (!shaderProgram) return;
  ensure2D(shaderProgram, game->screen->vp_w, game->screen->vp_h);

  // Set color (yellow like desktop)
  setColor(shaderProgram, 1.0f, 1.0f, 0.2f, 1.0f);

  // Ensure font texture exists
  glActiveTexture(GL_TEXTURE0);
  if (game->screen->texFont == 0) {
    game->screen->texFont = createFontTexture();
  }
  glBindTexture(GL_TEXTURE_2D, game->screen->texFont);
  setTexture(shaderProgram, 0);

  // Get attribute locations once
  GLint positionLoc = glGetAttribLocation(shaderProgram, "position");
  GLint texCoordLoc = glGetAttribLocation(shaderProgram, "texCoord");

  if (positionLoc < 0 || texCoordLoc < 0) {
    __android_log_print(ANDROID_LOG_ERROR, "GLTron", "Failed to get attributes for score rendering");
    return;
  }

  // Enable attributes for all characters
  glEnableVertexAttribArray(positionLoc);
  glEnableVertexAttribArray(texCoordLoc);

  // Render each character of the score
  float x = 5.0f;
  float y = 5.0f;
  float charSize = 32.0f; // Match desktop size
  
  for (int i = 0; tmp[i] != '\0'; i++) {
    char c = tmp[i];
    
    // Calculate texture coordinates for this character
    // Assuming font texture has 16x6 grid of ASCII characters (32-127)
    if (c < 32 || c > 127) c = '?'; // Fallback for invalid chars
    
    int charIndex = c - 32;
    float texX = (float)(charIndex % 16) / 16.0f;
    float texY = (float)(charIndex / 16) / 6.0f;
    float texW = 1.0f / 16.0f;
    float texH = 1.0f / 6.0f;

    // Create vertices for this character
    GLfloat vertices[] = {
      // Position (x,y,z)         // TexCoord (u,v)
      x,            y,            0.0f,   texX,        texY,
      x + charSize, y,            0.0f,   texX + texW, texY,
      x + charSize, y + charSize, 0.0f,   texX + texW, texY + texH,
      x,            y + charSize, 0.0f,   texX,        texY + texH
    };

    // Create and bind vertex buffer for this character
    GLuint vbo = 0;
    glGenBuffers(1, &vbo);
    if (vbo == 0) {
      checkGLError("glGenBuffers for score char");
      continue;
    }
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Set up vertex attributes
    glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), 0);
    glVertexAttribPointer(texCoordLoc, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));

    // Draw this character
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    // Clean up buffer for this character
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &vbo);

    // Move to next character position (with some spacing)
    x += charSize * 0.8f; // Slightly overlap for better spacing
  }

  // Disable attributes after all characters are drawn
  glDisableVertexAttribArray(positionLoc);
  glDisableVertexAttribArray(texCoordLoc);
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

        GLuint shaderProgram = ensure_basic_shader_bound();
        if (!shaderProgram) return;
        ensure3D(shaderProgram);
        setIdentityMatrix(shaderProgram, MATRIX_MODEL);

        // Setup texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, game->screen->texFloor);
        setTexture(shaderProgram, 0);

        // Set texture parameters for proper rendering
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        // Set lighting uniforms
        setAmbientLight(shaderProgram, 0.2f, 0.2f, 0.2f);
        setLightColor(shaderProgram, 1.0f, 1.0f, 1.0f);
        setLightPosition(shaderProgram, 1.0f, 1.0f, 1.0f);
        setColor(shaderProgram, 1.0f, 1.0f, 1.0f, 1.0f);

        // Use immediate mode style vertex arrays for simplicity
        l = GSIZE / 4;
        t = 5;
        
        // Static buffers for floor rendering to avoid repeated creation/deletion
        static GLuint floor_vbo = 0, floor_ebo = 0;
        static int floor_initialized = 0;
        static int floor_quad_count = 0;
        static int floor_index_count = 0;

        if (!floor_initialized) {
            floor_quad_count = (GSIZE / l) * (GSIZE / l);
            GLfloat *vertices = (GLfloat *)malloc(floor_quad_count * 4 * 5 * sizeof(GLfloat));
            GLushort *indices = (GLushort *)malloc(floor_quad_count * 6 * sizeof(GLushort));
            
            if (!vertices || !indices) {
                __android_log_print(ANDROID_LOG_ERROR, "GLTron", "Failed to allocate memory for floor vertices");
                free(vertices);
                free(indices);
                return;
            }

            int vIndex = 0, iIndex = 0, quadIndex = 0;

            for(j = 0; j < GSIZE; j += l) {
                for(k = 0; k < GSIZE; k += l) {
                    // Position (x,y,z) and Texture (u,v) for each vertex of the quad
                    vertices[vIndex++] = j;       vertices[vIndex++] = k;       vertices[vIndex++] = 0.0f; vertices[vIndex++] = 0.0f; vertices[vIndex++] = 0.0f;
                    vertices[vIndex++] = j + l;   vertices[vIndex++] = k;       vertices[vIndex++] = 0.0f; vertices[vIndex++] = t;    vertices[vIndex++] = 0.0f;
                    vertices[vIndex++] = j + l;   vertices[vIndex++] = k + l;   vertices[vIndex++] = 0.0f; vertices[vIndex++] = t;    vertices[vIndex++] = t;
                    vertices[vIndex++] = j;       vertices[vIndex++] = k + l;   vertices[vIndex++] = 0.0f; vertices[vIndex++] = 0.0f; vertices[vIndex++] = t;

                    // Indices for two triangles per quad
                    int base = quadIndex * 4;
                    indices[iIndex++] = base + 0; indices[iIndex++] = base + 1; indices[iIndex++] = base + 2;
                    indices[iIndex++] = base + 0; indices[iIndex++] = base + 2; indices[iIndex++] = base + 3;

                    quadIndex++;
                    polycount++;
                }
            }
            floor_index_count = iIndex;

            // Create VBO
            glGenBuffers(1, &floor_vbo);
            if (floor_vbo == 0) {
                __android_log_print(ANDROID_LOG_ERROR, "GLTron", "Failed to create floor VBO");
                free(vertices);
                free(indices);
                return;
            }
            glBindBuffer(GL_ARRAY_BUFFER, floor_vbo);
            glBufferData(GL_ARRAY_BUFFER, floor_quad_count * 4 * 5 * sizeof(GLfloat), vertices, GL_STATIC_DRAW);

            // Create EBO
            glGenBuffers(1, &floor_ebo);
            if (floor_ebo == 0) {
                __android_log_print(ANDROID_LOG_ERROR, "GLTron", "Failed to create floor EBO");
                glDeleteBuffers(1, &floor_vbo);
                floor_vbo = 0;
                free(vertices);
                free(indices);
                return;
            }
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, floor_ebo);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, floor_index_count * sizeof(GLushort), indices, GL_STATIC_DRAW);

            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            free(vertices);
            free(indices);
            floor_initialized = 1;
        }

        // Use pre-created buffers
        glBindBuffer(GL_ARRAY_BUFFER, floor_vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, floor_ebo);

        // Get attribute locations
        GLint posLoc = glGetAttribLocation(shaderProgram, "position");
        GLint texLoc = glGetAttribLocation(shaderProgram, "texCoord");
        GLint normalLoc = glGetAttribLocation(shaderProgram, "normal");

        // Set up vertex attributes
        if (posLoc >= 0) {
            glEnableVertexAttribArray(posLoc);
            glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)0);
        }

        if (texLoc >= 0) {
            glEnableVertexAttribArray(texLoc);
            glVertexAttribPointer(texLoc, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
        }

        // Set flat normal for floor (pointing up)
        if (normalLoc >= 0) {
            glVertexAttrib3f(normalLoc, 0.0f, 0.0f, 1.0f);
        }

        // Draw the floor
        glDrawElements(GL_TRIANGLES, floor_index_count, GL_UNSIGNED_SHORT, 0);

        // Clean up vertex attributes
        if (posLoc >= 0) glDisableVertexAttribArray(posLoc);
        if (texLoc >= 0) glDisableVertexAttribArray(texLoc);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

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
        GLuint shaderProgram = ensure_basic_shader_bound();
        if (!shaderProgram) return;
        ensure3D(shaderProgram);
        setIdentityMatrix(shaderProgram, MATRIX_MODEL);

        // Set lighting uniforms
        setAmbientLight(shaderProgram, 0.2f, 0.2f, 0.2f);
        setLightColor(shaderProgram, 1.0f, 1.0f, 1.0f);
        setLightPosition(shaderProgram, 1.0f, 1.0f, 1.0f);

        // Bind white texture for solid color rendering
        glActiveTexture(GL_TEXTURE0);
        static GLuint s_white = 0;
        if (s_white == 0) s_white = createWhiteTexture();
        glBindTexture(GL_TEXTURE_2D, s_white);
        setTexture(shaderProgram, 0);

        // Static buffers for line floor to avoid repeated allocation
        static GLuint line_vbo = 0;
        static int line_initialized = 0;
        static int line_vertex_count = 0;

        if (!line_initialized) {
            // Calculate number of lines
            int lineCount = 0;
            for(j = 0; j <= GSIZE; j += game->settings->line_spacing) {
                lineCount += 2; // horizontal and vertical line
            }
            line_vertex_count = lineCount * 2; // 2 vertices per line

            // Create vertex data
            GLfloat *vertices = malloc(line_vertex_count * 3 * sizeof(GLfloat));
            if (!vertices) {
                __android_log_print(ANDROID_LOG_ERROR, "GLTron", "Failed to allocate memory for line vertices");
                return;
            }

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
            glGenBuffers(1, &line_vbo);
            if (line_vbo == 0) {
                __android_log_print(ANDROID_LOG_ERROR, "GLTron", "Failed to create line VBO");
                free(vertices);
                return;
            }
            glBindBuffer(GL_ARRAY_BUFFER, line_vbo);
            glBufferData(GL_ARRAY_BUFFER, line_vertex_count * 3 * sizeof(GLfloat), vertices, GL_STATIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            free(vertices);
            line_initialized = 1;
        }

        // Use the static buffer
        glBindBuffer(GL_ARRAY_BUFFER, line_vbo);

        GLint posLoc = glGetAttribLocation(shaderProgram, "position");
        GLint normalLoc = glGetAttribLocation(shaderProgram, "normal");

        if (posLoc >= 0) {
            glEnableVertexAttribArray(posLoc);
            glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
        }

        // Set flat normal for lines
        if (normalLoc >= 0) {
            glVertexAttrib3f(normalLoc, 0.0f, 0.0f, 1.0f);
        }

        // Set blue color for lines (matching desktop)
        setColor(shaderProgram, 0.0f, 0.0f, 1.0f, 1.0f);

        // Draw lines
        glDrawArrays(GL_LINES, 0, line_vertex_count);

        // Clean up
        if (posLoc >= 0) glDisableVertexAttribArray(posLoc);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

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
    // For Android, use vertex buffers with unified shader helpers
    // Count the number of trail segments
    int segmentCount = 0;
    line = &(data->trails[0]);
    while(line != data->trail) {
      segmentCount++;
      line++;
    }
    
    // Need at least one segment to draw
    if (segmentCount == 0) {
      return;
    }
    
    // For a triangle strip, we need 2 vertices per point (bottom and top)
    // First segment needs 2 points (start and end), each additional segment adds 1 point
    int vertexCount = (segmentCount + 1) * 2; // (segments + 1) points * 2 (top and bottom)
    GLfloat *vertices = (GLfloat *)malloc(vertexCount * 3 * sizeof(GLfloat));
    if (!vertices) {
      __android_log_print(ANDROID_LOG_ERROR, "GLTron", "Failed to allocate memory for trace vertices");
      return;
    }
    
    int vIndex = 0;
    
    // Start with the first segment's start point
    line = &(data->trails[0]);
    vertices[vIndex++] = line->sx;
    vertices[vIndex++] = line->sy;
    vertices[vIndex++] = 0.0f;
    vertices[vIndex++] = line->sx;
    vertices[vIndex++] = line->sy;
    vertices[vIndex++] = height;
    
    // Add all segment endpoints
    while(line != data->trail) {
      vertices[vIndex++] = line->ex;
      vertices[vIndex++] = line->ey;
      vertices[vIndex++] = 0.0f;
      vertices[vIndex++] = line->ex;
      vertices[vIndex++] = line->ey;
      vertices[vIndex++] = height;
      line++;
      polycount++;
    }
    
    // Add the final endpoint
    vertices[vIndex++] = line->ex;
    vertices[vIndex++] = line->ey;
    vertices[vIndex++] = 0.0f;
    vertices[vIndex++] = line->ex;
    vertices[vIndex++] = line->ey;
    vertices[vIndex++] = height;
    polycount++;

    // Use shader program consistently
    GLuint shaderProgram = ensure_basic_shader_bound();
    if (!shaderProgram) {
      __android_log_print(ANDROID_LOG_ERROR, "GLTron", "Failed to bind shader for traces");
      free(vertices);
      return;
    }
    ensure3D(shaderProgram);

    // Identity model matrix for world-space trails
    setIdentityMatrix(shaderProgram, MATRIX_MODEL);

    // Bind white texture for solid color rendering
    glActiveTexture(GL_TEXTURE0);
    static GLuint s_white = 0;
    if (s_white == 0) s_white = createWhiteTexture();
    glBindTexture(GL_TEXTURE_2D, s_white);
    setTexture(shaderProgram, 0);

    // Set color from player material
    setColor(shaderProgram, p->model->color_alpha[0], p->model->color_alpha[1], p->model->color_alpha[2], p->model->color_alpha[3]);

    // Create and bind vertex buffer
    GLuint vbo = 0;
    glGenBuffers(1, &vbo);
    if (vbo == 0) {
      checkGLError("glGenBuffers");
      __android_log_print(ANDROID_LOG_ERROR, "GLTron", "Failed to create VBO for traces");
      free(vertices);
      return;
    }
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertexCount * 3 * sizeof(GLfloat), vertices, GL_STATIC_DRAW);

    // Set up attributes
    GLint positionLoc = glGetAttribLocation(shaderProgram, "position");
    GLint normalLoc = glGetAttribLocation(shaderProgram, "normal");
    
    if (positionLoc >= 0) {
      glEnableVertexAttribArray(positionLoc);
      glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    }
    
    // Set a default normal for the trail walls (could be improved with proper normals)
    if (normalLoc >= 0) {
      glVertexAttrib3f(normalLoc, 0.0f, 0.0f, 1.0f);
    }

    // Draw the trail wall as a triangle strip
    glDrawArrays(GL_TRIANGLE_STRIP, 0, vertexCount);

    // Clean up
    if (positionLoc >= 0) glDisableVertexAttribArray(positionLoc);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &vbo);
    free(vertices);

    if(game->settings->camType == 1) {
      GLfloat quadVertices[] = {
        data->trail->sx - LINE_D, data->trail->sy - LINE_D, 0.0f,
        data->trail->sx + LINE_D, data->trail->sy + LINE_D, 0.0f,
        data->trail->ex + LINE_D, data->trail->ey + LINE_D, 0.0f,
        data->trail->ex - LINE_D, data->trail->ey - LINE_D, 0.0f
      };

      // Create and bind vertex buffer for quad
      GLuint quadVbo = 0;
      glGenBuffers(1, &quadVbo);
      if (quadVbo == 0) {
        checkGLError("glGenBuffers for quad");
        return;
      }
      glBindBuffer(GL_ARRAY_BUFFER, quadVbo);
      glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

      // Attribute reuse
      if (positionLoc >= 0) {
        glEnableVertexAttribArray(positionLoc);
        glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
      }

      // Draw
      glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

      // Clean up
      if (positionLoc >= 0) glDisableVertexAttribArray(positionLoc);
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
  // For Android, render crash explosion effect on the ground around the cycle
  // This should be in 3D mode and positioned at the cycle location
  GLuint shaderProgram = ensure_basic_shader_bound();
  if (!shaderProgram) return;
  ensure3D(shaderProgram);  // Use 3D mode, not 2D
  
  // The crash effect is a horizontal quad on the ground
  // It expands outward based on the radius parameter
  float size = CRASH_W * (radius / EXP_RADIUS_MAX);  // Scale based on explosion radius
  
  GLfloat vertices[] = {
    // Position (x,y,z)                    // TexCoord (u,v)
    -size, -size, 0.1f,   0.0f, 0.0f,  // Slightly above ground to avoid z-fighting
     size, -size, 0.1f,   1.0f, 0.0f,
     size,  size, 0.1f,   1.0f, 1.0f,
    -size,  size, 0.1f,   0.0f, 1.0f
  };

  GLuint vbo = 0;
  glGenBuffers(1, &vbo);
  if (vbo == 0) {
    checkGLError("glGenBuffers");
    return;
  }
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  GLushort indices[] = {0,1,2, 0,2,3};
  GLuint ibo = 0;
  glGenBuffers(1, &ibo);
  if (ibo == 0) {
    glDeleteBuffers(1, &vbo);
    return;
  }
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

  // Set up attributes
  GLint positionLoc = glGetAttribLocation(shaderProgram, "position");
  GLint texCoordLoc = glGetAttribLocation(shaderProgram, "texCoord");
  GLint normalLoc = glGetAttribLocation(shaderProgram, "normal");

  if (positionLoc >= 0) {
    glEnableVertexAttribArray(positionLoc);
    glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), 0);
  }

  if (texCoordLoc >= 0) {
    glEnableVertexAttribArray(texCoordLoc);
    glVertexAttribPointer(texCoordLoc, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
  }
  
  // Set normal pointing up for ground explosion
  if (normalLoc >= 0) {
    glVertexAttrib3f(normalLoc, 0.0f, 0.0f, 1.0f);
  }

  // Set color with alpha for fade effect
  float alpha = (EXP_RADIUS_MAX - radius) / EXP_RADIUS_MAX;
  setColor(shaderProgram, 1.0f, 1.0f, 1.0f, alpha);

  // Bind crash texture
  glActiveTexture(GL_TEXTURE0);
  if (game->screen->texCrash != 0) {
    glBindTexture(GL_TEXTURE_2D, game->screen->texCrash);
  } else {
    // Fallback to wall texture if crash texture not available
    glBindTexture(GL_TEXTURE_2D, game->screen->texWall);
  }
  setTexture(shaderProgram, 0);
  
  // Enable blending for transparency
  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

  // Draw the crash explosion quad
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

  // Clean up
  if (positionLoc >= 0) glDisableVertexAttribArray(positionLoc);
  if (texCoordLoc >= 0) glDisableVertexAttribArray(texCoordLoc);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glDeleteBuffers(1, &vbo);
  glDeleteBuffers(1, &ibo);

  // Reset blend function
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  polycount++;
#else
  // For desktop OpenGL
  glColor4f(1.0, 1.0, 1.0, 1.0);

  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

  glEnable(GL_CULL_FACE);

  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, game->screen->texWall);
  glBegin(GL_QUADS);
  glTexCoord2f(0.0, 0.0); glVertex3f(0.0, 0.0, 0.0);
  glTexCoord2f(0.0, 1.0); glVertex3f(0.0, 0.0, WALL_H);
  glTexCoord2f(0.0, 1.0); glVertex3f(GSIZE, 0.0, WALL_H);
  glTexCoord2f(0.0, 0.0); glVertex3f(GSIZE, 0.0, 0.0);

  glTexCoord2f(0.0, 1.0); glVertex3f(GSIZE, 0.0, 0.0);
  glTexCoord2f(1.0, 0.0); glVertex3f(GSIZE, 0.0, WALL_H);
  glTexCoord2f(0.0, 0.0); glVertex3f(GSIZE, GSIZE, WALL_H);
  glTexCoord2f(0.0, 1.0); glVertex3f(GSIZE, GSIZE, 0.0);

  glTexCoord2f(0.0, 1.0); glVertex3f(GSIZE, GSIZE, 0.0);
  glTexCoord2f(1.0, 0.0); glVertex3f(GSIZE, GSIZE, WALL_H);
  glTexCoord2f(0.0, 0.0); glVertex3f(0.0, GSIZE, WALL_H);
  glTexCoord2f(0.0, 1.0); glVertex3f(0.0, GSIZE, 0.0);

  glTexCoord2f(0.0, 1.0); glVertex3f(0.0, GSIZE, 0.0);
  glTexCoord2f(1.0, 0.0); glVertex3f(0.0, GSIZE, WALL_H);
  glTexCoord2f(0.0, 0.0); glVertex3f(0.0, 0.0, WALL_H);
  glTexCoord2f(0.0, 1.0); glVertex3f(0.0, 0.0, 0.0);

  glEnd();
  polycount += 4;

  glDisable(GL_TEXTURE_2D);

  glDisable(GL_CULL_FACE);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#endif
}

void drawCycle(Player *p) {
  float dirangles[] = { 180, 90, 0, 270 , 360, -90 };
  int time = 0;
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
  GLuint prog = ensure_basic_shader_bound();
  if (!prog) {
    __android_log_print(ANDROID_LOG_ERROR, "GLTron", "Failed to get shader for cycle");
    return;
  }
  ensure3D(prog);  // Ensure we're in 3D mode

  // Set default lighting values
  setAmbientLight(prog, 0.2f, 0.2f, 0.2f);
  setLightColor(prog, 1.0f, 1.0f, 1.0f);
  setLightPosition(prog, 1.0f, 1.0f, 1.0f);

  // Bind a white texture for solid color rendering
  glActiveTexture(GL_TEXTURE0);
  static GLuint s_white = 0;
  if (s_white == 0) s_white = createWhiteTexture();
  glBindTexture(GL_TEXTURE_2D, s_white);
  setTexture(prog, 0);

  // Enable depth testing for 3D rendering
  glEnable(GL_DEPTH_TEST);
  glDepthMask(GL_TRUE);

  // Build transformation matrices (equivalent to glPushMatrix/glTranslatef/glRotatef sequence)
  GLfloat modelMatrix[16];
  
  // Start with identity matrix
  modelMatrix[0] = 1.0f; modelMatrix[1] = 0.0f; modelMatrix[2] = 0.0f; modelMatrix[3] = 0.0f;
  modelMatrix[4] = 0.0f; modelMatrix[5] = 1.0f; modelMatrix[6] = 0.0f; modelMatrix[7] = 0.0f;
  modelMatrix[8] = 0.0f; modelMatrix[9] = 0.0f; modelMatrix[10] = 1.0f; modelMatrix[11] = 0.0f;
  modelMatrix[12] = 0.0f; modelMatrix[13] = 0.0f; modelMatrix[14] = 0.0f; modelMatrix[15] = 1.0f;

  // Apply translation (equivalent to glTranslatef(p->data->posx, p->data->posy, 0.0))
  modelMatrix[12] = p->data->posx;
  modelMatrix[13] = p->data->posy;
  modelMatrix[14] = 0.0f;

  // Calculate rotation angle (same logic as desktop)
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

  // Apply rotation around Z-axis (equivalent to glRotatef(dirangle, 0, 0, 1))
  float radians = dirangle * M_PI / 180.0f;
  float cosAngle = cos(radians);
  float sinAngle = sin(radians);
  
  GLfloat rotatedMatrix[16];
  // Apply rotation to current matrix
  rotatedMatrix[0] = modelMatrix[0] * cosAngle - modelMatrix[4] * sinAngle;
  rotatedMatrix[1] = modelMatrix[1] * cosAngle - modelMatrix[5] * sinAngle;
  rotatedMatrix[2] = modelMatrix[2] * cosAngle - modelMatrix[6] * sinAngle;
  rotatedMatrix[3] = modelMatrix[3] * cosAngle - modelMatrix[7] * sinAngle;
  
  rotatedMatrix[4] = modelMatrix[0] * sinAngle + modelMatrix[4] * cosAngle;
  rotatedMatrix[5] = modelMatrix[1] * sinAngle + modelMatrix[5] * cosAngle;
  rotatedMatrix[6] = modelMatrix[2] * sinAngle + modelMatrix[6] * cosAngle;
  rotatedMatrix[7] = modelMatrix[3] * sinAngle + modelMatrix[7] * cosAngle;
  
  rotatedMatrix[8] = modelMatrix[8];
  rotatedMatrix[9] = modelMatrix[9];
  rotatedMatrix[10] = modelMatrix[10];
  rotatedMatrix[11] = modelMatrix[11];
  
  rotatedMatrix[12] = modelMatrix[12];
  rotatedMatrix[13] = modelMatrix[13];
  rotatedMatrix[14] = modelMatrix[14];
  rotatedMatrix[15] = modelMatrix[15];

  // Copy rotated matrix back
  memcpy(modelMatrix, rotatedMatrix, sizeof(modelMatrix));

  // Handle crash texture rendering (equivalent to desktop version)
  if(game->settings->show_crash_texture) {
    if(p->data->exp_radius > 0 && p->data->exp_radius < EXP_RADIUS_MAX) {
      // Save current model matrix
      GLfloat savedMatrix[16];
      memcpy(savedMatrix, modelMatrix, sizeof(savedMatrix));
      
      // Set the current transformation for the crash effect
      setModelMatrix(prog, modelMatrix);
      
      // Draw the crash explosion effect
      drawCrash(p->data->exp_radius);
      
      // Restore for cycle rendering
      memcpy(modelMatrix, savedMatrix, sizeof(modelMatrix));
    }
  }

  // Apply tilt rotation if turning (equivalent to second glRotatef in desktop)
  if(game->settings->turn_cycle && time < turn_length) {
    float axis = 1.0f;
    if(p->data->dir < p->data->last_dir && p->data->last_dir != 3) 
      axis = -1.0f;
    else if((p->data->last_dir == 3 && p->data->dir == 2) || 
            (p->data->last_dir == 0 && p->data->dir == 3)) 
      axis = -1.0f;
    
    float tiltAngle = neigung * sin(M_PI * time / turn_length);
    float tiltRadians = tiltAngle * M_PI / 180.0f;
    float tiltCos = cos(tiltRadians);
    float tiltSin = sin(tiltRadians);
    
    GLfloat tiltedMatrix[16];
    // Apply Y-axis rotation (axis determines direction)
    // For Y-axis rotation: X and Z components change, Y stays the same
    if (axis > 0) {
      // Rotation around +Y axis
      tiltedMatrix[0] = modelMatrix[0] * tiltCos + modelMatrix[8] * tiltSin;
      tiltedMatrix[1] = modelMatrix[1] * tiltCos + modelMatrix[9] * tiltSin;
      tiltedMatrix[2] = modelMatrix[2] * tiltCos + modelMatrix[10] * tiltSin;
      tiltedMatrix[3] = modelMatrix[3] * tiltCos + modelMatrix[11] * tiltSin;
      
      tiltedMatrix[4] = modelMatrix[4];
      tiltedMatrix[5] = modelMatrix[5];
      tiltedMatrix[6] = modelMatrix[6];
      tiltedMatrix[7] = modelMatrix[7];
      
      tiltedMatrix[8] = -modelMatrix[0] * tiltSin + modelMatrix[8] * tiltCos;
      tiltedMatrix[9] = -modelMatrix[1] * tiltSin + modelMatrix[9] * tiltCos;
      tiltedMatrix[10] = -modelMatrix[2] * tiltSin + modelMatrix[10] * tiltCos;
      tiltedMatrix[11] = -modelMatrix[3] * tiltSin + modelMatrix[11] * tiltCos;
    } else {
      // Rotation around -Y axis (reverse the sin terms)
      tiltedMatrix[0] = modelMatrix[0] * tiltCos - modelMatrix[8] * tiltSin;
      tiltedMatrix[1] = modelMatrix[1] * tiltCos - modelMatrix[9] * tiltSin;
      tiltedMatrix[2] = modelMatrix[2] * tiltCos - modelMatrix[10] * tiltSin;
      tiltedMatrix[3] = modelMatrix[3] * tiltCos - modelMatrix[11] * tiltSin;
      
      tiltedMatrix[4] = modelMatrix[4];
      tiltedMatrix[5] = modelMatrix[5];
      tiltedMatrix[6] = modelMatrix[6];
      tiltedMatrix[7] = modelMatrix[7];
      
      tiltedMatrix[8] = modelMatrix[0] * tiltSin + modelMatrix[8] * tiltCos;
      tiltedMatrix[9] = modelMatrix[1] * tiltSin + modelMatrix[9] * tiltCos;
      tiltedMatrix[10] = modelMatrix[2] * tiltSin + modelMatrix[10] * tiltCos;
      tiltedMatrix[11] = modelMatrix[3] * tiltSin + modelMatrix[11] * tiltCos;
    }
    
    tiltedMatrix[12] = modelMatrix[12];
    tiltedMatrix[13] = modelMatrix[13];
    tiltedMatrix[14] = modelMatrix[14];
    tiltedMatrix[15] = modelMatrix[15];
    
    // Copy tilted matrix back
    memcpy(modelMatrix, tiltedMatrix, sizeof(modelMatrix));
  }

  // Apply final translation (equivalent to glTranslatef(-cycle->bbox[0] / 2, -cycle->bbox[1] / 2, 0))
  GLfloat finalMatrix[16];
  memcpy(finalMatrix, modelMatrix, sizeof(finalMatrix));
  finalMatrix[12] += modelMatrix[0] * (-cycle->bbox[0] / 2) + modelMatrix[4] * (-cycle->bbox[1] / 2);
  finalMatrix[13] += modelMatrix[1] * (-cycle->bbox[0] / 2) + modelMatrix[5] * (-cycle->bbox[1] / 2);
  finalMatrix[14] += modelMatrix[2] * (-cycle->bbox[0] / 2) + modelMatrix[6] * (-cycle->bbox[1] / 2);

  // Set the final model matrix in the shader
  setModelMatrix(prog, finalMatrix);

  // Calculate and set normal matrix for proper lighting
  setNormalMatrix(prog, finalMatrix);

  // Handle explosion/normal rendering (same logic as desktop)
  if(p->data->exp_radius == 0) {
    drawModel(cycle, MODEL_USE_MATERIAL, 0);
  } else if(p->data->exp_radius < EXP_RADIUS_MAX) {
    // Enable blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    float alpha = (float)(EXP_RADIUS_MAX - p->data->exp_radius) / (float)EXP_RADIUS_MAX;
    setMaterialAlphas(cycle, alpha);
    drawExplosion(cycle, p->data->exp_radius, MODEL_USE_MATERIAL, 0);
    
    // Disable blending if alpha is not globally enabled
    if(game->settings->show_alpha == 0) {
      glDisable(GL_BLEND);
    }
  }

  // Clean up state (equivalent to desktop cleanup)
  glDisable(GL_DEPTH_TEST);
  glDepthMask(GL_FALSE);

  // Clean up
#else
  // Desktop OpenGL code remains unchanged
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
  // For Android, use vertex buffers with unified helpers
  GLuint shaderProgram = ensure_basic_shader_bound();
  if (!shaderProgram) {
    __android_log_print(ANDROID_LOG_ERROR, "GLTron", "Failed to ensure shader for players");
    return;
  }
  ensure3D(shaderProgram);

  // Enable blending for trail gradient effect
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // Ensure depth testing is enabled for 3D rendering
  glEnable(GL_DEPTH_TEST);
  glDepthMask(GL_TRUE);

  // Bind white texture once for all untextured geometry
  glActiveTexture(GL_TEXTURE0);
  static GLuint s_white = 0;
  if (s_white == 0) s_white = createWhiteTexture();
  glBindTexture(GL_TEXTURE_2D, s_white);
  setTexture(shaderProgram, 0);

  for (i = 0; i < game->players; i++) {
    height = game->player[i].data->trail_height;

    if (height > 0) {
      // Position the quad at the player's position via model matrix
      GLfloat playerModel[16] = {
        1,0,0,0, 
        0,1,0,0, 
        0,0,1,0, 
        game->player[i].data->posx, game->player[i].data->posy, 0, 1
      };
      setModelMatrix(shaderProgram, playerModel);

      // Get player color
      float* cm = game->player[i].model->color_model;

      // Create quad vertices with color gradient (matching desktop)
      // The gradient fades from player color at origin to black at the trail end
      dir = game->player[i].data->dir;
      
      // Vertices with per-vertex colors for gradient effect
      // Format: x, y, z, r, g, b, a
      GLfloat vertices[] = {
        // Vertex 0: origin (full color)
        0, 0, 0,                           cm[0], cm[1], cm[2], 1.0f,
        // Vertex 1: trail end bottom (black/transparent)
        -dirsX[dir] * l, -dirsY[dir] * l, 0,     0.0f, 0.0f, 0.0f, 0.0f,
        // Vertex 2: trail end top (black/transparent)  
        -dirsX[dir] * l, -dirsY[dir] * l, height, 0.0f, 0.0f, 0.0f, 0.0f,
        // Vertex 3: origin top (full color)
        0, 0, height,                      cm[0], cm[1], cm[2], 1.0f
      };

      GLuint vbo = 0;
      glGenBuffers(1, &vbo);
      glBindBuffer(GL_ARRAY_BUFFER, vbo);
      glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

      GLint positionLoc = glGetAttribLocation(shaderProgram, "position");
      GLint normalLoc = glGetAttribLocation(shaderProgram, "normal");
      
      if (positionLoc >= 0) {
        glEnableVertexAttribArray(positionLoc);
        glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), 0);
      }

      // For gradient effect, we'll use the color data in vertices
      // Set the base color and let vertex colors modulate it
      setColor(shaderProgram, 1.0f, 1.0f, 1.0f, 1.0f);
      
      // Since we can't easily pass per-vertex colors in this shader setup,
      // we'll draw two triangles with different colors
      // First triangle (bottom half)
      setColor(shaderProgram, cm[0], cm[1], cm[2], 1.0f);
      glDrawArrays(GL_TRIANGLE_FAN, 0, 2);
      
      // Second part with fade
      setColor(shaderProgram, cm[0] * 0.5f, cm[1] * 0.5f, cm[2] * 0.5f, 0.5f);
      glDrawArrays(GL_TRIANGLE_FAN, 2, 2);

      // Provide a normal for lighting
      if (normalLoc >= 0) {
        // Normal points toward the player's direction for better lighting
        glVertexAttrib3f(normalLoc, dirsX[dir], dirsY[dir], 0.0f);
      }

      // Actually draw as a quad using TRIANGLE_FAN
      setColor(shaderProgram, cm[0], cm[1], cm[2], 1.0f);
      glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

      if (positionLoc >= 0) glDisableVertexAttribArray(positionLoc);
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glDeleteBuffers(1, &vbo);

      polycount++;
    }

    // Draw the cycle model if visible
    if (playerVisible(p, &(game->player[i]))) {
      if (game->settings->show_model && game->player[i].model && game->player[i].model->mesh)
        drawCycle(&(game->player[i]));
    }
  }

  // Restore depth state
  glDisable(GL_DEPTH_TEST);
  glDepthMask(GL_FALSE);
  
  if (game->settings->show_alpha != 1) glDisable(GL_BLEND);
#else
  // For desktop OpenGL
  glShadeModel(GL_SMOOTH);
  glEnable(GL_BLEND);

  // Enable lighting
  /* no fixed-function lighting on GLES2 */

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
  /* no fixed-function lighting on GLES2 */

  if(game->settings->show_alpha != 1) glDisable(GL_BLEND);
  glShadeModel(GL_FLAT);
#endif
}

void drawGlow(Player *p, gDisplay *d, float dim) {
#ifdef ANDROID
  // For Android, use vertex buffers with unified helpers
  GLuint shaderProgram = ensure_basic_shader_bound();
  if (!shaderProgram) return;
  ensure3D(shaderProgram);

  // Set transformation matrix to position glow at player location
  GLfloat modelMatrix[16] = {
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    p->data->posx, p->data->posy, 0, 1
  };
  setModelMatrix(shaderProgram, modelMatrix);

  // Bind white texture for solid color rendering
  glActiveTexture(GL_TEXTURE0);
  static GLuint s_white = 0;
  if (s_white == 0) s_white = createWhiteTexture();
  glBindTexture(GL_TEXTURE_2D, s_white);
  setTexture(shaderProgram, 0);

  // Set player color
  float* cm = p->model->color_model;
  setColor(shaderProgram, cm[0], cm[1], cm[2], 1.0f);

  // Build the glow fan vertices (center + 7 points around)
  // Matching the exact angles from desktop version
  GLfloat fanVertices[] = {
    // Center point
    0, TRAIL_HEIGHT/2, 0,
    // Points around the fan (matching desktop angles)
    dim * cosf(-0.2f * M_PI / 5.0f), TRAIL_HEIGHT/2 + dim * sinf(-0.2f * M_PI / 5.0f), 0,
    dim * cosf(1.0f * M_PI / 5.0f),  TRAIL_HEIGHT/2 + dim * sinf(1.0f * M_PI / 5.0f), 0,
    dim * cosf(2.0f * M_PI / 5.0f),  TRAIL_HEIGHT/2 + dim * sinf(2.0f * M_PI / 5.0f), 0,
    dim * cosf(3.0f * M_PI / 5.0f),  TRAIL_HEIGHT/2 + dim * sinf(3.0f * M_PI / 5.0f), 0,
    dim * cosf(4.0f * M_PI / 5.0f),  TRAIL_HEIGHT/2 + dim * sinf(4.0f * M_PI / 5.0f), 0,
    dim * cosf(5.2f * M_PI / 5.0f),  TRAIL_HEIGHT/2 + dim * sinf(5.2f * M_PI / 5.0f), 0
  };

  GLuint vbo = 0;
  glGenBuffers(1, &vbo);
  if (vbo == 0) {
    return;
  }
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(fanVertices), fanVertices, GL_STATIC_DRAW);

  GLint positionLoc = glGetAttribLocation(shaderProgram, "position");
  GLint normalLoc = glGetAttribLocation(shaderProgram, "normal");
  
  if (positionLoc >= 0) {
    glEnableVertexAttribArray(positionLoc);
    glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  }
  
  // Set normal pointing up for the glow
  if (normalLoc >= 0) {
    glVertexAttrib3f(normalLoc, 0.0f, 0.0f, 1.0f);
  }

  // Enable additive blending for glow effect
  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE);

  // Draw the main fan
  glDrawArrays(GL_TRIANGLE_FAN, 0, 7);
  polycount += 5;

  // Now draw the two additional triangles for the "tail" of the glow
  GLfloat triVertices[] = {
    // First triangle
    0, TRAIL_HEIGHT/2, 0,
    0, -TRAIL_HEIGHT/4, 0,
    dim * cosf(-0.2f * M_PI / 5.0f), TRAIL_HEIGHT/2 + dim * sinf(-0.2f * M_PI / 5.0f), 0,
    // Second triangle
    0, TRAIL_HEIGHT/2, 0,
    dim * cosf(5.2f * M_PI / 5.0f), TRAIL_HEIGHT/2 + dim * sinf(5.2f * M_PI / 5.0f), 0,
    0, -TRAIL_HEIGHT/4, 0
  };
  
  glBufferData(GL_ARRAY_BUFFER, sizeof(triVertices), triVertices, GL_STATIC_DRAW);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  polycount += 2;

  // Clean up
  if (positionLoc >= 0) glDisableVertexAttribArray(positionLoc);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glDeleteBuffers(1, &vbo);

  // Restore blend function
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  if (game->settings->show_alpha != 1) glDisable(GL_BLEND);
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
  float t = 4;  // Texture repeat factor

#ifdef ANDROID
  // For Android, use vertex buffers for rendering walls
  // Vertices with corrected texture coordinates matching desktop version
  // Format: x, y, z, u, v
  GLfloat vertices[] = {
    // First quad (bottom wall - facing inward/up)
    0.0f,  0.0f, 0.0f,     0.0f, 0.0f,
    0.0f,  0.0f, WALL_H,   0.0f, 1.0f,
    GSIZE, 0.0f, WALL_H,   t,    1.0f,  // Repeat texture 't' times horizontally
    GSIZE, 0.0f, 0.0f,     t,    0.0f,

    // Second quad (right wall - facing inward/left)
    GSIZE, 0.0f,  0.0f,    0.0f, 1.0f,
    GSIZE, 0.0f,  WALL_H,  1.0f, 0.0f,
    GSIZE, GSIZE, WALL_H,  t,    0.0f,  // Different texture mapping
    GSIZE, GSIZE, 0.0f,    0.0f, 1.0f,

    // Third quad (top wall - facing inward/down)
    GSIZE, GSIZE, 0.0f,    0.0f, 1.0f,
    GSIZE, GSIZE, WALL_H,  1.0f, 0.0f,
    0.0f,  GSIZE, WALL_H,  t,    0.0f,  // Repeat texture
    0.0f,  GSIZE, 0.0f,    0.0f, 1.0f,

    // Fourth quad (left wall - facing inward/right)
    0.0f, GSIZE, 0.0f,     0.0f, 1.0f,
    0.0f, GSIZE, WALL_H,   1.0f, 0.0f,
    0.0f, 0.0f,  WALL_H,   t,    0.0f,  // Repeat texture
    0.0f, 0.0f,  0.0f,     0.0f, 1.0f
  };

  GLushort indices[] = {
    0,1,2, 0,2,3,      // Bottom wall
    4,5,6, 4,6,7,      // Right wall
    8,9,10, 8,10,11,   // Top wall
    12,13,14, 12,14,15 // Left wall
  };

  // Bind shader program and set to 3D mode
  GLuint shaderProgram = ensure_basic_shader_bound();
  if (!shaderProgram) {
    return;
  }
  ensure3D(shaderProgram);

  // Set identity model matrix (walls are in world space)
  setIdentityMatrix(shaderProgram, MATRIX_MODEL);

  // Set color to white for full texture color
  setColor(shaderProgram, 1.0f, 1.0f, 1.0f, 1.0f);

  // Bind wall texture
  glActiveTexture(GL_TEXTURE0);
  if (game->screen->texWall != 0) {
    glBindTexture(GL_TEXTURE_2D, game->screen->texWall);
  } else {
    // Fallback to a default texture if wall texture not loaded
    static GLuint s_white = 0;
    if (s_white == 0) s_white = createWhiteTexture();
    glBindTexture(GL_TEXTURE_2D, s_white);
  }
  setTexture(shaderProgram, 0);

  // Enable face culling for efficiency (only draw inward-facing sides)
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);

  // Set blend function and enable blending
  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

  // Create and bind vertex buffer
  GLuint vbo = 0;
  glGenBuffers(1, &vbo);
  if (vbo == 0) {
    return;
  }
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  // Create and bind index buffer
  GLuint ibo = 0;
  glGenBuffers(1, &ibo);
  if (ibo == 0) {
    glDeleteBuffers(1, &vbo);
    return;
  }
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

  // Set up attributes
  GLint positionLoc = glGetAttribLocation(shaderProgram, "position");
  GLint texCoordLoc = glGetAttribLocation(shaderProgram, "texCoord");
  GLint normalLoc = glGetAttribLocation(shaderProgram, "normal");

  if (positionLoc >= 0) {
    glEnableVertexAttribArray(positionLoc);
    glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), 0);
  }

  if (texCoordLoc >= 0) {
    glEnableVertexAttribArray(texCoordLoc);
    glVertexAttribPointer(texCoordLoc, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
  }

  // Draw each wall separately with its proper normal
  for (int wall = 0; wall < 4; wall++) {
    // Set normal for this wall (pointing inward)
    if (normalLoc >= 0) {
      switch(wall) {
        case 0: glVertexAttrib3f(normalLoc, 0.0f, 1.0f, 0.0f); break;  // Bottom wall normal points up (+Y)
        case 1: glVertexAttrib3f(normalLoc, -1.0f, 0.0f, 0.0f); break; // Right wall normal points left (-X)
        case 2: glVertexAttrib3f(normalLoc, 0.0f, -1.0f, 0.0f); break; // Top wall normal points down (-Y)
        case 3: glVertexAttrib3f(normalLoc, 1.0f, 0.0f, 0.0f); break;  // Left wall normal points right (+X)
      }
    }
    
    // Draw this wall (6 indices per wall)
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (void*)(wall * 6 * sizeof(GLushort)));
  }

  // Clean up
  if (positionLoc >= 0) glDisableVertexAttribArray(positionLoc);
  if (texCoordLoc >= 0) glDisableVertexAttribArray(texCoordLoc);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glDeleteBuffers(1, &vbo);
  glDeleteBuffers(1, &ibo);

  // Restore state
  glDisable(GL_CULL_FACE);
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
  glTexCoord2f(0.0, 0.0); glVertex3f(0.0, 0.0, 0.0);
  glTexCoord2f(0.0, 1.0); glVertex3f(0.0, 0.0, WALL_H);
  glTexCoord2f(0.0, 1.0); glVertex3f(GSIZE, 0.0, WALL_H);
  glTexCoord2f(0.0, 0.0); glVertex3f(GSIZE, 0.0, 0.0);

  glTexCoord2f(0.0, 1.0); glVertex3f(GSIZE, 0.0, 0.0);
  glTexCoord2f(1.0, 0.0); glVertex3f(GSIZE, 0.0, WALL_H);
  glTexCoord2f(0.0, 0.0); glVertex3f(GSIZE, GSIZE, WALL_H);
  glTexCoord2f(0.0, 1.0); glVertex3f(GSIZE, GSIZE, 0.0);

  glTexCoord2f(0.0, 1.0); glVertex3f(GSIZE, GSIZE, 0.0);
  glTexCoord2f(1.0, 0.0); glVertex3f(GSIZE, GSIZE, WALL_H);
  glTexCoord2f(0.0, 0.0); glVertex3f(0.0, GSIZE, WALL_H);
  glTexCoord2f(0.0, 1.0); glVertex3f(0.0, GSIZE, 0.0);

  glTexCoord2f(0.0, 1.0); glVertex3f(0.0, GSIZE, 0.0);
  glTexCoord2f(1.0, 0.0); glVertex3f(0.0, GSIZE, WALL_H);
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
  // For Android, use shaders for rendering with debug logging
  if (!p || !p->data || !game || !game->screen) {
    __android_log_print(ANDROID_LOG_ERROR, "GLTron", "drawCam failed: missing player or game data");
    return;
  }

  GLuint shaderProgram = ensure_basic_shader_bound();
  if (!shaderProgram) {
    __android_log_print(ANDROID_LOG_ERROR, "GLTron", "Failed to bind shader for drawCam");
    return;
  }
  ensure3D(shaderProgram);

  // Set up projection matrix for 3D perspective
  GLfloat projectionMatrix[16];
  float aspect = (float)game->screen->vp_w / (float)game->screen->vp_h;
  float fov = 45.0f * M_PI / 180.0f; // Default FOV
  if (game->settings->fov > 0) {
    fov = game->settings->fov * M_PI / 180.0f;
  }
  
  // Create perspective projection matrix matching gluPerspective
  float nearPlane = 3.0f;
  float farPlane = (float)GSIZE;
  float f = 1.0f / tan(fov / 2.0f);
  
  // Clear projection matrix
  memset(projectionMatrix, 0, sizeof(projectionMatrix));
  
  projectionMatrix[0] = f / aspect;  // X scaling
  projectionMatrix[5] = f;           // Y scaling
  projectionMatrix[10] = (farPlane + nearPlane) / (nearPlane - farPlane);     // Z scaling
  projectionMatrix[11] = -1.0f;      // W coefficient for Z
  projectionMatrix[14] = (2.0f * farPlane * nearPlane) / (nearPlane - farPlane); // Z translation

  setProjectionMatrix(shaderProgram, projectionMatrix);

  // Use actual camera data if available, otherwise use defaults
  float camX, camY, camZ;
  float lookX, lookY, lookZ;
  
  if (p->camera && p->camera->camType != -1) {
    // Use actual camera position from player's camera
    camX = p->camera->cam[0];
    camY = p->camera->cam[1];
    camZ = p->camera->cam[2];
    
    // Use actual look-at target from player's camera
    lookX = p->camera->target[0];
    lookY = p->camera->target[1];
    lookZ = p->camera->target[2];
  } else {
    // Fallback to calculated camera position
    float camDist = 12.0f;
    float camHeight = 6.0f;
    
    float playerX = p->data->posx;
    float playerY = p->data->posy;
    float dirX = dirsX[p->data->dir];
    float dirY = dirsY[p->data->dir];
    
    camX = playerX - dirX * camDist;
    camY = playerY - dirY * camDist;
    camZ = camHeight;
    
    lookX = playerX + dirX * 5.0f;
    lookY = playerY + dirY * 5.0f;
    lookZ = 0.5f;
  }

  // Up vector (Z-up in GLTron)
  float upX = 0.0f, upY = 0.0f, upZ = 1.0f;

  // Create look-at view matrix (equivalent to gluLookAt)
  GLfloat viewMatrix[16];
  
  // Calculate forward vector (from camera to target)
  float forward[3] = {lookX - camX, lookY - camY, lookZ - camZ};
  float length = sqrt(forward[0] * forward[0] + forward[1] * forward[1] + forward[2] * forward[2]);
  if (length > 0.0f) {
    forward[0] /= length;
    forward[1] /= length;
    forward[2] /= length;
  }

  // Calculate right vector (cross product of forward and up)
  float right[3];
  right[0] = forward[1] * upZ - forward[2] * upY;
  right[1] = forward[2] * upX - forward[0] * upZ;
  right[2] = forward[0] * upY - forward[1] * upX;
  length = sqrt(right[0] * right[0] + right[1] * right[1] + right[2] * right[2]);
  if (length > 0.0f) {
    right[0] /= length;
    right[1] /= length;
    right[2] /= length;
  }

  // Calculate actual up vector (cross product of right and forward)
  float up[3];
  up[0] = right[1] * forward[2] - right[2] * forward[1];
  up[1] = right[2] * forward[0] - right[0] * forward[2];
  up[2] = right[0] * forward[1] - right[1] * forward[0];

  // Build view matrix (rotation + translation)
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

  // Translation part (dot product of negative camera position with basis vectors)
  viewMatrix[12] = -(right[0] * camX + right[1] * camY + right[2] * camZ);
  viewMatrix[13] = -(up[0] * camX + up[1] * camY + up[2] * camZ);
  viewMatrix[14] = forward[0] * camX + forward[1] * camY + forward[2] * camZ;
  viewMatrix[15] = 1.0f;

  // Set view matrix in shader once
  setViewMatrix(shaderProgram, viewMatrix);

  // Set model matrix to identity for world rendering
  setIdentityMatrix(shaderProgram, MATRIX_MODEL);

  // Set lighting position (equivalent to glLightfv(GL_LIGHT0, GL_POSITION, p->camera->cam))
  // Use camera position as light position
  setLightPosition(shaderProgram, camX, camY, camZ);

  // Set default lighting values if not already set
  setLightColor(shaderProgram, 1.0f, 1.0f, 1.0f);
  setAmbientLight(shaderProgram, 0.2f, 0.2f, 0.2f);

  // Draw scene (same order as desktop)
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
  // For Android, implement text rendering using font texture and shaders
  GLuint shaderProgram = ensure_basic_shader_bound();
  if (!shaderProgram) return;
  ensure2D(shaderProgram, d->vp_w, d->vp_h);

  // Set color (white)
  setColor(shaderProgram, 1.0f, 1.0f, 1.0f, 1.0f);

  // Bind font texture
  glActiveTexture(GL_TEXTURE0);
  if (game->screen->texFont == 0) {
    game->screen->texFont = createFontTexture();
  }
  glBindTexture(GL_TEXTURE_2D, game->screen->texFont);
  setTexture(shaderProgram, 0);

  // Calculate text dimensions
  int textLen = strlen(ai);
  float charSize = d->vp_w / (2.0f * textLen);  // Size to fit text in half screen width
  float x = d->vp_w / 4.0f;  // Start at 1/4 screen width
  float y = 10.0f;

  // Build all vertices for the entire string at once
  int vertexCount = textLen * 4 * 5;  // 4 vertices per char, 5 floats per vertex
  GLfloat *vertices = (GLfloat *)malloc(vertexCount * sizeof(GLfloat));
  if (!vertices) return;

  int vIndex = 0;
  float currentX = x;
  
  for (int i = 0; i < textLen; i++) {
    char c = ai[i];
    if (c < 32 || c > 127) c = 32;  // Fallback to space for invalid chars
    
    int charIndex = c - 32;
    float texX = (float)(charIndex % 16) / 16.0f;
    float texY = (float)(charIndex / 16) / 6.0f;
    float texW = 1.0f / 16.0f;
    float texH = 1.0f / 6.0f;

    // Bottom-left vertex
    vertices[vIndex++] = currentX;
    vertices[vIndex++] = y;
    vertices[vIndex++] = 0.0f;
    vertices[vIndex++] = texX;
    vertices[vIndex++] = texY;

    // Bottom-right vertex
    vertices[vIndex++] = currentX + charSize;
    vertices[vIndex++] = y;
    vertices[vIndex++] = 0.0f;
    vertices[vIndex++] = texX + texW;
    vertices[vIndex++] = texY;

    // Top-right vertex
    vertices[vIndex++] = currentX + charSize;
    vertices[vIndex++] = y + charSize;
    vertices[vIndex++] = 0.0f;
    vertices[vIndex++] = texX + texW;
    vertices[vIndex++] = texY + texH;

    // Top-left vertex
    vertices[vIndex++] = currentX;
    vertices[vIndex++] = y + charSize;
    vertices[vIndex++] = 0.0f;
    vertices[vIndex++] = texX;
    vertices[vIndex++] = texY + texH;

    currentX += charSize * 0.8f;  // Advance with slight overlap for better spacing
  }

  // Create indices for all quads
  int indexCount = textLen * 6;  // 6 indices per quad (2 triangles)
  GLushort *indices = (GLushort *)malloc(indexCount * sizeof(GLushort));
  if (!indices) {
    free(vertices);
    return;
  }

  for (int i = 0; i < textLen; i++) {
    int base = i * 4;
    int idx = i * 6;
    indices[idx]     = base;
    indices[idx + 1] = base + 1;
    indices[idx + 2] = base + 2;
    indices[idx + 3] = base;
    indices[idx + 4] = base + 2;
    indices[idx + 5] = base + 3;
  }

  // Create single VBO for all characters
  GLuint vbo = 0, ibo = 0;
  glGenBuffers(1, &vbo);
  glGenBuffers(1, &ibo);
  
  if (vbo == 0 || ibo == 0) {
    if (vbo) glDeleteBuffers(1, &vbo);
    if (ibo) glDeleteBuffers(1, &ibo);
    free(vertices);
    free(indices);
    return;
  }

  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(GLfloat), vertices, GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(GLushort), indices, GL_STATIC_DRAW);

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

  // Draw all characters in one call
  glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_SHORT, 0);

  // Clean up
  if (positionLoc >= 0) glDisableVertexAttribArray(positionLoc);
  if (texCoordLoc >= 0) glDisableVertexAttribArray(texCoordLoc);
  
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glDeleteBuffers(1, &vbo);
  glDeleteBuffers(1, &ibo);
  
  free(vertices);
  free(indices);
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
  GLuint shaderProgram = ensure_basic_shader_bound();
  if (!shaderProgram) return;
  ensure2D(shaderProgram, display->vp_w, display->vp_h);

  // Ensure font texture exists
  glActiveTexture(GL_TEXTURE0);
  if (game->screen->texFont == 0) {
    game->screen->texFont = createFontTexture();
  }
  glBindTexture(GL_TEXTURE_2D, game->screen->texFont);
  setTexture(shaderProgram, 0);

  // Helper function to render text - we'll inline this code for each text rendering
  // First, render the main message with animated color
  {
    const char* text = message;
    float startX = display->vp_w / 6.0f;
    float startY = 20.0f;
    float charSize = display->vp_w / (6.0f / 4.0f * strlen(message));
    float animColor = (sin(d) + 1) / 2;
    float r = 1.0f, g = animColor, b = animColor, a = 1.0f;
    int textLen = strlen(text);
    if (textLen == 0) return;

    // Set color for this text
    setColor(shaderProgram, r, g, b, a);

    // Build all vertices for the string
    int vertexCount = textLen * 4 * 5;  // 4 vertices per char, 5 floats per vertex
    GLfloat *vertices = (GLfloat *)malloc(vertexCount * sizeof(GLfloat));
    if (!vertices) return;

    int vIndex = 0;
    float currentX = startX;
    
    for (int i = 0; i < textLen; i++) {
      char c = text[i];
      if (c < 32 || c > 127) c = 32;  // Fallback to space
      
      int charIndex = c - 32;
      float texX = (float)(charIndex % 16) / 16.0f;
      float texY = (float)(charIndex / 16) / 6.0f;
      float texW = 1.0f / 16.0f;
      float texH = 1.0f / 6.0f;

      // Bottom-left
      vertices[vIndex++] = currentX;
      vertices[vIndex++] = startY;
      vertices[vIndex++] = 0.0f;
      vertices[vIndex++] = texX;
      vertices[vIndex++] = texY;

      // Bottom-right
      vertices[vIndex++] = currentX + charSize;
      vertices[vIndex++] = startY;
      vertices[vIndex++] = 0.0f;
      vertices[vIndex++] = texX + texW;
      vertices[vIndex++] = texY;

      // Top-right
      vertices[vIndex++] = currentX + charSize;
      vertices[vIndex++] = startY + charSize;
      vertices[vIndex++] = 0.0f;
      vertices[vIndex++] = texX + texW;
      vertices[vIndex++] = texY + texH;

      // Top-left
      vertices[vIndex++] = currentX;
      vertices[vIndex++] = startY + charSize;
      vertices[vIndex++] = 0.0f;
      vertices[vIndex++] = texX;
      vertices[vIndex++] = texY + texH;

      currentX += charSize * 0.8f;  // Advance with slight overlap
    }

    // Create indices
    int indexCount = textLen * 6;
    GLushort *indices = (GLushort *)malloc(indexCount * sizeof(GLushort));
    if (!indices) {
      free(vertices);
      return;
    }

    for (int i = 0; i < textLen; i++) {
      int base = i * 4;
      int idx = i * 6;
      indices[idx]     = base;
      indices[idx + 1] = base + 1;
      indices[idx + 2] = base + 2;
      indices[idx + 3] = base;
      indices[idx + 4] = base + 2;
      indices[idx + 5] = base + 3;
    }

    // Create buffers
    GLuint vbo = 0, ibo = 0;
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ibo);
    
    if (vbo && ibo) {
      glBindBuffer(GL_ARRAY_BUFFER, vbo);
      glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(GLfloat), vertices, GL_STATIC_DRAW);

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(GLushort), indices, GL_STATIC_DRAW);

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

      // Draw all characters at once
      glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_SHORT, 0);

      // Clean up
      if (positionLoc >= 0) glDisableVertexAttribArray(positionLoc);
      if (texCoordLoc >= 0) glDisableVertexAttribArray(texCoordLoc);
    }

    // Clean up buffers
    if (vbo) glDeleteBuffers(1, &vbo);
    if (ibo) glDeleteBuffers(1, &ibo);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    
    free(vertices);
    free(indices);
  }

  // Show hint for touch/mouse if needed
  if (game->settings->input_mode != 0) {
    const char* hint = "Tap to resume";
    float hintX = display->vp_w / 6.0f;
    float hintY = 20.0f + display->vp_h / 12.0f;
    float hintScale = display->vp_w / (8.0f * strlen(hint));
    
    // Inline text rendering for hint
    int textLen = strlen(hint);
    if (textLen > 0) {
      // Set color for hint text
      setColor(shaderProgram, 1.0f, 1.0f, 1.0f, 1.0f);
      
      // Build all vertices for the string
      int vertexCount = textLen * 4 * 5;  // 4 vertices per char, 5 floats per vertex
      GLfloat *vertices = (GLfloat *)malloc(vertexCount * sizeof(GLfloat));
      if (vertices) {
        int vIndex = 0;
        float currentX = hintX;
        
        for (int i = 0; i < textLen; i++) {
          char c = hint[i];
          if (c < 32 || c > 127) c = 32;  // Fallback to space
          
          int charIndex = c - 32;
          float texX = (float)(charIndex % 16) / 16.0f;
          float texY = (float)(charIndex / 16) / 6.0f;
          float texW = 1.0f / 16.0f;
          float texH = 1.0f / 6.0f;

          // Bottom-left
          vertices[vIndex++] = currentX;
          vertices[vIndex++] = hintY;
          vertices[vIndex++] = 0.0f;
          vertices[vIndex++] = texX;
          vertices[vIndex++] = texY;

          // Bottom-right
          vertices[vIndex++] = currentX + hintScale;
          vertices[vIndex++] = hintY;
          vertices[vIndex++] = 0.0f;
          vertices[vIndex++] = texX + texW;
          vertices[vIndex++] = texY;

          // Top-right
          vertices[vIndex++] = currentX + hintScale;
          vertices[vIndex++] = hintY + hintScale;
          vertices[vIndex++] = 0.0f;
          vertices[vIndex++] = texX + texW;
          vertices[vIndex++] = texY + texH;

          // Top-left
          vertices[vIndex++] = currentX;
          vertices[vIndex++] = hintY + hintScale;
          vertices[vIndex++] = 0.0f;
          vertices[vIndex++] = texX;
          vertices[vIndex++] = texY + texH;

          currentX += hintScale * 0.8f;  // Advance with slight overlap
        }

        // Create indices
        int indexCount = textLen * 6;
        GLushort *indices = (GLushort *)malloc(indexCount * sizeof(GLushort));
        if (indices) {
          for (int i = 0; i < textLen; i++) {
            int base = i * 4;
            int idx = i * 6;
            indices[idx]     = base;
            indices[idx + 1] = base + 1;
            indices[idx + 2] = base + 2;
            indices[idx + 3] = base;
            indices[idx + 4] = base + 2;
            indices[idx + 5] = base + 3;
          }

          // Create buffers
          GLuint vbo = 0, ibo = 0;
          glGenBuffers(1, &vbo);
          glGenBuffers(1, &ibo);
          
          if (vbo && ibo) {
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(GLfloat), vertices, GL_STATIC_DRAW);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(GLushort), indices, GL_STATIC_DRAW);

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

            // Draw all characters at once
            glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_SHORT, 0);

            // Clean up
            if (positionLoc >= 0) glDisableVertexAttribArray(positionLoc);
            if (texCoordLoc >= 0) glDisableVertexAttribArray(texCoordLoc);
          }

          // Clean up buffers
          if (vbo) glDeleteBuffers(1, &vbo);
          if (ibo) glDeleteBuffers(1, &ibo);
          glBindBuffer(GL_ARRAY_BUFFER, 0);
          glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
          
          free(indices);
        }
        free(vertices);
      }
    }
  }
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
