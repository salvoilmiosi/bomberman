#include "tile_belt.h"

#include "ent_movable.h"

tile_belt::tile_belt(tile *t_tile, game_map *g_map, uint8_t direction) : tile_entity(SPECIAL_BELT, t_tile, g_map), direction(direction) {
    tx = g_map->getTileX(t_tile);
    ty = g_map->getTileY(t_tile);

    uint16_t data = (direction & 0x3) << 10;

    switch(t_tile->type) {
    case TILE_BREAKABLE:
        is_breakable = true;
        item = static_cast<item_type>(t_tile->data & 0xff);
        data |= 0x1000;
        break;
    default:
        break;
    }

    setData(data);
}

void tile_belt::tick() {
    entity **ents = g_map->getWorld()->findEntities(tx, ty);
    float speedx = 0;
    float speedy = 0;
    switch (direction) {
    case BELT_DIR_UP:
        speedy = -BELT_SPEED / TICKRATE;
        break;
    case BELT_DIR_DOWN:
        speedy = BELT_SPEED / TICKRATE;
        break;
    case BELT_DIR_LEFT:
        speedx = -BELT_SPEED / TICKRATE;
        break;
    case BELT_DIR_RIGHT:
        speedx = BELT_SPEED / TICKRATE;
        break;
    }
    for (size_t i = 0; i < SEARCH_SIZE; ++i) {
        entity *ent = ents[i];
        if (!ent) break;
        ent_movable *mov = dynamic_cast<ent_movable *>(ent);
        if (!mov) continue;
        mov->move(speedx, speedy);
    }
}

bool tile_belt::bombHit() {
    if (is_breakable) {
        game_world *world = g_map->getWorld();
        world->addEntity(new broken_wall(world, t_tile, item));
        is_breakable = false;
        setData((direction & 0x3) << 10);
        return true;
    } else {
        return false;
    }
}
