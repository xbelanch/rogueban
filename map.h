#ifndef MAP_H_
#define MAP_H_

#include "stdbool.h"
#include "./rogueban.h"
#include "./ecs.h"

#define MAP_WIDTH SCREEN_WIDTH / FONT_SIZE
#define MAP_HEIGHT SCREEN_HEIGHT / FONT_SIZE
#define MAX_ROOM_WIDTH 8
#define MAX_ROOM_HEIGHT 8

extern bool mapCells[MAP_HEIGHT][MAP_WIDTH];

void map_generate();
void map_add_wall(size_t x, size_t y);

#endif // MAP_H_
