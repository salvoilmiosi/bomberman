#ifndef __ENT_ITEM_H__
#define __ENT_ITEM_H__

#include "game_world.h"

static const int SKULL_LIFE = TICKRATE * 20;

static const int ITEM_EXPLOSION_LIFE = TICKRATE * 2 / 3;

class game_item : public entity {
private:
    uint8_t tx, ty;

    int life_ticks;
    bool exploded;

    item_type type;

public:
    game_item(game_world &world, const tile &t, item_type type);

public:
    void tick();

    byte_array toByteArray();

    const uint8_t getTileX() {
        return tx;
    }

    const uint8_t getTileY() {
        return ty;
    }

    const item_type getItemType() {
        return type;
    }

    bool pickup();
    void explode();

    bool isAlive();
};

#endif // __ENT_ITEM_H__
