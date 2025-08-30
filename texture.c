#include "gltron.h"
#include "sgi_texture.h"

void deleteTextures(gDisplay *d) {
    if (d->texFloor != 0) {
        glDeleteTextures(1, &(d->texFloor));
        d->texFloor = 0;
    }
    if (d->texWall != 0) {
        glDeleteTextures(1, &(d->texWall));
        d->texWall = 0;
    }
    if (d->texGui != 0) {
        glDeleteTextures(1, &(d->texGui));
        d->texGui = 0;
    }
    if (d->texCrash != 0) {
        glDeleteTextures(1, &(d->texCrash));
        d->texCrash = 0;
    }
}

void loadTexture(char *filename, GLenum format) {
    char *path;
    sgi_texture *tex;
    
#ifdef ANDROID
    __android_log_print(ANDROID_LOG_INFO, "GLTron", "loadTexture: requesting '%s'", filename);
#endif
    
    path = getFullPath(filename);
    
#ifdef ANDROID
    __android_log_print(ANDROID_LOG_INFO, "GLTron", "loadTexture: resolved path '%s'", path ? path : "(null)");
#endif
    
    if (path == NULL) {
#ifdef ANDROID
        __android_log_print(ANDROID_LOG_ERROR, "GLTron", "loadTexture: getFullPath failed for '%s'", filename);
#endif
        fprintf(stderr, "fatal: could not load %s, exiting...\n", filename);
        exit(1);
    }
    
    tex = load_sgi_texture(path);
    if (tex == NULL) {
#ifdef ANDROID
        __android_log_print(ANDROID_LOG_ERROR, "GLTron", "loadTexture: load_sgi_texture failed for '%s'", path);
#endif
        fprintf(stderr, "fatal: could not load texture data from %s, exiting...\n", path);
        free(path);
        exit(1);
    }
    
    // Validate texture data
    if (tex->data == NULL || tex->width <= 0 || tex->height <= 0) {
#ifdef ANDROID
        __android_log_print(ANDROID_LOG_ERROR, "GLTron", "loadTexture: invalid texture data for '%s'", path);
#endif
        fprintf(stderr, "fatal: invalid texture data for %s\n", path);
        if (tex->data) free(tex->data);
        free(tex);
        free(path);
        exit(1);
    }
    
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    
    // Use appropriate internal format based on input format
    GLenum internalFormat;
    GLenum dataFormat;
    
    if (format == GL_RGB) {
        internalFormat = GL_RGB;
        dataFormat = GL_RGB;
    } else if (format == GL_RGBA) {
        internalFormat = GL_RGBA;
        dataFormat = GL_RGBA;
    } else {
        // Default fallback
        internalFormat = format;
        dataFormat = GL_RGBA;
    }
    
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, tex->width, tex->height, 0,
                 dataFormat, GL_UNSIGNED_BYTE, tex->data);
    
    // Check for OpenGL errors after texture upload
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
#ifdef ANDROID
        __android_log_print(ANDROID_LOG_ERROR, "GLTron", "loadTexture: glTexImage2D failed with error 0x%x for '%s'", error, filename);
#endif
        fprintf(stderr, "OpenGL error 0x%x loading texture %s\n", error, filename);
    }
    
#ifdef ANDROID
    GLint boundTex = 0; 
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &boundTex);
    __android_log_print(ANDROID_LOG_INFO, "GLTron", "loadTexture: uploaded '%s' to GL id %d (%dx%d)", 
                       filename, boundTex, tex->width, tex->height);
#endif
    
    free(tex->data);
    free(tex);
    free(path);
}

void initTexture(gDisplay *d) {
    checkGLError("texture.c initTexture - start");
    
    // Initialize texture IDs to 0
    d->texFloor = 0;
    d->texGui = 0;
    d->texWall = 0;
    d->texCrash = 0;
    
    /* floor texture */
    glGenTextures(1, &(d->texFloor));
    glBindTexture(GL_TEXTURE_2D, d->texFloor);
    loadTexture("gltron_floor.sgi", GL_RGB);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    checkGLError("texture.c initTextures - floor");
    
    /* menu icon */
    glGenTextures(1, &(d->texGui));
    glBindTexture(GL_TEXTURE_2D, d->texGui);
    loadTexture("gltron.sgi", GL_RGBA);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // Note: No wrap parameters for GUI texture (uses default GL_REPEAT)
    checkGLError("texture.c initTextures - gui");
    
    /* wall texture */
    glGenTextures(1, &(d->texWall));
    glBindTexture(GL_TEXTURE_2D, d->texWall);
    loadTexture("gltron_wall.sgi", GL_RGBA);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    checkGLError("texture.c initTextures - wall");
    
    /* crash texture */
    glGenTextures(1, &(d->texCrash));
    glBindTexture(GL_TEXTURE_2D, d->texCrash);
    loadTexture("gltron_crash.sgi", GL_RGBA);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    checkGLError("texture.c initTextures - crash");
    
    checkGLError("texture.c initTextures - end");
}
