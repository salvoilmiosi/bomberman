#ifndef __ENT_ITEM_H__
#define __ENT_ITEM_H__

#include "entity.h"

extern const int SKULL_LIFE;

extern const int ITEM_EXPLOSION_LIFE;

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

class game_item : public entity {
private:
    uint8_t tx, ty;

    int life_ticks;
    bool exploded;

    item_type type;

public:
    game_item(class game_world &world, const class tile &t, item_type type);

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
