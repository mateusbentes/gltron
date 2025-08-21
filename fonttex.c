#include "fonttex.h"

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
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    /*glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); */
    /*glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  }
  /* checkGLError(FTX_ERR " establishing textures"); */
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

  for(i = 0; i < len; i++) {
    /* find out which texture it's in */
    /* TODO(4): find out why the +1 is necessary */
    index = string[i] - ftx->lower + 1;
    if(index >= ftx->upper) 
      fprintf(stderr, FTX_ERR " index out of bounds");
    tex = index / (w * 
w);
    /* bind texture */
    if(tex != bound) {
      glBindTexture(GL_TEXTURE_2D, ftx->texID[tex]);
      bound = tex;
    }
    /* find texture coordinates */
    index = index % (w * w);
    cx = (float)(index % w) / (float)w;
    cy = (float)(index / w) / (float)w;
    /* draw quad */
    /* fprintf(stderr, FTX_ERR "coords: tex %d (%.2f, %.2f), %.2f\n", */
    /*     bound, cx, cy, cw); */

    #ifdef ANDROID
    {
      GLfloat verts[8] = { i,0, i+1,0, i+1,1, i,1 };
      GLfloat uvs[8] = { cx,1-cy-cw, cx+cw,1-cy-cw, cx+cw,1-cy, cx,1-cy };
      GLubyte idx[6] = {0,1,2, 0,2,3};
      glEnableClientState(GL_VERTEX_ARRAY);
      glEnableClientState(GL_TEXTURE_COORD_ARRAY);
      glVertexPointer(2, GL_FLOAT, 0, verts);
      glTexCoordPointer(2, GL_FLOAT, 0, uvs);
      glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, idx);
      glDisableClientState(GL_TEXTURE_COORD_ARRAY);
      glDisableClientState(GL_VERTEX_ARRAY);
    }
#else
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
  /* checkGLError("fonttex.c ftxRenderString\n"); */
}

