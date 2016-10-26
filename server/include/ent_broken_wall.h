#ifndef __ENT_BROKEN_WALL_H__
#define __ENT_BROKEN_WALL_H__

#include "game_world.h"
#include "ent_item.h"

static const float BROKEN_WALL_LIFE = TICKRATE * 2 / 3;

class broken_wall : public entity {
private:
    uint8_t tx;
    uint8_t ty;

    uint8_t zone;

    int life_ticks;

    item_type item = ITEM_NONE;

public:
    broken_wall(game_world *world, tile *t, item_type item = ITEM_NONE);

public:
    void tick();

    const uint8_t getTileX() {
        return tx;
    }

    const uint8_t getTileY() {
        return ty;
    }

private:
    byte_array toByteArray();
};

#endif // __ENT_BROKEN_WALL_H__
