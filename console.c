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

void console_clear(Console *console)
{
    SDL_Rect src = { .x = 0, .y = 0, .w = console->width, .h = console->height };
    console_fillColor(console->pixels, console->width, src, U32_PURPLE);
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


void console_fillBlend(uint32_t *pixels, uint32_t pixelsPerRow, SDL_Rect dst, uint32_t color)
{
    // return if color is transparent
    if ((color & 0xff) == 0) return;

    // For each pixel in the destination rect, alpha blend the
    // bgColor to the existing color.
    // ref: https://en.wikipedia.org/wiki/Alpha_compositing
    uint32_t endX = dst.x + dst.w;
    uint32_t endY = dst.y + dst.h;

    float srcA = (color & 0xff) / 255.0;
    float invSrcA = 1.0 - srcA;

    // Otherwise, blend each pixel in the dest rect
    for (uint32_t row = dst.y; row < endY; ++row) {
        for (uint32_t col = dst.x; col < endX; ++col) {
            uint32_t *pixel = &pixels[(row * pixelsPerRow) + col];

            if ((color & 0xff) == 255) {
                // Just copy color because alpha blending is not necessary (opaque color)
                // @TODO: Fix this. You can see a line at the top of the screen! (30-07-2022)
                *pixel = 0xff00ffff;
            } else {
                // Do alpha blending
                uint32_t pixelColor = *pixel;

                float destA = (pixelColor && 0xff) / 255.0;

                float outAlpha = srcA + (destA * invSrcA);
                uint8_t fRed = (((color >> 24) * srcA) + ((pixelColor >> 24) * destA * invSrcA)) / outAlpha;
                uint8_t fGreen = (((color >> 16 & 0xff) * srcA) + ((pixelColor >> 16 & 0xff) * destA * invSrcA)) / outAlpha;
                uint8_t fBlue = (((color >> 8 & 0xff) * srcA) + ((pixelColor >> 8 & 0xff) * destA * invSrcA)) / outAlpha;
                uint8_t fAlpha = outAlpha * 255;

                *pixel = (fRed << 24) | (fGreen << 16) | (fBlue << 8) | fAlpha;
// @TODO: Transform ARGB to RGBA  (30-07-2022)
                {
                    uint8_t red = (*pixel >> 24);
                    uint8_t green = (*pixel >> 16 & 0xff);
                    uint8_t blue = (*pixel >> 8 & 0xff);
                    uint8_t alpha = (*pixel & 0xff);
                    *pixel = (red << 24) | (green << 16) | (blue << 8) | (alpha);
                }
            }
        }
    }
}

void console_copyBlend(uint32_t *dstPixels, SDL_Rect dstRect, uint32_t dstPixelsPerRow,
                        uint32_t *srcPixels, SDL_Rect srcRect, uint32_t srcPixelsPerRow,
                        uint32_t newColor)
{
    // For each pixel in the destination rect, alpha blend to it the
    // corresponding pixel in the source rect.
    // ref: https://en.wikipedia.org/wiki/Alpha_compositing
    uint32_t endX = dstRect.x + dstRect.w;
    uint32_t endY = dstRect.y + dstRect.h;


    for (uint32_t srcY = srcRect.y, dstY = dstRect.y; dstY < endY; ++srcY, ++dstY) {
        for (uint32_t srcX = srcRect.x, dstX = dstRect.x; dstX < endX; ++srcX, ++dstX) {
            uint32_t srcColor = srcPixels[(srcY * srcPixelsPerRow) + srcX];
            uint32_t *dstPixel = &dstPixels[(dstY * dstPixelsPerRow) + dstX];
            uint32_t dstColor = *dstPixel;

            // @TODO: Implement colorize pixel color given a (newColor)  (30-07-2022)
            (void) newColor;

            if ((srcColor & 0xff) == 0) {
                // Source pixel color is transparent. Do nothing.
                continue;
            } else if ((srcColor & 0xff) == 255) {
                // Source pixel color is opaque. Copy as it.
                // @TODO: This is at the moment the only condition that works (30-07-2022)
                // Transform ARGB to RGBA
                uint8_t alpha = (srcColor >> 24);
                uint8_t red = ((srcColor >> 16) & 0xff);
                uint8_t green = ((srcColor >> 8) & 0xff);
                uint8_t blue = (srcColor & 0xff);
                *dstPixel = (red << 24) | (green << 16) | (blue << 8) | (alpha);

            } else {
                // Do alpha blending operation
                // Not working at this moment
                float srcA = (srcColor && 0xff) / 255.0;
                float invSrcA = (1.0 - srcA);
                float destA = (dstColor && 0xff) / 255.0;

                float outAlpha = srcA + (destA * invSrcA);
                uint8_t fRed = (((srcColor >> 24) * srcA) + ((dstColor >> 24) * destA * invSrcA)) / outAlpha;
                uint8_t fGreen = (((srcColor >> 16 & 0xff) * srcA) + ((dstColor >> 16 & 0xff) * destA * invSrcA)) / outAlpha;
                uint8_t fBlue = (((srcColor >> 8 & 0xff) * srcA) + ((dstColor >> 8 & 0xff) * destA * invSrcA)) / outAlpha;
                uint8_t fAlpha = outAlpha * 255;

                *dstPixel = (fRed << 24) | (fGreen << 16) | (fBlue << 8) | fAlpha;
            }
        }
    }
}

void console_copyColor(uint32_t *srcPixels, uint32_t *dstPixels, uint32_t srcPixelsPerRow, uint32_t dstPixelsPerRow, SDL_Rect src, SDL_Rect dst, uint32_t alphaKeyColor, uint32_t newColor)
{
    size_t endX = dst.x + dst.w;
    size_t endY = dst.y + dst.h;
    for (size_t srcY = src.y, dstY = dst.y; dstY < endY; ++srcY, ++dstY) {
        for (size_t srcX = src.x, dstX = dst.x; dstX < endX; ++srcX, ++dstX) {
            Uint32 *pixelColor = &dstPixels[(dstY * dstPixelsPerRow) + dstX];

            // Tranform pixel color from ARGB (source) to RGBA space color (?)
            uint32_t srcColor = srcPixels[(srcY * srcPixelsPerRow) + srcX];
            uint8_t alpha = (srcColor >> 24);
            uint8_t red = ((srcColor >> 16) & 0xff);
            uint8_t green = ((srcColor >> 8) & 0xff);
            uint8_t blue = (srcColor & 0xff);
            srcColor = (red << 24) | (green << 16) | (blue << 8) | (alpha);

            if (srcColor == alphaKeyColor) {
                // No need to blend so we pick color below
                continue;
            } else {
                // @TODO: Implement blending color (30-07-2022)
                *pixelColor = newColor;
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

    // @TODO: Fill the background with alpha blending (29-07-2022)
    // This is not working as expected
    // console_fillBlend(con->pixels, con->cellWidth, dst, bgColor);

    uint32_t idx = chr - con->font->firstCharInAtlas;
    uint32_t charsPerRow = (con->font->atlasWidth / con->font->charWidth);
    uint32_t xOffset = (idx % charsPerRow) * con->font->charWidth;
    uint32_t yOffset = (idx / charsPerRow) * con->font->charHeight;
    SDL_Rect src = { .x = xOffset, .y = yOffset, .w = con->font->charWidth, .h = con->font->charHeight};

    // Copy pixel color from font atlas to console pixel buffer
    console_copyColor(con->font->atlas, con->pixels, con->font->atlasWidth, con->width, src, dst, U32_PURPLE, U32_GREEN);
    // console_copyBlend(con->pixels, dst, con->width, con->font->atlas, src, con->font->atlasWidth, fgColor);
}
