#ifndef __ENT_BOMB_H__
#define __ENT_BOMB_H__

#include "game_world.h"

static const float KICK_SPEED = 10.f;
static const float PUNCH_SPEED = 15.f;
static const float PUNCH_SPEED_Z = 10.f;
static const float Z_ACCEL = 0.8f;
static const float XY_BOUNCE = 0.8f;
static const float Z_BOUNCE = 0.65f;

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

class bomb : public entity {
private:
    class player *p;

    float fx;
    float fy;
    float fz;

    float speedx;
    float speedy;
    float speedz;

    int life_ticks;

    int explode_size;

    bool exploded = false;
    bool kicked = false;
    bool punched = false;

    bool piercing;

    friend class explosion;

public:
    bomb(game_world *world, class player *p);

public:
    void tick();

    void kick(uint8_t direction);
    void punch(uint8_t direction);

    uint8_t getTileX() const {
        return (fx / TILE_SIZE) + 0.5f;
    }

    uint8_t getTileY() const {
        return (fy / TILE_SIZE) + 0.5f;
    }

private:
    void writeEntity(packet_ext &packet);

    void explode();
};

#endif // __ENT_BOMB_H__
