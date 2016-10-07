#include "player.h"

#include "ent_bomb.h"

#include <cstdio>
#include <cstring>
#include <cstdlib>

player::player(game_world *world, input_handler *handler) : entity(world, TYPE_PLAYER), handler(handler) {
    if (!handler) {
        fprintf(stderr, "Player must have a handler\n");
    }

    memset(player_name, 0, NAME_SIZE);

    uint8_t r = rand() % 0xff;
    uint8_t g = rand() % 0xff;
    uint8_t b = rand() % 0xff;
    color = (r << 24) | (g << 16) | (b << 8) | 0xff;

    do_send_updates = false;
}

void player::respawn(float x, float y) {
    fx = x;
    fy = y;

    spawned = true;
    alive = true;

    speed = 3.f;
    explosion_size = 1;
    num_bombs = 1;

    do_send_updates = true;
}

void player::kill() {
    alive = false;
}

void player::setName(const char *name) {
    strncpy(player_name, name, NAME_SIZE);
}

void player::handleInput() {
    // how the fuck do I make this better
    movement_keys_down[0] = handler->isDown(USR_UP);
    movement_keys_down[1] = handler->isDown(USR_DOWN);
    movement_keys_down[2] = handler->isDown(USR_LEFT);
    movement_keys_down[3] = handler->isDown(USR_RIGHT);

    for (int i=0; i<4; ++i) {
        if (movement_keys_down[i]) {
            if (!movement_keys_pressed[i]) {
                movement_keys_pressed[i] = true;
                movement_priority.push_front(i);
            }
        } else {
            movement_keys_pressed[i] = false;
        }
    }

    while(! movement_priority.empty()) {
        uint8_t prio = movement_priority.front();
        if (movement_keys_down[prio]) {
            switch (prio) {
            case 0:
                fy -= speed;
                break;
            case 1:
                fy += speed;
                break;
            case 2:
                fx -= speed;
                break;
            case 3:
                fx += speed;
                break;
            }
            break;
        } else {
            movement_priority.pop_front();
        }
    }

    if (handler->isPressed(USR_FIRE)) {
        if (num_bombs > 0) {
            world->addEntity(new bomb(world, this));
            --num_bombs;
        }
    }
}

void player::tick() {
    if (alive) {
        handleInput();
    }
}

void player::writeEntity(packet_ext &packet) {
    packet.writeShort(NAME_SIZE + 16);
    packet.writeInt(fx);
    packet.writeInt(fy);
    packet.writeString(player_name, NAME_SIZE);
    packet.writeInt(color);
    packet.writeChar(alive ? 1 : 0);
    packet.writeChar(spawned ? 1 : 0);
}
