#include "bullet.h"
#include <math.h>

#include "server.h"

bullet::bullet(player *p, int dir_x, int dir_y) : game_obj(TYPE_BULLET) {
    owner = p;
    fx = p->fx;
    fy = p->fy;

    speedx = dir_x - fx;
    speedy = dir_y - fy;
    float dist = sqrtf(speedx * speedx + speedy * speedy);
    speedx = speedx / dist * SPEED;
    speedy = speedy / dist * SPEED;

    alive_time = BULLET_LIFE;
}

bool bullet::intersects(player *p) {
    if (p->isAlive()) {
        int dx = p->fx - fx;
        int dy = p->fy - fy;
        return (dx*dx + dy*dy) < 100;
    } else {
        return false;
    }
}

void bullet::tick(game_server *server) {
    fx += speedx;
    fy += speedy;
    --alive_time;
    if (alive_time <= 0) {
        server->removeObject(this);
    } else {
        for (game_obj *it : server->getObjects()) {
            if (it && it->isNotDestroyed() && it->getType() == TYPE_PLAYER) {
                player *p = (player *) it;
                if (intersects(p)) {
                    p->hit(DAMAGE, speedx * KNOCKBACK, speedy * KNOCKBACK);
                    server->removeObject(this);
                    break;
                }
            }
        }
    }
}

void bullet::writeObject(packet_ext &packet) {
    packet.writeShort(11);
    packet.writeShort(owner->getID());
    packet.writeInt(fx);
    packet.writeInt(fy);
}
