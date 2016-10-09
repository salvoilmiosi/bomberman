#ifndef __ENT_BROKEN_WALL_H__
#define __ENT_BROKEN_WALL_H__

#include "game_world.h"

static const float BROKEN_WALL_LIFE = TICKRATE * 2 / 3;

class broken_wall : public entity {
private:
    uint8_t tx;
    uint8_t ty;

    int life_ticks;

    bool drop_item;
    uint8_t item_type;

public:
    broken_wall(game_world *world, tile *t);

public:
    void tick();

    uint8_t getTileX() const {
        return tx;
    }

    uint8_t getTileY() const {
        return ty;
    }

private:
    void writeEntity(packet_ext &packet);
};

#endif // __ENT_BROKEN_WALL_H__
