#ifndef __ENT_BOMB_H__
#define __ENT_BOMB_H__

#include "ent_movable.h"

static const float BOMB_LIFE = TICKRATE * 3;

static const float BOMB_HEIGHT = 500.f;
static const float KICK_SPEED = 600.f;
static const float PUNCH_SPEED = 720.f;
static const float PUNCH_SPEED_BOUNCE = 400.f;
static const float PUNCH_SPEED_Z = 600.f;
static const float PUNCH_SPEED_Z_BOUNCE = 360.f;
static const float PUNCH_Z_ACCEL = 2880.f;

class bomb : public ent_movable {
private:
    class player *const planter;
    class player *kicker = nullptr;

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

    bool kick(player *kicker, float dx, float dy);
    bool punch(player *puncher);

    bool move(float dx, float dy);

    void stopKick();

    void explode();

    bool isFlying() {
        return flying;
    }

private:
    byte_array toByteArray();
};

#endif // __ENT_BOMB_H__
