#include <SDL2/SDL.h>
#include "common.h"
// Include the amazing STB image library - Only PNG support
// https://raw.githubusercontent.com/nothings/stb/master/stb_image.h
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STBI_ONLY_PNG

int SCC(int ret)
{
    if (ret < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s", SDL_GetError());
        return (3);
    }
    return (0);
}

void *SCP(void *ptr)
{
    if (ptr == NULL) {
        fprintf(stderr, "%s\n", SDL_GetError());
        exit(1);
    }
    return ptr;
}

SDL_Surface *get_suface_from_file(const char *file_path)
{
    // Source code robbed from https://wiki.libsdl.org/SDL_CreateRGBSurfaceFrom
    int req_format = STBI_rgb_alpha;
    int width, height, orig_format;
    unsigned char* pixels = stbi_load(file_path, &width, &height, &orig_format, req_format);
    if(pixels == NULL) {
        SDL_Log("Loading image failed: %s", stbi_failure_reason());
        exit(1);
    }

    // Set up the pixel format color masks for RGB(A) byte arrays.
    // Only STBI_rgb (3) and STBI_rgb_alpha (4) are supported here!
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    int shift = (req_format == STBI_rgb) ? 8 : 0;
    const u32 rmask = 0xff000000 >> shift;
    const u32 gmask = 0x00ff0000 >> shift;
    const u32 bmask = 0x0000ff00 >> shift;
    const u32 amask = 0x000000ff >> shift;
#else // little endian, like x86
    const u32 rmask = 0x000000ff;
    const u32 gmask = 0x0000ff00;
    const u32 bmask = 0x00ff0000;
    const u32 amask = (req_format == STBI_rgb) ? 0 : 0xff000000;
#endif

    // STBI_rgb_alpha (RGBA)
    int depth = 32;
    int pitch = 4*width;

    return SCP(SDL_CreateRGBSurfaceFrom((void*)pixels, width, height, depth, pitch, rmask, gmask, bmask, amask));
}
