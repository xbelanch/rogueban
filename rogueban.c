#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <SDL2/SDL.h>
#include "common.h"
#include "console.h"
#include "sdl_helpers.h"

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define NUM_ROWS 30 // SCREEN_HEIGHT / FONT_HEIGHT
#define NUM_COLS 40 // SCREEN_WIDTH / FONT_WIDTH

typedef enum {
    BLANK = 0,
    SAND = 66
} TileType;

typedef struct {
    TileType tileType[NUM_COLS * NUM_ROWS];
} Map;

Map *newMap()
{
    Map *map = malloc(sizeof(Map));
    for (size_t i = 0; i < NUM_COLS; ++i)
        map->tileType[i] = BLANK;
    for (size_t i = NUM_COLS; i < (NUM_COLS * NUM_ROWS); ++i) {
        map->tileType[i] = SAND;
    }
    return map;
}

// @TODO: This is awful. Need to be rewrite from scratch (26-07-2022)
void render_screen(SDL_Renderer *renderer, SDL_Texture *texture)
{
    SCC(SDL_SetRenderDrawColor(renderer, BLACK));
    SCC(SDL_RenderClear(renderer));
    SCC(SDL_RenderCopy(renderer, texture, NULL, NULL));
    // // draw whole screen of dots
    // for (int rows = 0; rows <= NUM_ROWS ; ++rows) {
    //     for (int cols = 0; cols <= NUM_COLS; ++cols) {
    //         // @TODO: Replace this render glyph function (26-07-2022)
    //         render_glyph(renderer, charmap, map->tileType[rows * NUM_COLS + cols], cols * 16, rows * 16);
    //     }
    // }
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

    // Testing simple map
    // Map *map = newMap();

    Console *con = console_new(1024, 768, 16, 16);
    console_setBitmapFont(con, "./Anikki_square_16x16.png", 0, 16, 16);
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
    SDL_Texture *texture = SCP(SDL_CreateTextureFromSurface(renderer, surf));
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

        render_screen(renderer, texture);
        SDL_Delay(250);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
	window = NULL;
	renderer = NULL;

    SDL_Quit();
    return (0);
}
