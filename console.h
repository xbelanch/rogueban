#ifndef INCLUDE_CONSOLE_H
#define INCLUDE_CONSOLE_H
#include <stdint.h>
#include <SDL2/SDL.h>
#include "common.h"

#define FONT_COLS 16
#define FONT_ROWS 16
#define FONT_WIDTH 16
#define FONT_HEIGHT 16
#define FONT_CHAR_WIDTH 16
#define FONT_CHAR_HEIGHT 16
#define CHARMAP_TABLE_SIZE 256

typedef unsigned char asciiChar;

typedef struct {
    asciiChar glyph;
    u32 fgColor;
    u32 bgColor;
} Cell;

typedef struct {
    SDL_Texture *spritesheet;
    SDL_Rect glyph_table[CHARMAP_TABLE_SIZE];
} Charmap;

Charmap *charmap_loadFromFile(const char *filepath, SDL_Renderer *renderer, Uint32 colorKey);
void render_glyph(SDL_Renderer *renderer, Charmap *charmap, uint8_t c, float x, float y);

#endif
