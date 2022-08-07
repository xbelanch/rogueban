#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "./ecs.h"
#include "./map.h"

bool mapCells[MAP_HEIGHT][MAP_WIDTH];
// Room rooms[128];
// Path paths[128];

Room *room_new(uint64_t id, Point start, Point end)
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

Path *path_new(Point start, Point end, bool allowed)
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

bool add_room(Map *map, Rooms *rooms)
{
    Dim dim = gen_dim();
    Point coords = {
        .x = get_center_coord(NUM_COLS, dim.width),
        .y = get_center_coord(NUM_ROWS, dim.height)
    };
    Room room = generate_room(coords, dim.width, dim.height);
    for (size_t i = 0; i < rooms->size; ++i) {
        if (room_overlaps(room, rooms->room[i], 1)) {
            printf("Can't create room\n");
            return (false);
        }
    }

    carve_room(map, &room);

    room.id = rooms->size;
    rooms->room[rooms->size++] = room;
    return (true);
}

void map_generate_rooms(Map *map, Room *rooms)
{
    // Carve out non-overlapping rooms that are randomly places
    // of random size
    bool roomsDone = false;
    size_t idRoom = 0;
    size_t cellsUsed = 0;

    // Get the boundary rectangle of all carving rooms
    // Point boundStart = {map->width, map->height};
    // Point boundEnd = {0, 0};

    for (size_t i = 0; i < MAX_ROOMS_SIZE; ++i) {
        size_t roomWidth = rand() % MAX_ROOM_WIDTH + MIN_ROOM_WIDTH;
        size_t roomHeight = rand() % MAX_ROOM_HEIGHT + MIN_ROOM_HEIGHT;
        Point start = { .x  = (rand() % (map->width - roomWidth)) + 1 , .y = (rand() % (map->height - roomHeight)) + 1 };
        Point end = { .x = (start.x + roomWidth) % map->width , .y = (start.y + roomHeight) % map->height };
        if (end.x == 0) end.x = map->width - 1;
        if (end.y == 0) end.y = map->height -1;
        assert(end.x < map->width && end.y < map->height);

        // update rectangle boundary
        // if (room->start.x < boundStart.x) boundStart.x = room->start.x;
        // if (room->start.y < boundStart.y) boundStart.y = room->start.y;
        // if (room->end.x > boundEnd.x) boundEnd.x = room->end.x;
        // if (room->end.y > boundEnd.y) boundEnd.y = room->end.y;

        Room *room = room_new(idRoom, start, end);
        bool success = carve_room(map, room);

        if (success) {
            rooms[idRoom] = *room;
            cellsUsed += (rooms[idRoom].end.x - rooms[idRoom].start.x) * (rooms[idRoom].end.y - rooms[idRoom].start.y);
            assert(cellsUsed < map->width * map->height);

            // Exit condition
            if (((float)cellsUsed / (float)(map->width * map->height)) > 0.85) {
                break;
            }

            idRoom += 1;
        }
    }
    assert(map != NULL);
}

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
        uint8_t w = rand() % MAX_ROOM_WIDTH + 3;
        uint8_t h = rand() % MAX_ROOM_HEIGHT + 3;
        assert( w < MAP_WIDTH);
        assert( h < MAP_HEIGHT);
        uint8_t x = rand() % (MAP_WIDTH - w - 1);
        uint8_t y = rand() % (MAP_HEIGHT - h - 1);
        if (x == 0) x = 1;
        if (y == 0) y = 1;

        bool success = map_carve_room(x, y, w, h);
        if (success) {
            rooms[rooms_count++] = (SDL_Rect){ .x = x, .y = y, .w = w, .h = h };
            cellsUsed += w * h;
        }

        // Exit condition
        if (((float)cellsUsed / (float)(MAP_WIDTH * MAP_HEIGHT)) > 0.35) {
            roomsDone = true;
        }
    }

    // TODO: Join all rooms with corridors
    // for (size_t i = 1 ; i < rooms_count; ++i) {
    //     SDL_Rect from = rooms[i - 1];
    //     SDL_Rect to = rooms[i];
    // }
}

void map_add_wall(size_t x, size_t y)
{
    GameObject *wall = game_object_new();
    Position wallPosition = { .id = wall->id, .x = x, .y = y };
    game_object_add_component(wall, POSITION, &wallPosition);
    Outfit wallOutfit = { .id = wall->id, .glyph = '#', .fgColor = 0x675644ff, .bgColor = 0xff };
    game_object_add_component(wall, OUTFIT, &wallOutfit);
    Physical wallPhysical = { .id = wall->id, .blocksMovement = true, .blocksSight = true };
    game_object_add_component(wall, PHYSICAL, &wallPhysical);
}
