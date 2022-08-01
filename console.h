#ifndef INCLUDE_CONSOLE_H
#define INCLUDE_CONSOLE_H
#include <stdint.h>
#include <SDL2/SDL.h>

typedef unsigned char asciiChar;

typedef struct {
    asciiChar glyph;
    uint32_t fgColor;
    uint32_t bgColor;
} Cell;

typedef struct {
    uint32_t *atlas;
    uint32_t atlasWidth;
    uint32_t atlasHeight;
    uint32_t charWidth;
    uint32_t charHeight;
    asciiChar firstCharInAtlas;
    uint32_t chromaKey;
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

Console *console_new(uint32_t width, uint32_t height, uint32_t rowCount, uint32_t colCount);
void console_clear(Console *console);
void console_setBitmapFont(Console *console, char *filename, asciiChar firstCharinAtlas, int charWidth, int charHeight);
void console_dumpTileset(Console *con);
void console_putGlyphAt(Console *con, asciiChar chr, uint32_t cellX, uint32_t cellY, uint32_t fgColor, uint32_t bgColor);

#endif
