#include <stdlib.h>
#include <stdio.h>
#include "common.h"
#include "sdl_helpers.h"
#include "console.h"

void render_glyph(SDL_Renderer *renderer, Charmap *charmap, uint8_t c, float x, float y)
{
    const SDL_Rect dst = {
        .x = (int) x,
        .y = (int) y,
        .w = 16,
        .h = 16
    };
    uint8_t index = c;
    SCC(SDL_RenderCopy(renderer, charmap->spritesheet, &charmap->glyph_table[index], &dst));
}

Charmap *charmap_loadFromFile(const char *filepath, SDL_Renderer *renderer, Uint32 colorKey)
{
    Charmap *charmap = malloc(sizeof(Charmap));
    (void) renderer;
    SDL_Surface *font_surface = SCP(get_suface_from_file(filepath));
    SDL_SetColorKey(font_surface, SDL_TRUE, colorKey);
    charmap->spritesheet = SCP(SDL_CreateTextureFromSurface(renderer, font_surface));
    SDL_FreeSurface(font_surface);

    for (size_t i = 0; i < CHARMAP_TABLE_SIZE; ++i) {
        const size_t col = i % FONT_COLS;
        const size_t row = i / FONT_ROWS;
        charmap->glyph_table[i] = (SDL_Rect) { .x = col * FONT_CHAR_WIDTH, .y = row * FONT_CHAR_HEIGHT, .w = FONT_CHAR_WIDTH, .h = FONT_CHAR_HEIGHT };
    }

    return (charmap);
}

// // Console functions definitions
// internal void consoleClear(CConsole *console)
// {
//     CRect r = { 0, 0, console->width, console->height };
//     (void) r;
//     // CFill(console->pixels, console->width, 0x000000ff);
// }

// internal CConsole *consoleNew(i32 width, i32 height, i32 rowCount, i32 colCount)
// {
//     CConsole *console = malloc(sizeof(CConsole));
//     console->pixels = calloc(width * height, sizeof(u32));
//     console->width = width;
//     console->height = height;
//     console->rowCount = rowCount;
//     console->colCount = colCount;
//     console->cellWidth = width / colCount;
//     console->cellHeight = height / rowCount;
//     console->font = NULL;
//     console->cells = calloc(rowCount * colCount, sizeof(CCell));

//     return console;
// }
