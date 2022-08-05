#ifndef ECS_H_
#define ECS_H_
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include "./rogueban.h"
#include "./console.h"

// Entity component system approach
// Based on https://en.wikipedia.org/wiki/Entity_component_system
// Simplistic implementation in C: https://gamedev.stackexchange.com/questions/172584/how-could-i-implement-an-ecs-in-c

#define UNUSED -1

typedef enum {
    POSITION = 0,
    OUTFIT,
    PHYSICAL,
    HEALTH,
    MOVEMENT,

    // Define other components above
    MAX_SIZE_COMPONENTS
} GameComponent;

typedef struct {
    int64_t id;
    void *components[MAX_SIZE_COMPONENTS];
} GameObject;

// Components
typedef struct {
    int64_t id;
    int x, y;
} Position;

typedef struct {
    int64_t id;
    asciiChar glyph;
    uint32_t fgColor;
    uint32_t bgColor;
} Outfit;

typedef struct {
    int64_t id;
    bool blocksMovement;
    bool blocksSight;
} Physical;

// World State
extern GameObject gameObjects[MAX_GAME_OBJECTS_SIZE];
extern Position positionGameObjects[MAX_GAME_OBJECTS_SIZE];
extern Outfit outfitGameObjects[MAX_GAME_OBJECTS_SIZE];
extern Physical physicalGameObjects[MAX_GAME_OBJECTS_SIZE];

void world_init_state();
GameObject *game_object_new();
void game_object_add_component(GameObject *obj, GameComponent comp, void *compData);
void *game_object_get_component(GameObject *obj, GameComponent comp);
void game_object_destroy(GameObject *obj);


#endif // ECS_H_
