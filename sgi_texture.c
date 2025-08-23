#include <stdio.h>
#include <stdlib.h>
#include <error.h>
#include <string.h>
#include <errno.h>

#ifndef WIN32
#include <unistd.h>
#endif

#include "sgi_texture.h"

// Ensure correct prototype is visible to avoid implicit int return issues
char* getFullPath(char *filename);

#define ERR_PREFIX "[load_sgi_texture] "
/* must be larger than 512 */
#define BUFSIZE 8192

/* todo: check for EOF errors */

sgi_texture* load_sgi_texture(char *filename) {
  FILE *f = NULL;
  unsigned char buf[BUFSIZE];
  unsigned int x, y, bpc, zsize;
  long count, bytes;
  unsigned char *tmp = NULL;
  int i, j;
  sgi_texture *tex = NULL;

  // Get the full path of the file
  char *resolved = getFullPath(filename);
  if(!resolved) {
    fprintf(stderr, ERR_PREFIX "could not resolve file '%s'\n", filename ? filename : "(null)");
    return NULL;
  }

  // Check the resolved pointer
  if (!resolved) {
    fprintf(stderr, ERR_PREFIX "internal error: resolved path is NULL for '%s'\n", filename ? filename : "(null)");
    return NULL;
  }

  // Open it
  f = fopen(resolved, "rb");
  if (!f) {
    fprintf(stderr, ERR_PREFIX "fopen('%s') failed: %s\n", resolved ? resolved : "(null)", strerror(errno));
    fprintf(stderr, ERR_PREFIX "failed to open file '%s' (resolved path: '%s')\n", filename ? filename : "(null)", resolved ? resolved : "(null)");
    free(resolved);
    return NULL;
  }

  // Print the resolved path for debugging only after fopen succeeds
  fprintf(stderr, ERR_PREFIX "resolved path: '%s'\n", resolved ? resolved : "(null)");
  if(!f) {
    fprintf(stderr, ERR_PREFIX);
    perror("fopen");
    fprintf(stderr, ERR_PREFIX "failed to open file '%s' (resolved path: '%s')\n", filename ? filename : "(null)", resolved ? resolved : "(null)");
    free(resolved);
    return NULL;
  }

  // Read the header
  size_t nread = fread(buf, 1, 512, f);
  if(nread != 512) {
    fprintf(stderr, ERR_PREFIX "failed to read SGI header from '%s' (resolved path: '%s')\n", filename ? filename : "(null)", resolved ? resolved : "(null)");
    fclose(f);
    free(resolved);
    return NULL;
  }

  // Check the magic number
  if((buf[0] << 8) + (buf[1] << 0) != 474) {
    fprintf(stderr, ERR_PREFIX "wrong magic: %d %d for file '%s' (resolved path: '%s')\n",
      buf[0], buf[1], filename ? filename : "(null)", resolved ? resolved : "(null)");
    fclose(f);
    free(resolved);
    return NULL;
  }

  // Check for RLE compression
  if(buf[2] != 0) {
    fprintf(stderr, ERR_PREFIX "RLE compression not supported for file '%s' (resolved path: '%s')\n", filename ? filename : "(null)", resolved ? resolved : "(null)");
    fclose(f);
    free(resolved);
    return NULL;
  }

  // Check bytes per channel
  if(buf[3] != 1) {
    fprintf(stderr, ERR_PREFIX "BPC is %d - not supported for file '%s' (resolved path: '%s')\n", buf[3], filename ? filename : "(null)", resolved ? resolved : "(null)");
    fclose(f);
    free(resolved);
    return NULL;
  }

  bpc = buf[3];

  // Check number of channels
  if((buf[10] << 8) + buf[11] != 4) {
    fprintf(stderr, ERR_PREFIX "number of channels is != 4 - not supported for file '%s' (resolved path: '%s')\n", filename ? filename : "(null)", resolved ? resolved : "(null)");
    fclose(f);
    free(resolved);
    return NULL;
  }

  zsize = (buf[10] << 8) + buf[11];

  // Get dimensions
  x = (buf[6] << 8) + buf[7];
  y = (buf[8] << 8) + buf[9];

  // Allocate texture structure
  tex = (sgi_texture*) malloc(sizeof(sgi_texture));
  if(!tex) {
    fprintf(stderr, ERR_PREFIX "out of memory allocating texture struct for file '%s' (resolved path: '%s')\n", filename ? filename : "(null)", resolved ? resolved : "(null)");
    fclose(f);
    free(resolved);
    return NULL;
  }

  // Allocate texture data
  tex->data = malloc(x * y * zsize * bpc);
  if(!tex->data) {
    fprintf(stderr, ERR_PREFIX "out of memory allocating texture data for file '%s' (resolved path: '%s')\n", filename ? filename : "(null)", resolved ? resolved : "(null)");
    free(tex);
    fclose(f);
    free(resolved);
    return NULL;
  }

  // Set texture properties
  tex->width = x;
  tex->height = y;
  tex->channels = zsize;

  // Read texture data
  count = x * y * zsize * bpc;
  /* fprintf(stderr, ERR_PREFIX "loading %ld bytes\n", count); */
  tmp = (unsigned char*) malloc(count);
  if(!tmp) {
    fprintf(stderr, ERR_PREFIX "out of memory allocating temp buffer for file '%s' (resolved path: '%s')\n", filename ? filename : "(null)", resolved ? resolved : "(null)");
    free(tex->data);
    free(tex);
    fclose(f);
    free(resolved);
    return NULL;
  }

  bytes = fread(tmp, count, 1, f);
  if(bytes != 1) {
    fprintf(stderr, ERR_PREFIX "failed to read texture data for file '%s' (resolved path: '%s')\n", filename ? filename : "(null)", resolved ? resolved : "(null)");
    free(tmp);
    free(tex->data);
    free(tex);
    fclose(f);
    free(resolved);
    return NULL;
  }

  // Reorder the data
  for(i = 0; i < x * y; i++) {
    for(j = 0; j < zsize; j++) {
      *(tex->data + i * zsize + j) = *(tmp + j * x * y + i);
      /* printf("%d -> %d\n", j * x * y + i, i * zsize + j); */
    }
  }

  // Clean up and return
  free(tmp);
  fclose(f);
  free(resolved);
  return tex;
}

void unload_sgi_texture(sgi_texture *tex) {
  free(tex->data);
  free(tex);
}
