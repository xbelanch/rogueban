#include <stdlib.h>
#include <stdio.h>
#include "common.h"
#include "sdl_helpers.h"
#include "console.h"

void set_texture_color(SDL_Texture *texture, Uint32 color)
{
    SCC(SDL_SetTextureColorMod(texture, 0x00, 0xff, 0x00));
    SCC(SDL_SetTextureAlphaMod(texture, 0xff));
}

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
    SDL_SetColorKey(font_surface, SDL_TRUE, 0xff00ff);
    charmap->spritesheet = SCP(SDL_CreateTextureFromSurface(renderer, font_surface));
    set_texture_color(charmap->spritesheet, U32_PURPLE);
    SDL_FreeSurface(font_surface);

    for (size_t i = 0; i < CHARMAP_TABLE_SIZE; ++i) {
        const size_t col = i % FONT_COLS;
        const size_t row = i / FONT_ROWS;
        charmap->glyph_table[i] = (SDL_Rect) { .x = col * FONT_CHAR_WIDTH, .y = row * FONT_CHAR_HEIGHT, .w = FONT_CHAR_WIDTH, .h = FONT_CHAR_HEIGHT };
    }

    return (charmap);
}
