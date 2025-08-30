#include <stdio.h>
#include <stdlib.h>
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

/* SGI Image File Format constants */
#define SGI_MAGIC 474
#define SGI_HEADER_SIZE 512

sgi_texture* load_sgi_texture(char *filename) {
    FILE *f = NULL;
    unsigned char buf[BUFSIZE];
    unsigned int x, y, bpc, zsize;
    long count, bytes;
    unsigned char *tmp = NULL;
    int i, j;
    sgi_texture *tex = NULL;
    char *resolved = NULL;

    // Validate input
    if (!filename) {
        fprintf(stderr, ERR_PREFIX "filename is NULL\n");
        return NULL;
    }

    // Get the full path of the file
    resolved = getFullPath(filename);
    if (!resolved) {
        fprintf(stderr, ERR_PREFIX "could not resolve file '%s'\n", filename);
        return NULL;
    }

    fprintf(stderr, ERR_PREFIX "attempting to load: '%s' (resolved: '%s')\n", filename, resolved);

    // Open the file
    f = fopen(resolved, "rb");
    if (!f) {
        fprintf(stderr, ERR_PREFIX "fopen('%s') failed: %s\n", resolved, strerror(errno));
        free(resolved);
        return NULL;
    }

    // Read the header
    size_t nread = fread(buf, 1, SGI_HEADER_SIZE, f);
    if (nread != SGI_HEADER_SIZE) {
        fprintf(stderr, ERR_PREFIX "failed to read SGI header from '%s' (read %zu bytes, expected %d)\n", 
                resolved, nread, SGI_HEADER_SIZE);
        fclose(f);
        free(resolved);
        return NULL;
    }

    // Check the magic number (big-endian format)
    unsigned short magic = (buf[0] << 8) | buf[1];
    if (magic != SGI_MAGIC) {
        fprintf(stderr, ERR_PREFIX "wrong magic: 0x%04x (expected 0x%04x) for file '%s'\n",
                magic, SGI_MAGIC, resolved);
        fclose(f);
        free(resolved);
        return NULL;
    }

    // Check for RLE compression (storage type)
    unsigned char storage = buf[2];
    if (storage != 0) {
        fprintf(stderr, ERR_PREFIX "RLE compression (storage=%d) not supported for file '%s'\n", 
                storage, resolved);
        fclose(f);
        free(resolved);
        return NULL;
    }

    // Check bytes per channel
    bpc = buf[3];
    if (bpc != 1) {
        fprintf(stderr, ERR_PREFIX "BPC is %d - only 1 byte per channel supported for file '%s'\n", 
                bpc, resolved);
        fclose(f);
        free(resolved);
        return NULL;
    }

    // Get dimensions (big-endian format)
    x = (buf[6] << 8) | buf[7];     // xsize
    y = (buf[8] << 8) | buf[9];     // ysize
    zsize = (buf[10] << 8) | buf[11]; // zsize (number of channels)

    fprintf(stderr, ERR_PREFIX "image dimensions: %dx%d, channels: %d, bpc: %d\n", 
            x, y, zsize, bpc);

    // Validate dimensions
    if (x == 0 || y == 0 || zsize == 0) {
        fprintf(stderr, ERR_PREFIX "invalid dimensions: %dx%d, channels: %d for file '%s'\n", 
                x, y, zsize, resolved);
        fclose(f);
        free(resolved);
        return NULL;
    }

    // Check for reasonable limits to prevent huge allocations
    if (x > 8192 || y > 8192 || zsize > 4) {
        fprintf(stderr, ERR_PREFIX "dimensions too large: %dx%d, channels: %d for file '%s'\n", 
                x, y, zsize, resolved);
        fclose(f);
        free(resolved);
        return NULL;
    }

    // Support both RGB (3 channels) and RGBA (4 channels)
    if (zsize != 3 && zsize != 4) {
        fprintf(stderr, ERR_PREFIX "unsupported number of channels: %d (only 3 or 4 supported) for file '%s'\n", 
                zsize, resolved);
        fclose(f);
        free(resolved);
        return NULL;
    }

    // Allocate texture structure
    tex = (sgi_texture*) malloc(sizeof(sgi_texture));
    if (!tex) {
        fprintf(stderr, ERR_PREFIX "out of memory allocating texture struct for file '%s'\n", resolved);
        fclose(f);
        free(resolved);
        return NULL;
    }

    // Initialize structure
    tex->data = NULL;
    tex->width = x;
    tex->height = y;
    tex->channels = zsize;

    // Calculate total size
    count = x * y * zsize * bpc;
    
    // Allocate texture data
    tex->data = malloc(count);
    if (!tex->data) {
        fprintf(stderr, ERR_PREFIX "out of memory allocating %ld bytes for texture data '%s'\n", 
                count, resolved);
        free(tex);
        fclose(f);
        free(resolved);
        return NULL;
    }

    // Allocate temporary buffer for reading
    tmp = (unsigned char*) malloc(count);
    if (!tmp) {
        fprintf(stderr, ERR_PREFIX "out of memory allocating temp buffer for file '%s'\n", resolved);
        free(tex->data);
        free(tex);
        fclose(f);
        free(resolved);
        return NULL;
    }

    // Read texture data
    bytes = fread(tmp, 1, count, f);
    if (bytes != count) {
        fprintf(stderr, ERR_PREFIX "failed to read texture data for file '%s' (read %ld bytes, expected %ld)\n", 
                resolved, bytes, count);
        free(tmp);
        free(tex->data);
        free(tex);
        fclose(f);
        free(resolved);
        return NULL;
    }

    // Reorder the data from planar (RRRR...GGGG...BBBB...AAAA) to interleaved (RGBA RGBA RGBA...)
    // SGI format stores each channel as a separate plane
    for (i = 0; i < x * y; i++) {
        for (j = 0; j < zsize; j++) {
            tex->data[i * zsize + j] = tmp[j * x * y + i];
        }
    }

    fprintf(stderr, ERR_PREFIX "successfully loaded texture '%s': %dx%d, %d channels, %ld bytes\n", 
            resolved, x, y, zsize, count);

    // Clean up and return
    free(tmp);
    fclose(f);
    free(resolved);
    return tex;
}

void unload_sgi_texture(sgi_texture *tex) {
    if (tex) {
        if (tex->data) {
            free(tex->data);
        }
        free(tex);
    }
}
