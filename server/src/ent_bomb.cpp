#include "ent_bomb.h"
#include "ent_explosion.h"
#include "ent_item.h"

#include "player.h"
#include "main.h"
#include "game_sound.h"

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

    piercing = p->pickups & PICKUP_HAS_REDBOMB;
    remocon = p->pickups & PICKUP_HAS_REMOCON;

    kicked = false;
    punched = false;

    do_send_updates = false;

    speedx = 0.f;
    speedy = 0.f;

    world->playWave(WAV_PLANT);
}

bomb::bomb(game_world *world, int tx, int ty) : entity(world, TYPE_BOMB) {
    fx = tx * TILE_SIZE;
    fy = ty * TILE_SIZE;
    fz = BOMB_HEIGHT;

    punched = true;
    speedx = 0.f;
    speedy = 0.f;
    speedz = 0.f;

    explode_size = 2;

    life_ticks = BOMB_LIFE;

    piercing = false;
    remocon = false;
    kicked = false;
    do_send_updates = true;

    int dir = random_engine() % 4;
    switch(dir) {
    case 0:
        speedx = 1.f;
        break;
    case 1:
        speedx = -1.f;
        break;
    case 2:
        speedy = 1.f;
        break;
    case 3:
        speedy = -1.f;
        break;
    }
}

void bomb::kick(uint8_t direction) {
    if (kicked || punched) return;

    speedx = 0;
    speedy = 0;
    kick_ticks = 0;

    switch (direction) {
    case 0:
        speedy = -KICK_SPEED / TICKRATE;
        break;
    case 1:
        speedy = KICK_SPEED / TICKRATE;
        break;
    case 2:
        speedx = -KICK_SPEED / TICKRATE;
        break;
    case 3:
        speedx = KICK_SPEED / TICKRATE;
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

    world->playWave(WAV_PUNCH);

    speedx = 0;
    speedy = 0;

    switch (direction) {
    case 0:
        speedy = -PUNCH_SPEED / TICKRATE;
        break;
    case 1:
        speedy = PUNCH_SPEED / TICKRATE;
        break;
    case 2:
        speedx = -PUNCH_SPEED / TICKRATE;
        break;
    case 3:
        speedx = PUNCH_SPEED / TICKRATE;
        break;
    default:
        return;
    }

    speedz = PUNCH_SPEED_Z / TICKRATE;

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
            world->isWalkable(to_tx, to_ty, WALK_BLOCK_PLAYERS | WALK_BLOCK_ITEMS)) {
            fx = to_fx;
            fy = to_fy;

            ++kick_ticks;
            if (kick_ticks % 5 == 0) {
                world->playWave(WAV_SLIDE);
            }
        } else {
            kicked = false;
            fx = getTileX() * TILE_SIZE;
            fy = getTileY() * TILE_SIZE;
            if (kick_ticks > 1) {
                world->playWave(WAV_HARDHIT);
            }
        }
    } else if (punched) {
        if (speedx > 1 || speedx < -1) fx += speedx;
        if (speedy > 1 || speedy < -1) fy += speedy;
        fz += speedz;

        speedz -= PUNCH_Z_ACCEL / (TICKRATE * TICKRATE);

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
            fz = 0.f;
            world->playWave(WAV_BOUNCE);
            if (world->isWalkable(getTileX(), getTileY(), WALK_BLOCK_PLAYERS | WALK_BLOCK_ITEMS)) {
                fx = getTileX() * TILE_SIZE;
                fy = getTileY() * TILE_SIZE;
                punched = false;
            } else {
                if (speedx > 0) speedx = PUNCH_SPEED_BOUNCE / TICKRATE;
                else if (speedx < 0) speedx = -PUNCH_SPEED_BOUNCE / TICKRATE;
                if (speedy > 0) speedy = PUNCH_SPEED_BOUNCE / TICKRATE;
                else if (speedy < 0) speedy = -PUNCH_SPEED_BOUNCE / TICKRATE;
                speedz = PUNCH_SPEED_Z_BOUNCE / TICKRATE;
            }
        }
    }

    if (!remocon) {
        if (life_ticks <= 0 && ! isFlying()) {
            explode();
        }
        --life_ticks;
    }
}

void bomb::explode() {
    if (exploded) return;

    if (p) {
        p->explodedBomb(this);
    }

    exploded = true;
    destroy();
    world->addEntity(new explosion(world, this));
}

byte_array bomb::toByteArray() {
    byte_array ba;

    ba.writeInt(fx);
    ba.writeInt(fy);
    ba.writeInt(fz);
    if (p) {
        ba.writeShort(p->getID());
    } else {
        ba.writeShort(0);
    }
    uint8_t flags = 0;
    flags |= (1 << 0) * piercing;
    flags |= (1 << 1) * remocon;
    ba.writeChar(flags);

    return ba;
}
