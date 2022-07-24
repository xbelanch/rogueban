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
#define NUM_ROWS 80
#define NUM_COLS 60

void render_screen(SDL_Renderer *renderer, Charmap *charmap)
{
    SCC(SDL_SetRenderDrawColor(renderer, BLACK));
    SCC(SDL_RenderClear(renderer));
    render_glyph(renderer, charmap, 11 * 16, 0, 0);
    SDL_RenderPresent(renderer);
}

// @TODO: Fix chroma key glyph color (24-07-2022)

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

    // Testing charmap
    Charmap *charmap = charmap_loadFromFile("./Anikki_square_16x16.png", renderer, U32_PURPLE);

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

        render_screen(renderer, charmap);
        SDL_Delay(250);
    }

    // SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
	window = NULL;
	renderer = NULL;
    charmap = NULL;
    SDL_Quit();
    return (0);
}
