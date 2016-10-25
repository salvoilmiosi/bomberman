#ifndef __ENT_ITEM_H__
#define __ENT_ITEM_H__

#include "game_world.h"

enum item_type {
    ITEM_NONE,
    ITEM_BOMB,
    ITEM_FIRE,
    ITEM_ROLLERBLADE,
    ITEM_KICK,
    ITEM_PUNCH,
    ITEM_SKULL,
    ITEM_FULL_FIRE,
    ITEM_REDBOMB,
    ITEM_REMOCON
};

enum disease_effect {
    SKULL_NONE,
    SKULL_RAPID_PACE,
    SKULL_SLOW_PACE,
    SKULL_DIARRHEA,
    SKULL_LOW_POWER,
    SKULL_CONSTIPATION
};

static const int SKULL_LIFE = TICKRATE * 20;

static const int ITEM_EXPLOSION_LIFE = TICKRATE * 2 / 3;

class game_item : public entity {
private:
    uint8_t tx, ty;

    int life_ticks;
    bool exploded;

    item_type type;

public:
    game_item(game_world *world, tile *t, item_type type);

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

    void pickup(class player *p);
    void explode();

    bool isAlive();
};

#endif // __ENT_ITEM_H__
