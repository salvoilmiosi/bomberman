#include "ent_item.h"

#include "main.h"
#include "player.h"
#include "game_sound.h"

game_item::game_item(game_world *world, tile *t, uint8_t it_type) : entity(world, TYPE_ITEM), item_type(it_type) {
    tx = world->getMap().getTileX(t);
    ty = world->getMap().getTileY(t);

    exploded = false;
    life_ticks = TICKRATE * 2 / 3;
}

void game_item::tick() {
    if (exploded) {
        if (life_ticks == 0) {
            destroy();
        }
        --life_ticks;
    }
}

void game_item::pickup(player *p) {
    if (exploded) return;

    if (item_type == ITEM_SKULL) {
        world->playWave(WAV_SKULL);
    } else {
        world->playWave(WAV_PICKUP);
    }

    switch (item_type) {
    case ITEM_BOMB:
        ++p->num_bombs;
        if (p->num_bombs > 10) {
            p->num_bombs = 10;
        }
        break;
    case ITEM_FIRE:
        ++p->explosion_size;
        if (p->explosion_size > 10) {
            p->explosion_size = 10;
        }
        break;
    case ITEM_ROLLERBLADE:
        p->speed += 80.f;
        break;
    case ITEM_KICK:
        p->pickups |= PICKUP_HAS_KICK;
        break;
    case ITEM_PUNCH:
        p->pickups |= PICKUP_HAS_PUNCH;
        break;
    case ITEM_SKULL:
        p->skull_ticks = SKULL_LIFE;
        p->skull_effect = random_engine() % 5 + 1;
        break;
    case ITEM_FULL_FIRE:
        p->explosion_size = 10;
        break;
    case ITEM_REDBOMB:
        p->pickups |= PICKUP_HAS_REDBOMB;
        break;
    case ITEM_REMOCON:
        p->pickups |= PICKUP_HAS_REMOCON;
        break;
    }
    destroy();
}

byte_array game_item::toByteArray() {
    byte_array ba;

    ba.writeChar(tx);
    ba.writeChar(ty);
    ba.writeChar(item_type);
    ba.writeChar(exploded ? 1 : 0);

    return ba;
}

void game_item::explode() {
    exploded = true;
}

bool game_item::isAlive() {
    return !exploded;
}
