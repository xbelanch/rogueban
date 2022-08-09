#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "./ecs.h"
#include "./map.h"

bool mapCells[MAP_HEIGHT][MAP_WIDTH];
// Room rooms[128];
// Path paths[128];

Room potential_rooms[1024];

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
static bool room_overlaps(Room this, Room other, int wall)
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
    for (int y = room->start.y; y < room->end.y; ++y) {
        for (int x = room->start.x; x < room->end.x; ++x) {
            map->cells[y * map->width + x] = FLOOR;
        }
    }

    return true;
}

static void map_room_find_potential_rooms(Room *room, Rooms *rooms)
{
    // Make first a copy of list of active rooms on map
    // Room potential_rooms[rooms->size];
    size_t potential_rooms_size = rooms->size;
    memcpy(potential_rooms, rooms->room, sizeof(rooms->room[0]) * rooms->size);

    // Then remove room itself from list
    // That was my approach... quite similar to stack overflow answer
    // https://stackoverflow.com/a/19967479/553803
    // @TODO: Refactor as a static function (09-08-2022)
    for (size_t i = 0 ; i < potential_rooms_size; ++i) {
        if (room->id == potential_rooms[i].id) {
            memmove(potential_rooms + potential_rooms[i].id, potential_rooms + potential_rooms[i].id + 1, sizeof(potential_rooms[0]) * (potential_rooms_size - 1));
            potential_rooms_size -= 1;
            break;
        }
    }

    // Finally remove existing previous rooms neighbors
    // free(room->neighbors);
    // room->neighbors_size = 0;

    // return potential_rooms;
}

static bool map_room_share_coords_with(Room *src_room, Room *dst_room, char coord, int min)
{
    if (coord == 'x') {
        return (bool)(dst_room->end.x - src_room->start.x + 1 >= min &&
            src_room->end.x - dst_room->start.x + 1 >= min);
    }

    if (coord == 'y'){
        return (bool)(dst_room->end.y - src_room->start.y + 1 >= min &&
            src_room->end.y - dst_room->start.y + 1 >= min);
    }

    return (false);
}

// tests whether each room is horizontally between its two companions.
static bool map_room_between_horizontal(Room *src_room, Room *dst_room, Room *test_room)
{
    return map_room_share_coords_with(test_room, src_room, 'y', 1) &&
        map_room_share_coords_with(test_room, dst_room, 'y', 1) &&
        map_room_share_coords_with(src_room, dst_room, 'y', 1) &&
        ((test_room->center.x > src_room->center.x && test_room->center.x < dst_room->center.x) ||
         (test_room->center.x > dst_room->center.x && test_room->center.x < src_room->center.x));
}

// tests whether each room is horizontally between its two companions.
static bool map_room_between_vertical(Room *src_room, Room *dst_room, Room *test_room)
{
    return map_room_share_coords_with(test_room, src_room, 'x', 1) &&
        map_room_share_coords_with(test_room, dst_room, 'x', 1) &&
        map_room_share_coords_with(src_room, dst_room, 'x', 1) &&
        ((test_room->center.y > src_room->center.y && test_room->center.y < dst_room->center.y) ||
         (test_room->center.y > dst_room->center.y && test_room->center.y < src_room->center.y));
}

static bool map_room_room_between(Room *src_room, Room *dst_room, Room *potential_rooms, size_t potential_rooms_size)
{
    // First remove dst_room from the potential_rooms
    // Make first a copy of list of potentials rooms
    size_t test_rooms_size = potential_rooms_size;
    Room test_rooms[test_rooms_size];
    // test_rooms = malloc(sizeof(potential_rooms[0]) * potential_rooms_size);
    memcpy(test_rooms, potential_rooms, sizeof(potential_rooms[0]) * potential_rooms_size);

    // Then remove src_room itself from test list rooms
    for (size_t i = 0 ; i < test_rooms_size; ++i) {
        if (dst_room->id == test_rooms[i].id) {
            memmove(test_rooms + test_rooms[i].id, test_rooms + test_rooms[i].id + 1, sizeof(test_rooms[0]) * (test_rooms_size - 1));
            test_rooms_size -= 1;
            break;
        }
    }
    // Then check horizontally and vertically if there any rooms
    // in between src and dst rooms.
    for (size_t i = 0; i < test_rooms_size; ++i) {
        if (map_room_between_vertical(src_room, dst_room, &test_rooms[i]) ||
            map_room_between_horizontal(src_room, dst_room, &test_rooms[i])) {

            // Return true if match that condition
            return (true);
        }
    }

    // Otherwise return false
    return (false);
}

static bool map_room_find_facing_rooms(Room *room, Rooms *rooms, size_t min)
{
   bool success = false;
   map_room_find_potential_rooms(room, rooms);
   // Room *potential_rooms = potential_rooms[1024];
   size_t potential_rooms_size = rooms->size - 1;

   for (size_t i = 0; i < potential_rooms_size; ++i) {
       // if (!map_room_room_between(room, &potential_rooms[i], potential_rooms, potential_rooms_size) &&
       //     map_room_share_coords_with(room, &potential_rooms[i], 'x', min) &&
       //     map_room_share_coords_with(room, &potential_rooms[i], 'y', min)) {
       //     printf("Match room[%ld] with room[%ld]\n", room->id, potential_rooms[i].id);
       // }
       /*
  if (!this.roomBetween(room) &&

    (this.sharesCoordsWith(room,'x', min) ||
     this.sharesCoordsWith(room,'y', min))) {

     success = this.connectRoom(room, min);

   } // end i
       */
   }

   // free(potential_rooms);
   // potential_rooms_size = 0;

   return success;
}

void map_test_funcs(Map *map, Rooms *rooms)
{
    map_room_find_facing_rooms(&rooms->room[1], rooms, 1);
}

void map_add_path(Map *map, Path* path)
{
    for (int row = path->from.y; row <= path->to.y; ++row) {
        for (int col = path->from.x; col <= path->to.x; ++col) {
            map->cells[row * map->width + col] = FLOOR;
        }
    }
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
    new->from = start;
    new->to = end;
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
