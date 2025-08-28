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

  path = (char*)getFullPath(filename);  // Explicit cast
  if(path == 0) {
    fprintf(stderr, FTX_ERR "can't load font file '%s'\n", filename);
    return 0;
  }

  file = fopen(path, "rb");
  if(!file) {
    fprintf(stderr, FTX_ERR "fopen failed for font '%s'\n", filename);
    perror(FTX_ERR "fopen");
    free(path);
    return 0;
  }
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
    sscanf(buf, "%99s", texname);
    // sanitize: strip trailing CR/LF and whitespace just in case
    {
      size_t L = strlen(texname);
      while(L > 0 && (texname[L-1] == '\r' || texname[L-1] == '\n' || texname[L-1] == ' ' || texname[L-1] == '\t')) {
        texname[--L] = '\0';
      }
    }
    // resolve the parsed texture filename, not the .ftx file name
    path = (char*)getFullPath(texname);  // Explicit cast
    if(path == 0) {
      // clean up allocated memory & spit out error
      int j;
      for(j = 0; j < i; j++)
        unload_sgi_texture(*(ftx->textures + j));
      free(ftx->textures);
      free(ftx->fontname);
      free(ftx);
      fprintf(stderr, FTX_ERR "can't resolve texture file '%s'\n", texname);
      fclose(file);
      return 0;
    }
    // sanity check: avoid passing .ftx here
    {
      const char *dot = strrchr(texname, '.');
      if (!dot || strcmp(dot, ".sgi") != 0) {
        fprintf(stderr, FTX_ERR "unexpected texture filename '%s' (expect .sgi) in font '%s'\n", texname, filename);
      }
      fprintf(stderr, "[fonttex] loading texture '%s'\n", texname);
    }
    *(ftx->textures + i) = load_sgi_texture(path);
    free(path);
    if(!*(ftx->textures + i)) {
      int j;
      fprintf(stderr, FTX_ERR "Failed to load texture '%s' listed in font file '%s'\n", texname, filename);
      for(j = 0; j < i; j++)
        unload_sgi_texture(*(ftx->textures + j));
      free(ftx->textures);
      free(ftx->fontname);
      free(ftx);
      fclose(file);
      return 0;
    }
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
    if (w <= 0) w = 1;
    cw = (float)ftx->width / (float)ftx->texwidth;

#ifdef ANDROID
    GLuint sp = shader_get_basic();
    if (!sp) return;
    useShaderProgram(sp);
    setColor(sp, 1.0f, 1.0f, 1.0f, 1.0f);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, (ftx && ftx->texID) ? ftx->texID[0] : 0);
    setTexture(sp, 0);

    GLint positionLoc = glGetAttribLocation(sp, "position");
    GLint texCoordLoc = glGetAttribLocation(sp, "texCoord");
    if (positionLoc < 0 || texCoordLoc < 0) return;
    glEnableVertexAttribArray(positionLoc);
    glEnableVertexAttribArray(texCoordLoc);

    GLfloat vertices[8];
    GLfloat texCoords[8];
#endif

    /* Ensure texture is bound */
    glActiveTexture(GL_TEXTURE0);
    if (ftx && ftx->texID) {
        glBindTexture(GL_TEXTURE_2D, ftx->texID[0]);
    }

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

#ifdef ANDROID
        /* Unit quad; caller scales via model matrix */
        vertices[0] = (float)i;     vertices[1] = 0.0f;
        vertices[2] = (float)i + 1; vertices[3] = 0.0f;
        vertices[4] = (float)i + 1; vertices[5] = 1.0f;
        vertices[6] = (float)i;     vertices[7] = 1.0f;

        texCoords[0] = cx; texCoords[1] = 1 - cy - cw;
        texCoords[2] = cx + cw; texCoords[3] = 1 - cy - cw;
        texCoords[4] = cx + cw; texCoords[5] = 1 - cy;
        texCoords[6] = cx; texCoords[7] = 1 - cy;

        glVertexAttribPointer(positionLoc, 2, GL_FLOAT, GL_FALSE, 0, vertices);
        glVertexAttribPointer(texCoordLoc, 2, GL_FLOAT, GL_FALSE, 0, texCoords);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
#else
        // For desktop, use immediate mode with unit quad
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
    glDisableVertexAttribArray(positionLoc);
    glDisableVertexAttribArray(texCoordLoc);
#endif
}
