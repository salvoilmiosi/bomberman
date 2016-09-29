#ifndef __BULLET_H__
#define __BULLET_H__

#include "game_obj.h"
#include "player.h"

static const float SPEED = 13.f;
static const float RECOIL = 0.3f;
static const int BULLET_LIFE = 60;
static const int DAMAGE = 4;
static const float KNOCKBACK = 0.1f;

class bullet: public game_obj {
private:
    float fx, fy;

    float speedx, speedy;

    int alive_time;

    player *owner;

public:
    bullet(player *p, int dir_x, int dir_y);

public:
    void tick(class game_server *server);

    void writeObject(packet_ext &packet);

    player *getOwner() {
        return owner;
    }

    float getSpeedX() {
        return speedx;
    }

    float getSpeedY() {
        return speedy;
    }

private:
    bool intersects(player *p);
};

#endif // __BULLET_H__
