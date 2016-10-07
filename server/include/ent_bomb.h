#ifndef __ENT_BOMB_H__
#define __ENT_BOMB_H__

#include "game_world.h"

class explosion : public entity {
private:
    uint16_t bomb_id;

    uint8_t tx;
    uint8_t ty;

    uint8_t len_l, len_t, len_r, len_b;
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

class bomb : public entity {
private:
    class player *p;

    uint8_t tx;
    uint8_t ty;

    int life_ticks;

    int explode_size;

    bool exploded = false;
    bool piercing;

    friend class explosion;

public:
    bomb(game_world *world, class player *p);

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

    void explode();
};

#endif // __ENT_BOMB_H__
