#ifndef _ROGUEBAN_H
#define _ROGUEBAN_H

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include "console.h"

#define TILESET "./Zilk_16x16.png"
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define FONT_SIZE 16
#define ZOOM 1
#define NUM_ROWS SCREEN_HEIGHT / FONT_SIZE
#define NUM_COLS SCREEN_WIDTH / FONT_SIZE
#define FPS 60
#define MAX_GAME_OBJECTS_SIZE 1024

typedef enum {
    POSITION = 0,
    OUTFIT,
    PHYSICAL,
    HEALTH,
    MOVEMENT,

    // Define other components above
    MAX_SIZE_COMPONENTS
} GameComponent;

// Entities and Components
// Based on https://en.wikipedia.org/wiki/Entity_component_system
// Simplistic implementation in C: https://gamedev.stackexchange.com/questions/172584/how-could-i-implement-an-ecs-in-c

typedef struct {
    uint8_t x, y;
} Hero;

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

// Initialize all game objects and components
GameObject gameObjects[MAX_GAME_OBJECTS_SIZE];
Position positionGameObjects[MAX_GAME_OBJECTS_SIZE];
Outfit outfitGameObjects[MAX_GAME_OBJECTS_SIZE];
Physical physicalGameObjects[MAX_GAME_OBJECTS_SIZE];

GameObject *newGameObject()
{
    GameObject *obj = NULL;
    // First (amb naive) way for storing gameObjects
    // @TODO: Try to rewrite this because trick of i = 1 when id is equal to 0 (02-08-2022)
    for (size_t i = 1; i < MAX_GAME_OBJECTS_SIZE; ++i) {
        if (gameObjects[i].id == 0) {
            obj = &gameObjects[i];
            obj->id = i;
            break;
        }
    }
    assert(obj != NULL);
    return obj;
}

void addComponentToGameObject(GameObject *obj, GameComponent comp, void *compData)
{
    assert(obj->id != 0);
    switch(comp) {
    case POSITION: {
        Position *position = &positionGameObjects[obj->id];
        position->id = obj->id;
        Position *data = (Position *) compData;
        position->x = data->x;
        position->y = data->y;
        obj->components[comp] = &positionGameObjects[obj->id];
        break;
    }
    case OUTFIT: {
        Outfit *outfit = &outfitGameObjects[obj->id];
        outfit->id = obj->id;
        Outfit *data = (Outfit *) compData;
        outfit->glyph = data->glyph;
        outfit->fgColor = data->fgColor;
        outfit->bgColor = data->bgColor;
        obj->components[comp] = &outfitGameObjects[obj->id];
        break;
    }
    case PHYSICAL: {
        Physical *physics = &physicalGameObjects[obj->id];
        physics->id = obj->id;
        Physical *data = (Physical *)compData;
        physics->blocksMovement = data->blocksMovement;
        physics->blocksSight = data->blocksSight;
        obj->components[comp] = &physicalGameObjects[obj->id];
        break;
    }
    default:
        assert(false && "Unknow component");
        exit(1);
    }
}

void *getGameObjectComponent(GameObject *obj, GameComponent comp)
{
    return obj->components[comp];
}

void destroyGameObject(GameObject *obj)
{
    positionGameObjects[obj->id].id = 0;
    outfitGameObjects[obj->id].id = 0;
    physicalGameObjects[obj->id].id = 0;
    // Set index object to zero
    obj->id = 0;
    // Clean up all components used by this object
}


#endif // _ROGUEBAN_H
