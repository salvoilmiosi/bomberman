#include "tile_belt.h"

#include "ent_bomb.h"
#include "player.h"

tile_belt::tile_belt(tile *t_tile, game_map *g_map, uint8_t direction) : tile_entity(SPECIAL_BELT, t_tile, g_map), direction(direction) {
    tx = g_map->getTileX(t_tile);
    ty = g_map->getTileY(t_tile);

    uint16_t data = (direction & 0x3) << 10;

    switch(t_tile->type) {
    case TILE_BREAKABLE:
        is_breakable = true;
        data |= 0x1000;
        break;
    case TILE_ITEM:
        is_breakable = true;
        tile_data = t_tile->data & 0xff;
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
        switch(ent->getType()) {
        case TYPE_BOMB:
        {
            bomb *b = dynamic_cast<bomb *>(ent);
            b->move(speedx, speedy);
            break;
        }
        case TYPE_PLAYER:
        {
            player *p = dynamic_cast<player *>(ent);
            p->move(speedx, speedy);
            break;
        }
        default:
            break;
        }
    }
}

bool tile_belt::bombHit() {
    if (is_breakable) {
        game_world *world = g_map->getWorld();
        world->addEntity(new broken_wall(world, t_tile, static_cast<item_type>(tile_data)));
        is_breakable = false;
        tile_data = 0;
        setData((direction & 0x3) << 10);
        return true;
    } else {
        return false;
    }
}
