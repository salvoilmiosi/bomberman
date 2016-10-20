#ifndef __ENT_BOMB_H__
#define __ENT_BOMB_H__

#include "game_world.h"

static const float BOMB_LIFE = TICKRATE * 3;

static const float BOMB_HEIGHT = 500.f;
static const float KICK_SPEED = 600.f;
static const float PUNCH_SPEED = 720.f;
static const float PUNCH_SPEED_BOUNCE = 400.f;
static const float PUNCH_SPEED_Z = 600.f;
static const float PUNCH_SPEED_Z_BOUNCE = 360.f;
static const float PUNCH_Z_ACCEL = 2880.f;

class bomb : public entity {
private:
    class player *p = nullptr;

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
    bool flying = false;

    int kick_ticks;

    bool piercing = false;
    bool remocon = false;

    friend class explosion;

public:
    bomb(game_world *world, class player *p);

    bomb(game_world *world, int tx, int ty);

public:
    void tick();

    bool kick(uint8_t direction);
    bool punch(uint8_t direction);

    void stopKick();

    void explode();

    bool isFlying() {
        return flying;
    }

    uint8_t getTileX() const {
        return (fx / TILE_SIZE) + 0.5f;
    }

    uint8_t getTileY() const {
        return (fy / TILE_SIZE) + 0.5f;
    }

private:
    byte_array toByteArray();
};

#endif // __ENT_BOMB_H__
