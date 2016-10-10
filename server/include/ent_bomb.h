#ifndef __ENT_BOMB_H__
#define __ENT_BOMB_H__

#include "game_world.h"

static const float BOMB_LIFE = TICKRATE * 3;

static const float KICK_SPEED = 600.f;
static const float PUNCH_SPEED = 720.f;
static const float PUNCH_SPEED_BOUNCE = 400.f;
static const float PUNCH_SPEED_Z = 600.f;
static const float PUNCH_SPEED_Z_BOUNCE = 360.f;
static const float Z_ACCEL = 1550.f;

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

    int punched_ticks;

    bool piercing;
    bool remocon;

    friend class explosion;

public:
    bomb(game_world *world, class player *p);

public:
    void tick();

    void kick(uint8_t direction);
    void punch(uint8_t direction);

    bool isFlying() {
        return punched;
    }

    uint8_t getTileX() const {
        return (fx / TILE_SIZE) + 0.5f;
    }

    uint8_t getTileY() const {
        return (fy / TILE_SIZE) + 0.5f;
    }

private:
    byte_array toByteArray();

    void explode();
};

#endif // __ENT_BOMB_H__
