#ifndef __TILE_BELT_H__
#define __TILE_BELT_H__

#include "game_map.h"
#include "ent_broken_wall.h"

static const float BELT_SPEED = 100.f;

class tile_belt : public tile_entity {
private:
    uint8_t tx;
    uint8_t ty;

    direction belt_direction;

    bool is_breakable = false;
    item_type item = ITEM_NONE;

public:
    tile_belt(tile &t_tile, game_map &g_map, direction belt_direction);

    void tick();

    bool isWalkable() {
        return !is_breakable;
    }

    bool bombHit();
};

#endif // __TILE_BELT_H__
