#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <SDL2/SDL.h>
#include "colors.h"
#include "console.h"
#include "sdl_helpers.h"

#define TILESET "./Zilk_16x16.png"
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
    // console_dumpTileset(con);
    console_putGlyphAt(con, '@', 0, 0, 0x0, 0xff);
    console_putGlyphAt(con, '@', 0, 1, 0xffff00ff, 0x0);
    for (size_t i = 0, j = 0; j < 0x10; ++j) {
        console_putGlyphAt(con, '@', j, 2, 0xff000000 | i % 0xff, 0x0);
        console_putGlyphAt(con, '@', j, 3, 0x00ff0000 | i % 0xff, 0x0);
        console_putGlyphAt(con, '@', j, 4, 0x0000ff00 | i % 0xff, 0x0);
        i = 0x10 * j;
    }
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

    Console *con = console_new(SCREEN_WIDTH, SCREEN_HEIGHT, NUM_COLS, NUM_ROWS);
    console_setBitmapFont(con, TILESET, 0, FONT_SIZE, FONT_SIZE);
    SDL_Texture *screen = SCP(SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, SCREEN_WIDTH, SCREEN_HEIGHT));

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
    free(con);
    con = NULL;
    SDL_Quit();
    return (0);
}
