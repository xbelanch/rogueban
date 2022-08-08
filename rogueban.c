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
#include "./map.h"

GameObject *hero;
GameObject *wall;

static void map_add_wall(size_t x, size_t y)
{
    GameObject *wall = game_object_new();
    Position wallPosition = { .id = wall->id, .x = x, .y = y };
    game_object_add_component(wall, POSITION, &wallPosition);
    Outfit wallOutfit = { .id = wall->id, .glyph = '#', .fgColor = 0x675644ff, .bgColor = 0xff };
    game_object_add_component(wall, OUTFIT, &wallOutfit);
    Physical wallPhysical = { .id = wall->id, .blocksMovement = true, .blocksSight = true };
    game_object_add_component(wall, PHYSICAL, &wallPhysical);
}

static bool can_move(Position *new)
{
    // Check first screen boundaries
    if ((new->y >= 0 && new->y < MAP_HEIGHT) && (new->x >= 0 && new->x < MAP_WIDTH)) {
        // Check physical objects
        for (size_t i = 0; i < MAX_GAME_OBJECTS_SIZE; ++i) {
            Position p = positionGameObjects[i];
            // Check if we encounter some block
            if (p.id != UNUSED && (p.x == new->x && p.y == new->y)) {
                if (physicalGameObjects[i].blocksMovement == true) {
                    return false;
                }
            }
        }
        return true;
    } else {
        return false;
    }

}

static void render_screen(SDL_Renderer *renderer, SDL_Texture *screen, Console *con)
{
    console_clear(con);

    for (size_t i = 0; i < MAX_GAME_OBJECTS_SIZE; ++i) {
        if (outfitGameObjects[i].id != UNUSED) {
            GameObject *obj = &gameObjects[i];
            Position *pos = (Position *)game_object_get_component(obj, POSITION);
            Outfit *vis = (Outfit *)game_object_get_component(obj, OUTFIT);
            console_put_glyph_at(con, vis->glyph, pos->x, pos->y, vis->fgColor, vis->bgColor);
        }
    }

    SDL_UpdateTexture(screen, NULL, con->pixels, SCREEN_WIDTH * (sizeof(uint32_t)));
 	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, screen, NULL, NULL);
    SDL_RenderPresent(renderer);
}

int main(int argc, char *argv[])
{
    srand(time(NULL)); // randomize seed
    (void) argc;
    (void) argv[0];

    Map *map = map_new(NUM_COLS, NUM_ROWS);
    Rooms rooms = { 0 };
    rooms.room = malloc(sizeof(Room) * MAX_ROOMS_SIZE);

	map_reset(map);
    for (size_t i = 0; i < MAX_ROOMS_SIZE; ++i) {
        map_add_room(map, &rooms);
    }

    for (size_t i = 0; i < rooms.size; ++i) {
        printf("Added room with id: %ld\n", rooms.room[i].id);
    }

    // assert(map != NULL);
    // Display map
    for (size_t i = 0; i < map->height; ++i) {
        for (size_t j = 0; j < map->width; ++j)
            if (map->cells[i * map->width + j] == BLOCK) {
                putchar('#');
            } else {
                putchar('.');
            }
        putchar('\n');
    }

    return (0);
}

int main2(int argc, char *argv[])
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
    console_set_bitmap_font(con, TILESET, 0, FONT_SIZE, FONT_SIZE);
    SDL_Texture *screen = SCP(SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, SCREEN_WIDTH, SCREEN_HEIGHT));

    // Initialize world state
    world_init_state();
    // @TODO: drop function. Need to be removed (08-08-2022)
    // map_generate();

    // Carve rooms
    for (size_t row = 0; row < MAP_HEIGHT; ++row) {
        for (size_t col = 0; col < MAP_WIDTH; ++col) {
            if (mapCells[row][col]) {
                map_add_wall(col, row);
            }
        }
    }

    // Add our hero
    hero = game_object_new();

    // Find a free starting position cell for our player
    Position heroPosition = { 0 };
    for (size_t row = 0; row < MAP_HEIGHT; ++row) {
        for (size_t col = 0; col < MAP_WIDTH; ++col) {
            if (!mapCells[row][col]) {
                heroPosition = (Position){ .id = hero->id, .x = col, .y = row };
                break;
            }
        }
    }

    // Add components to our hero
    game_object_add_component(hero, POSITION, &heroPosition);
    Outfit heroOutfit = { .id = hero->id, .glyph = '@', .fgColor = 0xffffffff, .bgColor = 0x0 };
    game_object_add_component(hero, OUTFIT, &heroOutfit);
    Physical heroPhysical = { .id = hero->id, .blocksMovement = true, .blocksSight = true };
    game_object_add_component(hero, PHYSICAL, &heroPhysical);

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
