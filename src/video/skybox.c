// skybox.c
#include "video/video.h"
#include "game/game.h"
#include "configuration/settings.h"
#include "video/nebu_renderer_gl.h"
#include "video/skybox.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <png.h>


// Function to load a texture from a PNG file
GLuint loadTexture(const char* filename) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        fprintf(stderr, "Failed to open file: %s\n", filename);
        return 0;
    }

    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png) {
        fclose(fp);
        return 0;
    }

    png_infop info = png_create_info_struct(png);
    if (!info) {
        png_destroy_read_struct(&png, NULL, NULL);
        fclose(fp);
        return 0;
    }

    if (setjmp(png_jmpbuf(png))) {
        png_destroy_read_struct(&png, &info, NULL);
        fclose(fp);
        return 0;
    }

    png_init_io(png, fp);
    png_read_info(png, info);

    int width = png_get_image_width(png, info);
    int height = png_get_image_height(png, info);
    png_byte color_type = png_get_color_type(png, info);
    png_byte bit_depth = png_get_bit_depth(png, info);

    if (bit_depth == 16)
        png_set_strip_16(png);

    if (color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_palette_to_rgb(png);

    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
        png_set_expand_gray_1_2_4_to_8(png);

    if (png_get_valid(png, info, PNG_INFO_tRNS))
        png_set_tRNS_to_alpha(png);

    if (color_type == PNG_COLOR_TYPE_RGB ||
        color_type == PNG_COLOR_TYPE_GRAY ||
        color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_filler(png, 0xFF, PNG_FILLER_AFTER);

    if (color_type == PNG_COLOR_TYPE_GRAY ||
        color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
        png_set_gray_to_rgb(png);

    png_read_update_info(png, info);

    png_bytep* row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * height);
    for (int y = 0; y < height; y++) {
        row_pointers[y] = (png_byte*)malloc(png_get_rowbytes(png, info));
    }

    png_read_image(png, row_pointers);

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    GLenum format;
    if (color_type == PNG_COLOR_TYPE_RGB)
        format = GL_RGB;
    else if (color_type == PNG_COLOR_TYPE_RGBA)
        format = GL_RGBA;

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, row_pointers[0]);
    glGenerateMipmap(GL_TEXTURE_2D);

    for (int y = 0; y < height; y++) {
        free(row_pointers[y]);
    }
    free(row_pointers);

    png_destroy_read_struct(&png, &info, NULL);
    fclose(fp);

    return texture;
}

static void bindSkyboxTexture(int index) {
    glBindTexture(GL_TEXTURE_2D, gScreen->textures[TEX_SKYBOX0 + index]);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
}

void drawSkybox(Skybox *skybox) {
    /* these are the values for y == up, x == front */
    /*
    float sides[6][4][3] = {
        { { 1, -1, -1 }, { 1, -1, 1 }, { 1, 1, 1 },  { 1, 1, -1 } }, // front
        { { 1, 1, -1 }, { 1, 1, 1 }, { -1, 1, 1 }, { -1, 1, -1 } }, // top
        { { -1, -1, -1 }, { 1, -1, -1 },  { 1, 1, -1 }, { -1, 1, -1 } }, // left
        { { 1, -1, 1 }, { -1, -1, 1 }, { -1, 1, 1 }, { 1, 1, 1 } }, // right
        { { -1, -1, -1 }, { 1, -1, -1 }, { 1, -1, 1 }, { -1, -1, 1 } }, // bottom
        { { -1, -1, 1 }, { -1, -1, -1 }, { -1, 1, -1 }, { -1, 1, a1 } } // back
        };
    */

    /* these values are for z == up, x == front */
    float sides[6][12] = {
        { 1, 1, -1,
          1, -1, -1,
          1, 1, 1,
          1, -1, 1 }, /* front */
        { 1, 1, 1,
         -1, 1, 1,
         1, -1, 1,
         -1, -1, 1 }, /* top */
        { -1, 1, -1,
          1, 1, -1,
          -1, 1, 1,
          1, 1, 1 }, /* left */
        { 1, -1, -1,
        -1, -1, -1,
        1, -1, 1,
        -1, -1, 1 }, /* right */
        { -1, 1, -1,
          -1, -1, -1,
          1, 1, -1,
          1, -1, -1 }, /* bottom */
        { -1, -1, -1,
          -1, 1, -1,
          -1, -1, 1,
          -1, 1, 1 } /* back */
    };

    float uv[4][2] = { 0, 0, 1, 0, 0, 1, 1, 1 };
    int i;

    if (!gSettingsCache.show_skybox)
        return;

    glEnable(GL_TEXTURE_2D);
    glColor4f(1.0, 1.0, 1.0, 1.0f);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, 0, uv);
    glPushMatrix();
    glScalef(1.0f, 1.0f, 1.0f);

    for (i = 0; i < 6; i++) {
        bindSkyboxTexture(i);
        glVertexPointer(3, GL_FLOAT, 0, sides[i]);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
}

void loadSkyboxTextures(Skybox *skybox, const char* filenames[6]) {
    for (int i = 0; i < 6; i++) {
        gScreen->textures[TEX_SKYBOX0 + i] = loadTexture(filenames[i]);
        if (!gScreen->textures[TEX_SKYBOX0 + i]) {
            fprintf(stderr, "[FATAL] Failed to load skybox texture: %s\n", filenames[i]);
            exit(EXIT_FAILURE);
        }
    }
}