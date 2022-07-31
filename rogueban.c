#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <SDL2/SDL.h>
#include "colors.h"
#include "console.h"
#include "sdl_helpers.h"

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define FONT_SIZE 16
#define ZOOM 1
#define NUM_ROWS SCREEN_HEIGHT / FONT_SIZE
#define NUM_COLS SCREEN_WIDTH / FONT_SIZE

void render_screen(SDL_Renderer *renderer, SDL_Texture *screen, Console *con)
{
    console_clear(con);
    // size_t charSize = con->font->charWidth * con->font->charHeight;
    // for (size_t i = 0; i < charSize; ++i) {
    //     console_putGlyphAt(con, i, i % con->font->charWidth * 2, (i / con->font->charWidth), 0xff0000ff, 0xffffffff);
    // }
    console_putGlyphAt(con, 'A', 1, 2, 0xffffff00, 0x000000ff);
    console_putGlyphAt(con, 'A', 2, 2, 0xffffff11, 0x000000aa);
    console_putGlyphAt(con, 'A', 3, 2, 0xffffff55, 0x00000088);
    console_putGlyphAt(con, 'A', 4, 2, 0xffffff88, 0x00000055);
    console_putGlyphAt(con, 'A', 5, 2, 0xffffffaa, 0x00000011);
    console_putGlyphAt(con, 'A', 5, 2, 0xffffffff, 0x00000000);

    SDL_UpdateTexture(screen, NULL, con->pixels, SCREEN_WIDTH * (sizeof(uint32_t)));
 	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, screen, NULL, NULL);
    SDL_RenderPresent(renderer);
}

int main(int argc, char *argv[])
{
    (void) argc;
    (void) argv[0];

    // SDL2 init stuff
    SCC(SDL_Init(SDL_INIT_VIDEO));
    SDL_Window *window = SCP(SDL_CreateWindow("Rogueban", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0));
    SDL_Renderer *renderer = SCP(SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED));
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
    SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);

    // @TODO: Glyphs distorted when screen width and height are not equal (29-07-2022)
    Console *con = console_new(SCREEN_WIDTH, SCREEN_HEIGHT, NUM_COLS, NUM_ROWS);
    console_setBitmapFont(con, "./Anikki_square_16x16.png", 0, FONT_SIZE, FONT_SIZE);
    // Testing atlas data image
    SDL_Surface* surf = SDL_CreateRGBSurfaceFrom(con->font->atlas,
                                                 con->font->atlasWidth,
                                                 con->font->atlasHeight,
                                                 32, //rgb_alpha
                                                 4 * con->font->atlasWidth,
                                                 0x000000ff,
                                                 0x0000ff00,
                                                 0x00ff0000,
                                                 0xff000000
        );

    SDL_Texture *screen = SCP(SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, SCREEN_WIDTH, SCREEN_HEIGHT));
    SDL_FreeSurface(surf);
    surf = NULL;

    // Game loop
    bool quit = false;
    while (!quit) {
        SDL_Event event = {0};
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
                break;
            } else if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                case SDLK_ESCAPE: {
                    quit = true;
                    break;
                }
                default: break;
                }
            }
        }

        render_screen(renderer, screen, con);
        SDL_Delay(250);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
	window = NULL;
	renderer = NULL;

    SDL_Quit();
    return (0);
}
