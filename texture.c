#include "gltron.h"

#include "sgi_texture.h"

void deleteTextures(gDisplay *d) {
  glDeleteTextures(1, &(d->texFloor));
  glDeleteTextures(1, &(d->texWall));
  glDeleteTextures(1, &(d->texGui));
  glDeleteTextures(1, &(d->texCrash));
}

void loadTexture(char *filename, int format) {
    char *path;
    sgi_texture *tex;

#ifdef ANDROID
    __android_log_print(ANDROID_LOG_INFO, "GLTron", "loadTexture: requesting '%s'", filename);
#endif
    path = getFullPath(filename);
#ifdef ANDROID
    __android_log_print(ANDROID_LOG_INFO, "GLTron", "loadTexture: resolved path '%s'", path ? path : "(null)");
#endif
    if(path != 0)
        tex = load_sgi_texture(path);
    else {
#ifdef ANDROID
        __android_log_print(ANDROID_LOG_ERROR, "GLTron", "loadTexture: getFullPath failed for '%s'", filename);
#endif
        fprintf(stderr, "fatal: could not load %s, exiting...\n", filename);
        exit(1);
    }
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Remove the GL_RGB16 check entirely
    glTexImage2D(GL_TEXTURE_2D, 0, format, tex->width, tex->height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, tex->data);
#ifdef ANDROID
    GLint boundTex = 0; glGetIntegerv(GL_TEXTURE_BINDING_2D, &boundTex);
    __android_log_print(ANDROID_LOG_INFO, "GLTron", "loadTexture: uploaded '%s' to GL id %d (%dx%d)", filename, boundTex, tex->width, tex->height);
#endif
    free(tex->data);
    free(tex);
}

void initTexture(gDisplay *d) {
    checkGLError("texture.c initTexture - start");

    /* floor texture */
    glGenTextures(1, &(d->texFloor));
    glBindTexture(GL_TEXTURE_2D, d->texFloor);
    loadTexture("gltron_floor.sgi", GL_RGB);  // Changed from GL_RGB16 to GL_RGB
    // Removed glTexEnvi calls
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    checkGLError("texture.c initTextures - floor");

    /* menu icon */
    glGenTextures(1, &(d->texGui));
    glBindTexture(GL_TEXTURE_2D, d->texGui);
    loadTexture("gltron.sgi", GL_RGBA);
    // Removed glTexEnvi calls
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    checkGLError("texture.c initTextures - gui");

    /* wall texture */
    glGenTextures(1, &(d->texWall));
    glBindTexture(GL_TEXTURE_2D, d->texWall);
    loadTexture("gltron_wall.sgi", GL_RGBA);
    // Removed glTexEnvi calls
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    /* crash texture */
    glGenTextures(1, &(d->texCrash));
    glBindTexture(GL_TEXTURE_2D, d->texCrash);
    loadTexture("gltron_crash.sgi", GL_RGBA);
    // Removed glTexEnvi calls
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    checkGLError("texture.c initTextures - end");
}
