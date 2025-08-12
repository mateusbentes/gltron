#include "video/nebu_renderer_gl.h"
#include "video/nebu_video_system.h"
#include "video/nebu_texture2d.h"
#include "video/nebu_png_texture.h"
#include "filesystem/nebu_filesystem.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "base/nebu_debug_memory.h"
#include "base/nebu_assert.h"

#include <SDL2/SDL.h>
#if defined(__ANDROID__)
  #include <GLES2/gl2.h>
#else
  #include <GL/gl.h>
#endif

static void loadTexture(const char *path, int format);
static png_texture* loadTextureData(const char *path);
static void freeTextureData(png_texture *tex);

void nebu_Texture2D_Free(nebu_Texture2D* pTexture)
{
    glDeleteTextures(1, &pTexture->id);
    free(pTexture);
}

nebu_Texture2D* nebu_Texture2D_Load(const char *path, const nebu_Texture2D_meta* meta)
{
    nebu_Texture2D *pTexture = (nebu_Texture2D*)malloc(sizeof(nebu_Texture2D));
    if (!pTexture) return NULL;

    glGenTextures(1, &pTexture->id);
    glBindTexture(GL_TEXTURE_2D, pTexture->id);

    loadTexture(path, meta->format);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, meta->min_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, meta->mag_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, meta->wrap_s);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, meta->wrap_t);

#if !defined(__ANDROID__)
    // On desktop OpenGL, check for anisotropic filtering support manually
    // Here we do a basic extension check without GLEW:
    const char *ext = (const char*)glGetString(GL_EXTENSIONS);
    if (ext && strstr(ext, "GL_EXT_texture_filter_anisotropic")) {
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, meta->anisotropy);
    }
#endif

    return pTexture;
}

void freeTextureData(png_texture *tex) {
    if (!tex) return;
    if (tex->data) free(tex->data);
    free(tex);
}

png_texture* loadTextureData(const char *path) {
    png_texture *tex = load_png_texture(path);

    if (tex == NULL) {
        fprintf(stderr, "fatal: failed loading %s, exiting...\n", path);
        nebu_assert(0);
        exit(1); // Critical error, abort program
    }
    return tex;
}

void loadTexture(const char *path, int format) {
    png_texture *tex;
    GLint internal;
    int maxSize;

    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxSize);

    tex = loadTextureData(path);

    if (tex->channels == 3)
        internal = GL_RGB;
    else
        internal = GL_RGBA;

#if defined(GL_DONT_CARE)
    if (format == GL_DONT_CARE)
#else
    if (format == 0)
#endif
    {
        if (tex->channels == 3) format = GL_RGB;
        if (tex->channels == 4) format = GL_RGBA;
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Upload mipmap levels
    {
        png_texture *newtex;
        int level = 0;
        while (tex->width > 1 || tex->height > 1)
        {
            if (tex->width <= maxSize && tex->height <= maxSize)
            {
                glTexImage2D(GL_TEXTURE_2D, level, internal,
                             tex->width, tex->height,
                             0, format, GL_UNSIGNED_BYTE, tex->data);
#ifdef PRINTF_VERBOSE
                printf("uploading level %d, %dx%d texture\n",
                       level, tex->width, tex->height);
#endif
                level++;
            }
            newtex = mipmap_png_texture(tex, 1, 0, 0);
            freeTextureData(tex);
            tex = newtex;
        }
        // Upload 1x1 mip level
        glTexImage2D(GL_TEXTURE_2D, level, internal,
                     tex->width, tex->height,
                     0, format, GL_UNSIGNED_BYTE, tex->data);
#ifdef PRINTF_VERBOSE
        printf("uploading level %d, %dx%d texture\n",
               level, tex->width, tex->height);
#endif
        freeTextureData(tex);
    }

#if defined(__ANDROID__)
    // Optionally generate mipmaps on OpenGLES
    // glGenerateMipmap(GL_TEXTURE_2D);
#endif
}
