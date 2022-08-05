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

void console_set_bitmap_font(Console *console,
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
// Set up the pixel format color masks for RGB(A) byte arrays.
// Only STBI_rgb (3) and STBI_rgb_alpha (4) are supported here!
    Uint32 rmask, gmask, bmask, amask;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    int shift = (req_format == STBI_rgb) ? 8 : 0;
    rmask = 0xff000000 >> shift;
    gmask = 0x00ff0000 >> shift;
    bmask = 0x0000ff00 >> shift;
    amask = 0x000000ff >> shift;
#else // little endian, like x86
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = (req_format == STBI_rgb) ? 0 : 0xff000000;
#endif

    int depth, pitch;
    if (req_format == STBI_rgb) {
        depth = 24;
        pitch = 3 * imgWidth; // 3 bytes per pixel * pixels per row
    } else { // STBI_rgb_alpha (RGBA)
        depth = 32;
        pitch = 4* imgWidth;
    }

    SDL_Surface* surf = SDL_CreateRGBSurfaceFrom((void*)pixelsImg, imgWidth, imgHeight, depth, pitch,
                                                 rmask, gmask, bmask, amask);

    if (surf == NULL) {
        SDL_Log("Creating surface failed: %s", SDL_GetError());
        stbi_image_free(pixelsImg);
        exit(1);
    }

    // Copy image data
    uint32_t atlasSize = imgWidth * imgHeight * sizeof(uint32_t);
    uint32_t *atlasData = malloc(atlasSize);
    memcpy(atlasData, surf->pixels, atlasSize);

    // Create and configure the font
    Font *font = malloc(sizeof(Font));
    font->atlas = atlasData;
    font->atlasWidth = imgWidth;
    font->atlasHeight = imgHeight;
    font->charWidth = charWidth;
    font->charHeight = charHeight;
    font->firstCharInAtlas = firstCharinAtlas;
    font->chromaKey = *atlasData; // guess first color pixel?

    stbi_image_free(pixelsImg);
    if (console->font != NULL) {
        free(console->font->atlas);
        free(console->font);
    }

    console->font = font;
}

void console_fill_color(uint32_t *pixels, uint32_t pixelsPerRow, SDL_Rect dst, uint32_t color)
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
    console_fill_color(console->pixels, console->width, src, CHROMA_KEY);
}

void console_dump_tileset(Console *con)
{
    SDL_Rect src = { .x = 0, .y = 0, .w = con->font->atlasWidth, .h = con->font->atlasHeight };
    uint32_t endX = src.x + src.w;
    uint32_t endY = src.y + src.h;
    for (uint32_t srcY = src.y; srcY < endY; ++srcY) {
        for (uint32_t srcX = src.x; srcX < endX; ++srcX) {
            uint32_t *pixel = &con->pixels[(srcY * con->width) + srcX];
            *pixel = con->font->atlas[(srcY * con->font->atlasWidth) + srcX];
        }
    }

}

static void console_paint_glyph(uint32_t *srcPixel, uint32_t *dstPixel,
                             SDL_Rect src, SDL_Rect dst,
                             size_t srcPixelPerRow, size_t dstPixelPerRow,
                        uint32_t fgColor, uint32_t bgColor, uint32_t chromaKey)
{
    size_t endX = dst.x + dst.w;
    size_t endY = dst.y + dst.h;
    for (size_t srcY = src.y, dstY = dst.y; dstY < endY; ++srcY, ++dstY) {
        for (size_t srcX = src.x, dstX = dst.x; dstX < endX; ++srcX, ++dstX) {

            // @TODO: Explore and rewrite (31-07-2022)
            // srcColor is in ARGB color space. We need to transform it to RGBA
            // Drop it if we set SDL texture to ARGB Space model
            //
            uint32_t srcColor = srcPixel[(srcY * srcPixelPerRow) + srcX];
            // uint8_t alpha = srcColor >> 24;
            // uint8_t red = srcColor >> 16 & 0xff;
            // uint8_t green = srcColor >> 8 & 0xff;
            // uint8_t blue = srcColor & 0xff;
            // srcColor = (red << 24) | (green << 16) | (blue << 8) | (alpha);

            // set background and foreground color
            Uint32 *dstColor = &dstPixel[(dstY * dstPixelPerRow) + dstX];
            if (srcColor == chromaKey) {
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
                    // source code stolen from: https://github.com/PeteyCodes/DarkCaverns/blob/14e13b844f43cd86b6aa4bbaa6c12cf9c55c2239/pt_console.c

                    float srcA = (bgColor & 0xff) / 255.0;
                    float invSrcA = (1.0 - srcA);
                    float destA = (*dstColor & 0xff) / 255.0;

                    float outAlpha = srcA + (destA * invSrcA);
                    uint8_t fRed = (((bgColor >> 24) * srcA) + ((*dstColor >> 24) * destA * invSrcA)) / outAlpha;
                    uint8_t fGreen = (((bgColor >> 16 & 0xff) * srcA) + ((*dstColor >> 16 & 0xff) * destA * invSrcA)) / outAlpha;
                    uint8_t fBlue = (((bgColor >> 8 & 0xff) * srcA) + ((*dstColor >> 8 & 0xff) * destA * invSrcA)) / outAlpha;
                    uint8_t fAlpha = outAlpha * 255;
                    // Set color to rgba space color as SDL2 texture
                    *dstColor =  (fRed << 24) | (fGreen << 16) | (fBlue << 8) | fAlpha;
                }
            } else {
                if ((fgColor & 0xff) == 0xff) {
                    // new foreground color is opaque, don't blend
                    *dstColor = fgColor;
                } else if ((fgColor & 0xff) == 0x0) {
                    // transparent! take original color from the tileset
                    uint8_t alpha = srcColor >> 24;
                    uint8_t red = srcColor >> 16 & 0xff;
                    uint8_t green = srcColor >> 8 & 0xff;
                    uint8_t blue = srcColor & 0xff;
                    srcColor = (red << 24) | (green << 16) | (blue << 8) | (alpha);
                    *dstColor = srcColor;
                } else {
                    // Colorize tileset because we don't want to blend it with background color
                    // Before to do that we need to convert src color from tileset to RGBA Space model
                    uint8_t alpha = srcColor >> 24;
                    uint8_t red = srcColor >> 16 & 0xff;
                    uint8_t green = srcColor >> 8 & 0xff;
                    uint8_t blue = srcColor & 0xff;
                    srcColor = (red << 24) | (green << 16) | (blue << 8) | (alpha);

                    float srcA = (fgColor & 0xff) / 255.0;
                    float invSrcA = (1.0 - srcA);
                    float destA = (srcColor & 0xff) / 255.0;

                    float outAlpha = srcA + (destA * invSrcA);
                    uint8_t fRed = (((fgColor >> 24) * srcA) + ((srcColor >> 24) * destA * invSrcA)) / outAlpha;
                    uint8_t fGreen = (((fgColor >> 16 & 0xff) * srcA) + ((srcColor >> 16 & 0xff) * destA * invSrcA)) / outAlpha;
                    uint8_t fBlue = (((fgColor >> 8 & 0xff) * srcA) + ((srcColor >> 8 & 0xff) * destA * invSrcA)) / outAlpha;
                    uint8_t fAlpha = outAlpha * 255;
                    *dstColor = (fRed << 24) | (fGreen << 16) | (fBlue << 8) | (fAlpha);
                }
            }
        }
    }
}

void console_put_glyph_at(Console *con,
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

    console_paint_glyph(con->font->atlas, con->pixels, src, dst, con->font->atlasWidth, con->width, fgColor, bgColor, con->font->chromaKey);
}
