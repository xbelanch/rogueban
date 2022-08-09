#ifndef MAP_H_
#define MAP_H_

#include "stdbool.h"
#include "./rogueban.h"
#include "./ecs.h"

#define MAP_WIDTH SCREEN_WIDTH / FONT_SIZE
#define MAP_HEIGHT SCREEN_HEIGHT / FONT_SIZE
#define MAX_ROOMS_SIZE 16
#define MAX_ROOM_WIDTH 24
#define MAX_ROOM_HEIGHT 24
#define MIN_ROOM_WIDTH 2
#define MIN_ROOM_HEIGHT 2

typedef enum {
    BLOCK = 0,
    FLOOR,
    MAX_SIZE_KIND_CELLS
} CellsType;

typedef enum {
    TALL,
    WIDE
} RoomType;

typedef struct {
    int x, y;
} Point;

typedef struct {
    int width, height;
} Dim;

typedef struct {
    size_t width;
    size_t height;
    CellsType *cells;
} Map;

typedef struct room {
    uint64_t id;
    Point start;
    Point end;
    Dim dim;
    Point center;
    struct room* neighbors;
    size_t neighbors_size;
} Room;

typedef struct {
    Point from;
    Point to;
    bool allowed;
} Path;

// @TODO: Perhaps that'd make sense if belongs to Map struct (07-08-2022)
typedef struct {
    Room *room;
    size_t size;
} Rooms;

extern bool mapCells[MAP_HEIGHT][MAP_WIDTH];

Map *map_new(size_t cols, size_t rows);
void map_reset(Map *map);
void map_generate_rooms(Map *map, Room *rooms);
Room *map_room_new(uint64_t id, Point start, Point end);
bool map_add_room(Map *map, Rooms *rooms);
Path *map_path_new(Point from, Point to, bool allowed);
void map_add_path(Map *map, Path* path);

void map_test_funcs(Map *map, Rooms *rooms);
#endif // MAP_H_
