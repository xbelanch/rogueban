#include <stdlib.h>
#include <stdio.h>
#include "common.h"
#include "sdl_helpers.h"
#include "console.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STBI_ONLY_PNG

Console *console_new(uint32_t width,
                     uint32_t height,
                     uint32_t rowCount,
                     uint32_t colCount)
{
    Console *console = malloc(sizeof(Console));

    console->pixels = calloc(width * height, sizeof(uint32_t));
    console->width = width;
    console->height = height;
    console->rowCount = rowCount;
    console->colCount = colCount;
    console->cellWidth = width / colCount;
    console->cellHeight = height / rowCount;
    console->font = NULL;
    console->cells = calloc(colCount * rowCount, sizeof(Cell));

    return console;
}

void console_setBitmapFont(Console *console,
                           char *filename,
                           asciiChar firstCharinAtlas,
                           int charWidth, int charHeight)
{
    // Load image data
    int req_format = STBI_rgb_alpha;
    int imgWidth, imgHeight, orig_format;
    unsigned char* pixelsImg = stbi_load(filename, &imgWidth, &imgHeight, &orig_format, req_format);
    if(pixelsImg == NULL) {
        SDL_Log("Loading image failed: %s", stbi_failure_reason());
        exit(1);
    }

    // Copy image data
    uint32_t atlasSize = imgWidth * imgHeight * sizeof(uint32_t);
    uint32_t *atlasData = malloc(atlasSize);
    memcpy(atlasData, pixelsImg, atlasSize);

    // Create and configure the font
    Font *font = malloc(sizeof(Font));
    font->atlas = atlasData;
    font->atlasWidth = imgWidth;
    font->atlasHeight = imgHeight;
    font->charWidth = charWidth;
    font->charHeight = charHeight;
    font->firstCharInAtlas = firstCharinAtlas;

    stbi_image_free(pixelsImg);
    if (console->font != NULL) {
        free(console->font->atlas);
        free(console->font);
    }

    console->font = font;
}

void set_texture_color(SDL_Texture *texture, Uint32 color)
{
    SCC(SDL_SetTextureColorMod(texture, 0x00, 0xff, 0x00));
    SCC(SDL_SetTextureAlphaMod(texture, 0xff));
}

// void render_glyph(SDL_Renderer *renderer, Charmap *charmap, uint8_t c, float x, float y)
// {
//     const SDL_Rect dst = {
//         .x = (int) x,
//         .y = (int) y,
//         .w = 16,
//         .h = 16
//     };
//     uint8_t index = c;
//     SCC(SDL_RenderCopy(renderer, charmap->spritesheet, &charmap->glyph_table[index], &dst));
// }

// Charmap *charmap_loadFromFile(const char *filepath, SDL_Renderer *renderer, Uint32 colorKey)
// {
//     Charmap *charmap = malloc(sizeof(Charmap));
//     (void) renderer;
//     SDL_Surface *font_surface = SCP(get_suface_from_file(filepath));
//     SDL_SetColorKey(font_surface, SDL_TRUE, 0xff00ff);
//     charmap->spritesheet = SCP(SDL_CreateTextureFromSurface(renderer, font_surface));
//     set_texture_color(charmap->spritesheet, U32_PURPLE);
//     SDL_FreeSurface(font_surface);

//     for (size_t i = 0; i < CHARMAP_TABLE_SIZE; ++i) {
//         const size_t col = i % FONT_COLS;
//         const size_t row = i / FONT_ROWS;
//         charmap->glyph_table[i] = (SDL_Rect) { .x = col * FONT_CHAR_WIDTH, .y = row * FONT_CHAR_HEIGHT, .w = FONT_CHAR_WIDTH, .h = FONT_CHAR_HEIGHT };
//     }

//     return (charmap);
// }
