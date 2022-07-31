#include <stdlib.h>
#include <stdio.h>
#include "colors.h"
#include "sdl_helpers.h"
#include "console.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STBI_ONLY_PNG

Console *console_new(uint32_t width,
                     uint32_t height,
                     uint32_t colCount,
                     uint32_t rowCount)
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

void console_clear(Console *console)
{
    SDL_Rect src = { .x = 0, .y = 0, .w = console->width, .h = console->height };
    console_fillColor(console->pixels, console->width, src, CHROMA_KEY);
}

void console_paintGlyphColor(uint32_t *srcPixel, uint32_t *dstPixel,
                             SDL_Rect src, SDL_Rect dst,
                             size_t srcPixelPerRow, size_t dstPixelPerRow,
                             uint32_t fgColor, uint32_t bgColor)
{
    size_t endX = dst.x + dst.w;
    size_t endY = dst.y + dst.h;
    for (size_t srcY = src.y, dstY = dst.y; dstY < endY; ++srcY, ++dstY) {
        for (size_t srcX = src.x, dstX = dst.x; dstX < endX; ++srcX, ++dstX) {

            // @TODO: Explore and rewrite (31-07-2022)
            // srcColor is in ARGB color space. We need to transform it to RGBA
            uint32_t srcColor = srcPixel[(srcY * srcPixelPerRow) + srcX];
            uint8_t alpha = srcColor >> 24;
            uint8_t red = srcColor >> 16 & 0xff;
            uint8_t green = srcColor >> 8 & 0xff;
            uint8_t blue = srcColor & 0xff;
            srcColor = (red << 24) | (green << 16) | (blue << 8) | (alpha);

            // set background and foreground color
            Uint32 *dstColor = &dstPixel[(dstY * dstPixelPerRow) + dstX];
            if (srcColor == CHROMA_KEY) {
                if ((bgColor & 0xff) == 0xff) {
                    // new background color is opaque, don't blend
                    *dstColor = bgColor;
                } else if ((bgColor & 0xff) == 0x0) {
                    // transparent! dont blend!
                    continue;
                } else {
                    // Do alpha blending operation
                    // For each pixel in the destination rect, alpha blend to it the
                    // corresponding pixel in the source rect.
                    // ref: https://en.wikipedia.org/wiki/Alpha_compositing

                    float srcA = (bgColor & 0xff) / 255.0;
                    float invSrcA = (1.0 - srcA);
                    float destA = (*dstColor & 0xff) / 255.0;

                    float outAlpha = srcA + (destA * invSrcA);
                    uint8_t fRed = (((bgColor >> 24) * srcA) + ((*dstColor >> 24) * destA * invSrcA)) / outAlpha;
                    uint8_t fGreen = (((bgColor >> 16 & 0xff) * srcA) + ((*dstColor >> 16 & 0xff) * destA * invSrcA)) / outAlpha;
                    uint8_t fBlue = (((bgColor >> 8 & 0xff) * srcA) + ((*dstColor >> 8 & 0xff) * destA * invSrcA)) / outAlpha;
                    uint8_t fAlpha = outAlpha * 255;
                    *dstColor = (fRed << 24) | (fGreen << 16) | (fBlue << 8) | (fAlpha);
                }
            } else {
                if ((fgColor & 0xff) == 0xff) {
                    // new foreground color is opaque, don't blend
                    *dstColor = fgColor;
                } else if ((fgColor & 0xff) == 0x0) {
                    // transparent! dont blend!
                    continue;
                } else {
                    // Same as bgColor
                    float srcA = (fgColor & 0xff) / 255.0;
                    float invSrcA = (1.0 - srcA);
                    float destA = (*dstColor & 0xff) / 255.0;

                    float outAlpha = srcA + (destA * invSrcA);
                    uint8_t fRed = (((fgColor >> 24) * srcA) + ((*dstColor >> 24) * destA * invSrcA)) / outAlpha;
                    uint8_t fGreen = (((fgColor >> 16 & 0xff) * srcA) + ((*dstColor >> 16 & 0xff) * destA * invSrcA)) / outAlpha;
                    uint8_t fBlue = (((fgColor >> 8 & 0xff) * srcA) + ((*dstColor >> 8 & 0xff) * destA * invSrcA)) / outAlpha;
                    uint8_t fAlpha = outAlpha * 255;
                    *dstColor = (fRed << 24) | (fGreen << 16) | (fBlue << 8) | (fAlpha);
                }
            }
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

    uint32_t x = cellX * con->cellWidth;
    uint32_t y = cellY * con->cellHeight;
    SDL_Rect dst = { .x = x, .y = y, .w = con->cellWidth, .h = con->cellHeight };

    uint32_t idx = chr - con->font->firstCharInAtlas;
    uint32_t charsPerRow = (con->font->atlasWidth / con->font->charWidth);
    uint32_t xOffset = (idx % charsPerRow) * con->font->charWidth;
    uint32_t yOffset = (idx / charsPerRow) * con->font->charHeight;
    SDL_Rect src = { .x = xOffset, .y = yOffset, .w = con->font->charWidth, .h = con->font->charHeight};

    console_paintGlyphColor(con->font->atlas, con->pixels,
                                 src, dst,
                                 con->font->atlasWidth, con->width,
                                 fgColor, bgColor);
}
