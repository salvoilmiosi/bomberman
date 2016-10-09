#include "ent_bomb.h"
#include "ent_explosion.h"
#include "ent_item.h"

#include "player.h"
#include "main.h"

bomb::bomb(game_world *world, player *p) : entity(world, TYPE_BOMB), p(p) {
    fx = p->getTileX() * TILE_SIZE;
    fy = p->getTileY() * TILE_SIZE;
    fz = 0.f;

    life_ticks = BOMB_LIFE;

    if (p->skull_effect == SKULL_LOW_POWER) {
        explode_size = 1;
    } else {
        explode_size = p->explosion_size;
    }

    piercing = false;
    kicked = false;

    do_send_updates = false;

    speedx = 0.f;
    speedy = 0.f;
}

void bomb::kick(uint8_t direction) {
    if (kicked || punched) return;

    switch (direction) {
    case 0:
        speedy = -KICK_SPEED;
        break;
    case 1:
        speedy = KICK_SPEED;
        break;
    case 2:
        speedx = -KICK_SPEED;
        break;
    case 3:
        speedx = KICK_SPEED;
        break;
    default:
        return;
    }

    kicked = true;
    do_send_updates = true;
}

void bomb::punch(uint8_t direction) {
    if (kicked) kicked = false;
    if (punched) return;

    switch (direction) {
    case 0:
        speedy = -PUNCH_SPEED;
        break;
    case 1:
        speedy = PUNCH_SPEED;
        break;
    case 2:
        speedx = -PUNCH_SPEED;
        break;
    case 3:
        speedx = PUNCH_SPEED;
        break;
    default:
        return;
    }

    speedz = PUNCH_SPEED_Z;

    punched = true;
    do_send_updates = true;
}

void bomb::tick() {
    if (kicked) {
        float to_fx = fx + speedx;
        float to_fy = fy + speedy;

        float check_fx = to_fx;
        float check_fy = to_fy;

        if (speedx > 0) {
            check_fx += TILE_SIZE / 2;
        } else if (speedx < 0) {
            check_fx -= TILE_SIZE / 2;
        } else if (speedy > 0) {
            check_fy += TILE_SIZE / 2;
        } else if (speedy < 0) {
            check_fy -= TILE_SIZE / 2;
        }

        int to_tx = (check_fx / TILE_SIZE) + 0.5f;
        int to_ty = (check_fy / TILE_SIZE) + 0.5f;

        if ((to_tx == getTileX() && to_ty == getTileY()) ||
            world->isWalkable(to_tx, to_ty, WALK_BLOCK_PLAYERS)) {
            fx = to_fx;
            fy = to_fy;
        } else {
            kicked = false;
            fx = getTileX() * TILE_SIZE;
            fy = getTileY() * TILE_SIZE;
        }
    } else if (punched) {
        fx += speedx;
        fy += speedy;
        fz += speedz;

        speedz -= Z_ACCEL;

        if (speedx > 0 && fx > TILE_SIZE * MAP_WIDTH) {
            fx -= TILE_SIZE * MAP_WIDTH;
        }
        if (speedx < 0 && fx < 0) {
            fx += TILE_SIZE * MAP_WIDTH;
        }
        if (speedy > 0 && fy > TILE_SIZE * MAP_HEIGHT) {
            fy -= TILE_SIZE * MAP_HEIGHT;
        }
        if (speedy < 0 && fy < 0) {
            fy += TILE_SIZE * MAP_HEIGHT;
        }

        if (fz <= 0) {
            if (world->isWalkable(getTileX(), getTileY(), WALK_BLOCK_PLAYERS)) {
                fx = getTileX() * TILE_SIZE;
                fy = getTileY() * TILE_SIZE;
                fz = 0.f;
                punched = false;
            } else {
                if (speedx > 0) speedx = PUNCH_SPEED_BOUNCE;
                else if (speedx < 0) speedx = -PUNCH_SPEED_BOUNCE;
                if (speedy > 0) speedy = PUNCH_SPEED_BOUNCE;
                else if (speedy < 0) speedy = -PUNCH_SPEED_BOUNCE;
                speedz = PUNCH_SPEED_Z_BOUNCE;
            }
        }
    }

    if (life_ticks == 0) {
        explode();
    }
    --life_ticks;
}

void bomb::explode() {
    if (exploded) return;

    ++p->num_bombs;

    exploded = true;
    destroy();
    world->addEntity(new explosion(world, this));
}

void bomb::writeEntity(packet_ext &packet) {
    packet.writeShort(14);
    packet.writeInt(fx);
    packet.writeInt(fy);
    packet.writeInt(fz);
    packet.writeShort(p->getID());
}
