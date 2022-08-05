#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "./map.h"
#include "./ecs.h"

bool mapCells[MAP_HEIGHT][MAP_WIDTH];

static bool map_carve_room(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
    // Determine if all the cells within the given rectangle are filled
    for (uint8_t row = y; row < y + h; ++row) {
        for (uint8_t col = x; col < x + w; ++col) {
            if (mapCells[row][col] == false)
                return false;
        }
    }

    // Carve the room
    for (uint8_t row = y; row < y + h; ++row) {
        for (uint8_t col = x; col < x + w; ++col) {
            mapCells[row][col] = false;
        }
    }
    return true;
}

void map_generate()
{
    // Mask all the map cells as filled
    for (size_t row = 0; row < MAP_HEIGHT; ++row) {
        for (size_t col = 0; col < MAP_WIDTH; ++col) {
            mapCells[row][col] = true;
        }
    }

    // Carve out non-overlapping rooms that are randomly places
    // of random size
    bool roomsDone = false;
    SDL_Rect rooms[100];
    size_t rooms_count = 0;
    size_t cellsUsed = 0;

    while (!roomsDone) {
        // Generate random width/height rooms
        uint8_t w = rand() % 17 + 3;
        uint8_t h = rand() % 17 + 3;

        uint8_t x = (rand() % MAP_WIDTH) - w - 1;
        uint8_t y = (rand() % MAP_HEIGHT) - h - 1;

        // really?
        if (x >= MAP_WIDTH || y >= MAP_HEIGHT)
            continue;

        bool success = map_carve_room(x, y, w, h);
        if (success) {
            rooms[rooms_count++] = (SDL_Rect){ .x = x, .y = y, .w = w, .h = h };
            cellsUsed += w * h;
        }

        // Exit condition
        if (((float)cellsUsed / (float)(MAP_WIDTH * MAP_HEIGHT)) > 0.65) {
            roomsDone = true;
        }
    }

    // TODO: Join all rooms with corridors
}

void map_add_wall(size_t x, size_t y)
{
    GameObject *wall = game_object_new();
    Position wallPosition = { .id = wall->id, .x = x, .y = y };
    game_object_add_component(wall, POSITION, &wallPosition);
    Outfit wallOutfit = { .id = wall->id, .glyph = '#', .fgColor = 0xff0000ff, .bgColor = 0xffffffff };
    game_object_add_component(wall, OUTFIT, &wallOutfit);
    Physical wallPhysical = { .id = wall->id, .blocksMovement = true, .blocksSight = true };
    game_object_add_component(wall, PHYSICAL, &wallPhysical);
}
