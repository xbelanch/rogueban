#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <SDL2/SDL.h>
#include "./colors.h"
#include "./console.h"
#include "./sdl_helpers.h"
#include "./rogueban.h"
#include "./ecs.h"

GameObject *hero;
GameObject *wall;

bool can_move(Position *pos)
{
    bool allowMove = true;

    // Check first screen boundaries
    if ((pos->y >= 0 && pos->y < NUM_ROWS) && (pos->x >= 0 && pos->x < NUM_COLS)) {
        // Check physical objects
        for (size_t i = 0; i < MAX_GAME_OBJECTS_SIZE; ++i) {
            Position p = positionGameObjects[i];
            // Check if we encounter some block
            if (p.id != UNUSED && (p.x == pos->x && p.y == pos->y)) {
                if (physicalGameObjects[i].blocksMovement == true) {
                    allowMove = false;
                    break;
                }
            }
        }
    }
    return allowMove;
}

void render_screen(SDL_Renderer *renderer, SDL_Texture *screen, Console *con)
{
    console_clear(con);

    for (size_t i = 0; i < MAX_GAME_OBJECTS_SIZE; ++i) {
        if (outfitGameObjects[i].id != UNUSED) {
            GameObject *obj = &gameObjects[i];
            Position *pos = (Position *)game_object_get_component(obj, POSITION);
            Outfit *vis = (Outfit *)game_object_get_component(obj, OUTFIT);
            console_putGlyphAt(con, vis->glyph, pos->x, pos->y, vis->fgColor, vis->bgColor);
        }
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

    srand(time(NULL)); // randomize seed

    // SDL2 init stuff
    SCC(SDL_Init(SDL_INIT_VIDEO));
    SDL_Window *window = SCP(SDL_CreateWindow("Rogueban", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0));
    SDL_Renderer *renderer = SCP(SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED));
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
    SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);

    Console *con = console_new(SCREEN_WIDTH, SCREEN_HEIGHT, NUM_COLS, NUM_ROWS);
    console_setBitmapFont(con, TILESET, 0, FONT_SIZE, FONT_SIZE);
    SDL_Texture *screen = SCP(SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, SCREEN_WIDTH, SCREEN_HEIGHT));

    // Initialize world state
    world_init_state();

    // Add entities to the game
    hero = game_object_new();
    Position heroPosition = { .id = hero->id, .x = (NUM_COLS / 2) - 1, .y = (NUM_ROWS / 2) - 1};
    game_object_add_component(hero, POSITION, &heroPosition);
    Outfit heroOutfit = { .id = hero->id, .glyph = '@', .fgColor = 0x00ff00ff, .bgColor = 0x0 };
    game_object_add_component(hero, OUTFIT, &heroOutfit);
    Physical heroPhysical = { .id = hero->id, .blocksMovement = true, .blocksSight = true };
    game_object_add_component(hero, PHYSICAL, &heroPhysical);

    wall = game_object_new();
    Position wallPosition = { .id = wall->id, .x = rand() % NUM_COLS, .y = rand() % NUM_ROWS };
    game_object_add_component(wall, POSITION, &wallPosition);
    Outfit wallOutfit = { .id = wall->id, .glyph = '#', .fgColor = 0xff0000ff, .bgColor = 0xffffffff };
    game_object_add_component(wall, OUTFIT, &wallOutfit);
    Physical wallPhysical = { .id = wall->id, .blocksMovement = true, .blocksSight = true };
    game_object_add_component(wall, PHYSICAL, &wallPhysical);


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

                Position *heroPos = (Position *)game_object_get_component(hero, POSITION);

                switch (event.key.keysym.sym) {
                case SDLK_ESCAPE: {
                    quit = true;
                    break;
                }
                case SDLK_w: {
                    Position newPosition = { .id = heroPos->id, .x = heroPos->x, .y = heroPos->y - 1 };
                    if (can_move(&newPosition))
                        game_object_add_component(hero, POSITION, &newPosition);
                    break;
                }
                case SDLK_s: {
                    Position newPosition = { .id = heroPos->id, .x = heroPos->x, .y = heroPos->y + 1};
                    if (can_move(&newPosition))
                        game_object_add_component(hero, POSITION, &newPosition);
                    break;
                }
                case SDLK_a: {
                    Position newPosition = { .id = heroPos->id, .x = heroPos->x - 1, .y = heroPos->y };
                    if (can_move(&newPosition))
                        game_object_add_component(hero, POSITION, &newPosition);
                    break;
                }
                case SDLK_d: {
                    Position newPosition = { .id = heroPos->id, .x = heroPos->x + 1, .y = heroPos->y };
                    if (can_move(&newPosition))
                        game_object_add_component(hero, POSITION, &newPosition);
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
