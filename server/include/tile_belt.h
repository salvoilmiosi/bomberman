#ifndef __TILE_BELT_H__
#define __TILE_BELT_H__

#include "game_map.h"
#include "ent_broken_wall.h"

static const uint8_t BELT_DIR_UP = 0;
static const uint8_t BELT_DIR_DOWN = 1;
static const uint8_t BELT_DIR_LEFT = 2;
static const uint8_t BELT_DIR_RIGHT = 3;

static const float BELT_SPEED = 100.f;

class tile_belt : public tile_entity {
private:
    uint8_t tx;
    uint8_t ty;

    uint8_t direction;

    bool is_breakable = false;
    item_type item = ITEM_NONE;

public:
    tile_belt(tile *t_tile, game_map *g_map, uint8_t direction);

    void tick();

    bool isWalkable() {
        return !is_breakable;
    }

    bool bombHit();
};

#endif // __TILE_BELT_H__
