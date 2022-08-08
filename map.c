#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "./ecs.h"
#include "./map.h"

bool mapCells[MAP_HEIGHT][MAP_WIDTH];
// Room rooms[128];
// Path paths[128];

static Dim gen_dim() {
    const size_t BASE_DIM = 8;
    const size_t EXTRA = 5;
    RoomType type = (rand() % 10) < 5 ? TALL : WIDE;

    Dim dim = { .width = BASE_DIM, .height = BASE_DIM };

    const float range[] = { 0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.7, 0.8, 0.9, 1.0 };
    size_t range_len = sizeof range / sizeof(range[0]);
    size_t additional = round(range[rand() % range_len] * EXTRA);

    if (type == TALL) {
        dim.height += additional;
    } else {
        dim.width += additional;
    }

    return dim;
}

static size_t get_center_coord(size_t numCells, size_t dim)
{
    const size_t OUTER_LIMIT = 3;
    // get limit on either side based on outer limit and a room dimension - width or height
    size_t minDist = OUTER_LIMIT + round(dim / 2);
    // get range based on cells in array - limit on either side.
    size_t range = numCells - 2 * minDist;
    // get a random  number within
    return minDist + round(rand() % range);
}

static void set_room_coordinates(Room *room, Point center, size_t width, size_t height)
{
   size_t halfW = round(width / 2);
   size_t halfH = round(height / 2);

   Point start = {
       .x = center.x - halfW,
       .y = center.y - halfH
   };

   Point end = {
      .x = center.x + halfW,
      .y = center.y + halfH
   };

   room->start = start;
   room->end = end;
}

static Room generate_room(Point center, size_t width, size_t height)
{
    Room room = { 0 };
    set_room_coordinates(&room, center, width, height);
    room.dim.width = width;
    room.dim.height = height;
    room.center = center;
    return room;
}

// Check horizontal and vertical room overlap
static bool room_overlaps(Room this, Room other, size_t wall)
{
    if (other.end.x + wall >= this.start.x &&
        this.end.x + wall >= other.start.x &&
        other.end.y + wall >= this.start.y &&
        this.end.y + wall >= other.start.y)
        return true;

    return false;
}


static bool carve_room(Map *map, Room *room)
{
    for (size_t y = room->start.y; y < room->end.y; ++y) {
        for (size_t x = room->start.x; x < room->end.x; ++x) {
            map->cells[y * map->width + x] = CARVED;
        }
    }

    return true;
}

bool map_add_room(Map *map, Rooms *rooms)
{
    Dim dim = gen_dim();
    Point coords = {
        .x = get_center_coord(NUM_COLS, dim.width),
        .y = get_center_coord(NUM_ROWS, dim.height)
    };
    Room room = generate_room(coords, dim.width, dim.height);
    for (size_t i = 0; i < rooms->size; ++i) {
        if (room_overlaps(room, rooms->room[i], 1))
            return (false);
    }

    carve_room(map, &room);

    room.id = rooms->size;
    rooms->room[rooms->size++] = room;
    return (true);
}

Room *map_room_new(uint64_t id, Point start, Point end)
{
    Room *new = malloc(sizeof(Room));
    new->id = id;
    new->start = start;
    new->end = end;
    new->center.x = (end.x - start.x) / 2;
    new->center.y = (end.y - start.y) / 2;
    new->neighbors = NULL;
    return new;
}

Path *map_path_new(Point start, Point end, bool allowed)
{
    Path *new = malloc(sizeof(Path));
    new->start = start;
    new->end = end;
    new->allowed = allowed;
    return new;
}

Map *map_new(size_t width, size_t height)
{
    Map *new = malloc(sizeof(Map));
    new->width = width;
    new->height = height;
    new->cells = malloc(sizeof(CellsType) * width * height);
    memset(new->cells, BLOCK, sizeof(CellsType) * width * height);
    return new;
}

void map_reset(Map *map)
{
    memset(map->cells, BLOCK, sizeof(CellsType) * map->width * map->height);
}
