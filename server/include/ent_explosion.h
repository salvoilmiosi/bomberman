#ifndef __ENT_EXPLOSION_H__
#define __ENT_EXPLOSION_H__

#include "game_world.h"

static const float EXPLOSION_LIFE = TICKRATE * 2 / 3;

class explosion : public entity {
private:
    uint16_t bomb_id;

    uint8_t tx;
    uint8_t ty;

    uint8_t len_l = 0, len_t = 0, len_r = 0, len_b = 0;
    bool trunc_l, trunc_t, trunc_r, trunc_b;

    int life_ticks;

    bool piercing;

public:
    explosion(game_world *world, class bomb *b);

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

    uint8_t destroyTiles(int dx, int dy, bool *trunc);
    void checkPlayers(int dx, int dy);
};

#endif // __ENT_EXPLOSION_H__
