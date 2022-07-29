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

void console_clear(Console *console)
{
    SDL_Rect src = { .x = 0, .y = 0, .w = console->width, .h = console->height };
    console_fillColor(console->pixels, console->width, src, 0x0);
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

void console_fillColor(uint32_t *pixels, uint32_t pixelsPerRow, SDL_Rect dst, uint32_t color)
{
    uint32_t endX = dst.x + dst.w;
    uint32_t endY = dst.y + dst.h;

    for (uint32_t row = dst.y; row < endY; ++row) {
        for (uint32_t col = dst.x; col < endX; ++col) {
            pixels[row * pixelsPerRow + col] = color;
        }
    }
}

// @TODO: Improve this raw copy paste (29-07-2022)
void console_CopyPaste(uint32_t *srcPixels, SDL_Rect srcRect, uint32_t srcPixelsPerRow,
                       uint32_t *dstPixels, SDL_Rect dstRect, uint32_t dstPixelsPerRow)
{
    uint32_t endX = dstRect.x + dstRect.w;
    uint32_t endY = dstRect.y + dstRect.h;

    for (uint32_t srcY = srcRect.y, dstY = dstRect.y; dstY < endY; ++srcY, ++dstY) {
        for (uint32_t srcX = srcRect.x, dstX = dstRect.x; dstX < endX; ++srcX, ++dstX) {
            uint32_t srcColor = srcPixels[srcY * srcPixelsPerRow + srcX];
            dstPixels[dstY * dstPixelsPerRow + dstX] = srcColor;
        }
    }
}

void console_putGlyphAt(Console *con,
                        asciiChar chr,
                        uint32_t cellX,
                        uint32_t cellY,
                        uint32_t fgColor,
                        uint32_t bgColor)
{
// @TODO: Implement fgColor and bgColor (29-07-2022)
    (void) fgColor;
    (void) bgColor;

    uint32_t x = cellX * con->cellWidth;
    uint32_t y = cellY * con->cellHeight;
    SDL_Rect dst = { .x = x, .y = y, .w = con->cellWidth, .h = con->cellHeight };

    // @TODO: Fill the background with alpha blending (29-07-2022)

    uint32_t idx = chr - con->font->firstCharInAtlas;
    uint32_t charsPerRow = (con->font->atlasWidth / con->font->charWidth);
    uint32_t xOffset = (idx % charsPerRow) * con->font->charWidth;
    uint32_t yOffset = (idx / charsPerRow) * con->font->charHeight;
    SDL_Rect src = { .x = xOffset, .y = yOffset, .w = con->font->charWidth, .h = con->font->charHeight};

    console_CopyPaste(con->font->atlas, src, con->font->atlasWidth, con->pixels, dst, con->width);
}
