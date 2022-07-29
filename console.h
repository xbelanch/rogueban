#ifndef INCLUDE_CONSOLE_H
#define INCLUDE_CONSOLE_H
#include <stdint.h>
#include <SDL2/SDL.h>
#include "common.h"

// @TODO: Drop this (29-07-2022)
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
    uint32_t *atlas;
    uint32_t atlasWidth;
    uint32_t atlasHeight;
    uint32_t charWidth;
    uint32_t charHeight;
    asciiChar firstCharInAtlas;
} Font;

typedef struct {
    uint32_t *pixels;
    uint32_t width;
    uint32_t height;
    uint32_t rowCount;
    uint32_t colCount;
    uint32_t cellWidth;
    uint32_t cellHeight;
    Font *font;
    Cell *cells;
} Console;

// @TODO: Drop this (29-07-2022)
typedef struct {
    SDL_Texture *spritesheet;
    SDL_Rect glyph_table[CHARMAP_TABLE_SIZE];
} Charmap;

Console *console_new(uint32_t width, uint32_t height, uint32_t rowCount, uint32_t colCount);
void console_setBitmapFont(Console *console, char *filename, asciiChar firstCharinAtlas, int charWidth, int charHeight);
void console_putGlyphAt(SDL_Renderer *renderer, SDL_Texture *texture, Console *con, asciiChar chr, uint32_t cellX, uint32_t cellY, uint32_t fgColor, uint32_t bgColor);

#endif
