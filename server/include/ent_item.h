#ifndef __ENT_ITEM_H__
#define __ENT_ITEM_H__

#include "game_world.h"

static const uint8_t ITEM_BOMB = 1;
static const uint8_t ITEM_FIRE = 2;
static const uint8_t ITEM_ROLLERBLADE = 3;
static const uint8_t ITEM_KICK = 4;
static const uint8_t ITEM_PUNCH = 5;
static const uint8_t ITEM_SKULL = 6;

class game_item : public entity {
private:
    uint8_t tx, ty;

    int life_ticks;
    bool destroyed;

    uint8_t item_type;

public:
    game_item(game_world *world, tile *t, uint8_t it_type);

public:
    void tick();

    void writeEntity(packet_ext &packet);

    uint8_t getTileX() const {
        return tx;
    }

    uint8_t getTileY() const {
        return ty;
    }

    uint8_t getItemType() const {
        return item_type;
    }

    void pickup(class player *p);
    void explode();
};

#endif // __ENT_ITEM_H__
