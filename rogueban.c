#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <SDL2/SDL.h>
#include "colors.h"
#include "console.h"
#include "sdl_helpers.h"
#include "rogueban.h"

GameObject *hero;
GameObject *rock;

void render_screen(SDL_Renderer *renderer, SDL_Texture *screen, Console *con)
{
    console_clear(con);

    Position *rockPos = (Position *)getGameObjectComponent(rock, POSITION);
    Outfit *rockOutfit = (Outfit *)getGameObjectComponent(rock, OUTFIT);
    console_putGlyphAt(con, rockOutfit->glyph, rockPos->x, rockPos->y, rockOutfit->fgColor, rockOutfit->bgColor);

    Position *heroPos = (Position *)getGameObjectComponent(hero, POSITION);
    Outfit *heroOutfit = (Outfit *)getGameObjectComponent(hero, OUTFIT);
    console_putGlyphAt(con, heroOutfit->glyph, heroPos->x, heroPos->y, heroOutfit->fgColor, heroOutfit->bgColor);

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

    // Add our hero to the game
    hero = newGameObject();
    Position heroPosition = { .id = hero->id, .x = (NUM_COLS / 2) - 1, .y = (NUM_ROWS / 2) - 1};
    Outfit heroOutfit = { .id = hero->id, .glyph = '@', .fgColor = 0x00ff00ff, .bgColor = 0x0 };
    addComponentToGameObject(hero, POSITION, &heroPosition);
    addComponentToGameObject(hero, OUTFIT, &heroOutfit);

    // Add a simple rock
    rock = newGameObject();
    Position rockPosition = { .id = rock->id, .x = rand() % NUM_COLS, .y = rand() % NUM_ROWS };
    Outfit rockOutfit = { .id = rock->id, .glyph = 'A', .fgColor = 0xff0000ff, .bgColor = 0xffffffff };
    addComponentToGameObject(rock, POSITION, &rockPosition);
    addComponentToGameObject(rock, OUTFIT, &rockOutfit);

    // Game loop
    bool quit = false;
    while (!quit) {
        const Uint32 start = SDL_GetTicks();
        SDL_Event event = {0};
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
                break;
            } else if (event.type == SDL_KEYDOWN) {

                Position *heroPos = (Position *)getGameObjectComponent(hero, POSITION);

                switch (event.key.keysym.sym) {
                case SDLK_ESCAPE: {
                    quit = true;
                    break;
                }
                case SDLK_w: {
                    if (heroPos->y > 0)
                        heroPos->y--;
                    break;
                }
                case SDLK_s: {
                    if (heroPos->y < NUM_ROWS - 1)
                        heroPos->y++;
                    break;
                }
                case SDLK_a: {
                    if (heroPos->x > 0)
                        heroPos->x--;
                    break;
                }
                case SDLK_d: {
                    if (heroPos->x < NUM_COLS - 1)
                        heroPos->x++;
                    break;
                }
                default: break;
                }
            }
        }

        render_screen(renderer, screen, con);

        const Uint32 duration = SDL_GetTicks() - start;
        const Uint32 delta_time_ms = 1000 / FPS;
        if (duration < delta_time_ms) {
            SDL_Delay(delta_time_ms - duration);
        }

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
