#include "fonttex.h"
#include <string.h>

#ifdef ANDROID
#include <GLES2/gl2.h>
#include "shaders.h"
#else
#include <GL/gl.h>
#endif

#define FTX_ERR "[fonttex error]: "
extern char *getFullPath(char*);

void getLine(char *buf, int size, FILE *f) {
  do {
    fgets(buf, size, f);
  } while( buf[0] == '\n' || buf[0] == '#');
}

fonttex *ftxLoadFont(char *filename) {
  char *path;
  FILE *file;
  char buf[100];
  char texname[100];
  int i;

  fonttex *ftx;

  path = getFullPath(filename);
  if(path == 0) {
    fprintf(stderr, FTX_ERR "can't load font file '%s'\n", filename);
    return 0;
  }

  file = fopen(path, "r");
  free(path);

  // TODO(5): check for EOF errors in the following code

  ftx = (fonttex*) malloc(sizeof(fonttex));
  getLine(buf, sizeof(buf), file);
  sscanf(buf, "%d %d %d ", &(ftx->nTextures), &(ftx->texwidth), &(ftx->width));
  getLine(buf, sizeof(buf), file);
  sscanf(buf, "%d %d ", &(ftx->lower), &(ftx->upper));
  getLine(buf, sizeof(buf), file);
  ftx->fontname = malloc(strlen(buf) + 1);
  memcpy(ftx->fontname, buf, strlen(buf) + 1);

  ftx->textures = (sgi_texture**)
    malloc(ftx->nTextures * sizeof(sgi_texture*));
  for(i = 0; i < ftx->nTextures; i++) {
    getLine(buf, sizeof(buf), file);

    // no spaces in texture filesnames
    sscanf(buf, "%s ", texname);
    path = getFullPath(texname);
    if(path == 0) {
      // clean up allocated memory & spit out error
      int j;
      for(j = 0; j < i; j++)
        unload_sgi_texture(*(ftx->textures + j));
      free(ftx->textures);
      free(ftx->fontname);
      free(ftx);
      fprintf(stderr, FTX_ERR "can't load texture file '%s'\n", texname);
      return 0;
    }
    *(ftx->textures + i) = load_sgi_texture(path);
    free(path);
  }
  return ftx;
}

void ftxUnloadFont(fonttex *ftx) {
  int i;
  for(i = 0; i < ftx->nTextures; i++)
    unload_sgi_texture(*(ftx->textures + i));
  free(ftx->textures);
  free(ftx->texID);
  free(ftx->fontname);
  free(ftx);
}

void ftxEstablishTexture(fonttex *ftx, unsigned char setupMipmaps) {
    /* TODO(1): add support for mipmaps */
    int i;

    ftx->texID = (unsigned int*) malloc(ftx->nTextures * sizeof(unsigned int));
    glGenTextures(ftx->nTextures, ftx->texID);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    for(i = 0; i < ftx->nTextures; i++) {
        glBindTexture(GL_TEXTURE_2D, ftx->texID[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                     (*(ftx->textures + i))->width, (*(ftx->textures + i))->height,
                     0, GL_RGBA, GL_UNSIGNED_BYTE,
                     (*(ftx->textures + i))->data);
        // Removed glTexEnvi calls
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  // Changed from GL_CLAMP
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);  // Changed from GL_CLAMP
    }
}

void ftxRenderString(fonttex *ftx, char *string, int len) {
    int i;
    int bound = -1;
    int index;

    int tex;
    int w;
    float cw;
    float cx, cy;

    w = ftx->texwidth / ftx->width;
    cw = (float)ftx->width / (float)ftx->texwidth;

    // Set up orthographic projection for text rendering
    GLfloat projection[16] = {
        2.0f/len, 0, 0, 0,
        0, 2.0f, 0, 0,
        0, 0, -1, 0,
        -1, -1, 0, 1
    };

    // Set up model-view matrix (identity)
    GLfloat modelView[16] = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };

    // Set up vertex data
    GLfloat vertices[8];
    GLfloat texCoords[8];

    // Enable texture and set up shader (for Android)
#ifdef ANDROID
    // Get shader program (you'll need to store this somewhere)
    extern GLuint shaderProgram; // Assuming this is defined elsewhere
    useShaderProgram(shaderProgram);

    // Set up matrices
    setProjectionMatrix(shaderProgram, projection);
    setModelMatrix(shaderProgram, modelView);
    setViewMatrix(shaderProgram, modelView);

    // Set color to white
    setColor(shaderProgram, 1.0f, 1.0f, 1.0f, 1.0f);

    // Set texture unit
    setTexture(shaderProgram, 0);

    // Get attribute locations
    GLint positionLoc = glGetAttribLocation(shaderProgram, "position");
    GLint texCoordLoc = glGetAttribLocation(shaderProgram, "texCoord");

    // Enable attributes
    glEnableVertexAttribArray(positionLoc);
    glEnableVertexAttribArray(texCoordLoc);
#endif

    for(i = 0; i < len; i++) {
        /* find out which texture it's in */
        index = string[i] - ftx->lower + 1;
        if(index >= ftx->upper) {
            fprintf(stderr, FTX_ERR " index out of bounds");
            continue;
        }
        tex = index / (w * w);

        /* bind texture */
        if(tex != bound) {
            glBindTexture(GL_TEXTURE_2D, ftx->texID[tex]);
            bound = tex;
        }

        /* find texture coordinates */
        index = index % (w * w);
        cx = (float)(index % w) / (float)w;
        cy = (float)(index / w) / (float)w;

        // Set up vertex data
        vertices[0] = i; vertices[1] = 0;
        vertices[2] = i + 1; vertices[3] = 0;
        vertices[4] = i + 1; vertices[5] = 1;
        vertices[6] = i; vertices[7] = 1;

        texCoords[0] = cx; texCoords[1] = 1 - cy - cw;
        texCoords[2] = cx + cw; texCoords[3] = 1 - cy - cw;
        texCoords[4] = cx + cw; texCoords[5] = 1 - cy;
        texCoords[6] = cx; texCoords[7] = 1 - cy;

#ifdef ANDROID
        // For Android, use vertex attributes
        glVertexAttribPointer(positionLoc, 2, GL_FLOAT, GL_FALSE, 0, vertices);
        glVertexAttribPointer(texCoordLoc, 2, GL_FLOAT, GL_FALSE, 0, texCoords);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
#else
        // For desktop, use immediate mode
        glBegin(GL_QUADS);
        glTexCoord2f(cx, 1 - cy - cw);
        glVertex2f(i, 0);
        glTexCoord2f(cx + cw, 1 - cy - cw);
        glVertex2f(i + 1, 0);
        glTexCoord2f(cx + cw, 1 - cy);
        glVertex2f(i + 1, 1);
        glTexCoord2f(cx, 1 - cy);
        glVertex2f(i, 1);
        glEnd();
#endif
    }

#ifdef ANDROID
    // Disable attributes
    glDisableVertexAttribArray(positionLoc);
    glDisableVertexAttribArray(texCoordLoc);
#endif
}
