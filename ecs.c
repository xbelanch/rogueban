#include "./ecs.h"

GameObject gameObjects[MAX_GAME_OBJECTS_SIZE];
Position positionGameObjects[MAX_GAME_OBJECTS_SIZE];
Outfit outfitGameObjects[MAX_GAME_OBJECTS_SIZE];
Physical physicalGameObjects[MAX_GAME_OBJECTS_SIZE];

void world_init_state()
{
    for (size_t i = 0; i < MAX_GAME_OBJECTS_SIZE; ++i) {
        gameObjects[i].id = UNUSED;
        positionGameObjects[i].id = UNUSED;
        outfitGameObjects[i].id = UNUSED;
        physicalGameObjects[i].id = UNUSED;
    }
}

GameObject *game_object_new()
{
    GameObject *obj = NULL;
    // First (amb naive) way for storing gameObjects
    // @TODO: Try to rewrite this because trick of i = 1 when id is equal to 0 (02-08-2022)
    for (size_t i = 0; i < MAX_GAME_OBJECTS_SIZE; ++i) {
        if (gameObjects[i].id == UNUSED) {
            obj = &gameObjects[i];
            obj->id = i;
            break;
        }
    }
    assert(obj != NULL);

    for (size_t i = 0; i < MAX_SIZE_COMPONENTS; ++i) {
        obj->components[i] = NULL;
    }

    return obj;
}

void game_object_add_component(GameObject *obj, GameComponent comp, void *compData)
{
    assert(obj->id != UNUSED);
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
        assert(true && "Unknow component");
        exit(1);
    }
}

void *game_object_get_component(GameObject *obj, GameComponent comp)
{
    return obj->components[comp];
}

void game_object_destroy(GameObject *obj)
{
    positionGameObjects[obj->id].id = UNUSED;
    outfitGameObjects[obj->id].id = UNUSED;
    physicalGameObjects[obj->id].id = UNUSED;
    // Set index object to unused
    obj->id = UNUSED;
    // Clean up all components used by this object
}
