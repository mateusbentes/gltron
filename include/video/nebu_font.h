#ifndef NEBU_FONT_H
#define NEBU_FONT_H

// Android stub for nebu_font.h

typedef struct {
    int dummy;
} nebu_Font;

// Basic font rendering functions
void nebu_font_init(void);
void nebu_font_render(const char* text, float x, float y, float scale);

#endif // NEBU_FONT_H

