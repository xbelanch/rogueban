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
    uint32_t id;
    void *components[MAX_SIZE_COMPONENTS];
} GameObject;

// Components
typedef struct {
    uint32_t id;
    int x, y;
} Position;

typedef struct {
    uint32_t id;
    asciiChar glyph;
    uint32_t fgColor;
    uint32_t bgColor;
} Outfit;

typedef struct {
    uint32_t id;
    bool blocksMovement;
    bool blocksSight;
} Physical;

GameObject *newGameObject();
void addComponentToGameObject(GameObject *obj, GameComponent comp, void *compData);
void *getGameObjectComponent(GameObject *obj, GameComponent comp);
void destroyGameObject(GameObject *obj);

// World State
extern GameObject gameObjects[MAX_GAME_OBJECTS_SIZE];
extern Position positionGameObjects[MAX_GAME_OBJECTS_SIZE];
extern Outfit outfitGameObjects[MAX_GAME_OBJECTS_SIZE];
extern Physical physicalGameObjects[MAX_GAME_OBJECTS_SIZE];

#endif // ECS_H_
