#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "game_obj.h"
#include "input_handler.h"

static const float ACCEL = 0.7f;
static const float MAX_SPEED = 10.f;
static const float FRICTION = 0.8f;

static const size_t NAME_SIZE = 32;

static const int RESPAWN_TIME = 300;
static const int MAX_HEALTH = 100;

class player: public game_obj {
private:
    input_handler *handler;

    float speedx, speedy;

    Uint32 color;

    char player_name[NAME_SIZE];

    int fire_timer;

    short health_points;

    int respawn_ticks;

public:
    float fx, fy;

public:
    player(input_handler *handler);

public:
    void respawn();

    void setName(const char *name);

    void tick(class game_server *server);

    void writeObject(packet_ext &packet);

    void hit(short damage, float knockback_x, float knockback_y);
    void heal(short amount);

    bool isAlive() {
        return health_points > 0;
    }
};

#endif // __PLAYER_H__
