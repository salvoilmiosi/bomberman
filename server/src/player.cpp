#include "player.h"

#include <cstdio>
#include <cstring>
#include <cstdlib>

#include "server.h"
#include "bullet.h"

player::player(input_handler *handler) : game_obj(TYPE_PLAYER), handler(handler) {
    if (!handler) {
        fprintf(stderr, "Player must have a handler\n");
    }

    memset(player_name, 0, NAME_SIZE);

    Uint8 r = rand() % 0xff;
    Uint8 g = rand() % 0xff;
    Uint8 b = rand() % 0xff;
    color = (r << 24) | (g << 16) | (b << 8) | 0xff;

    respawn();
}

void player::respawn() {
    fx = rand() % 800;
    fy = rand() % 600;

    speedx = 0.f;
    speedy = 0.f;

    fire_timer = 0;

    respawn_ticks = 0;

    health_points = MAX_HEALTH;

    do_send_updates = true;
}

void player::setName(const char *name) {
    strncpy(player_name, name, NAME_SIZE);
}

void player::tick(game_server *server) {
    if (!isAlive()) {
        if (respawn_ticks > 0) {
            --respawn_ticks;
            if (respawn_ticks < 200) {
                do_send_updates = false;
            }
            return;
        } else {
            respawn();
        }
    }

    if (handler->isDown(USR_UP)) {
        speedy -= ACCEL;
        if (speedy < -MAX_SPEED) speedy = -MAX_SPEED;
    } else if (handler->isDown(USR_DOWN)) {
        speedy += ACCEL;
        if (speedy > MAX_SPEED) speedy = MAX_SPEED;
    } else {
        speedy *= FRICTION;
    }
    if (handler->isDown(USR_LEFT)) {
        speedx -= ACCEL;
        if (speedx < -MAX_SPEED) speedx = -MAX_SPEED;
    } else if (handler->isDown(USR_RIGHT)) {
        speedx += ACCEL;
        if (speedx > MAX_SPEED) speedx = MAX_SPEED;
    } else {
        speedx *= FRICTION;
    }
    fx += speedx;
    fy += speedy;
    if (speedx > 0 && fx > 800) speedx = -speedx;
    if (speedx < 0 && fx < 0)   speedx = -speedx;
    if (speedy > 0 && fy > 600) speedy = -speedy;
    if (speedy < 0 && fy < 0)   speedy = -speedy;

    if (handler->isDown(USR_FIRE)) {
        if (fire_timer <= 0) {
            fire_timer = 5;
            bullet *b = new bullet(this, handler->getMouseX(), handler->getMouseY());
            speedx -= b->getSpeedX() * RECOIL;
            speedy -= b->getSpeedY() * RECOIL;
            server->addObject(b);
        }
    }

    if (fire_timer > 0) {
        --fire_timer;
    }

    if (handler->isPressed(USR_ACTION)) {
        heal(10);
    }
}

void player::heal(short amt) {
    health_points += 10;
    if (health_points > MAX_HEALTH) {
        health_points = MAX_HEALTH;
    }
}

void player::hit(short damage, float knockback_x, float knockback_y) {
    health_points -= damage;
    speedx += knockback_x;
    speedy += knockback_y;

    if (!isAlive()) {
        respawn_ticks = RESPAWN_TIME;
    }
}

void player::writeObject(packet_ext &packet) {
    packet.writeShort(NAME_SIZE + 19);
    packet.writeInt(fx);
    packet.writeInt(fy);
    packet.writeString(player_name, NAME_SIZE);
    packet.writeInt(color);
    packet.writeShort(health_points);
}
