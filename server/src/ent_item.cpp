#include "ent_item.h"

#include "random.h"
#include "player.h"
#include "game_sound.h"
#include "game_world.h"
#include "tile_trampoline.h"

const int SKULL_LIFE = TICKRATE * 20;
const int ITEM_EXPLOSION_LIFE = TICKRATE * 2 / 3;
const int TRAMPOLINE_JUMP_TICKS = TICKRATE * 5 / 2;

game_item::game_item(game_world &world, const tile &t, item_type type) : entity(world, TYPE_ITEM), type(type) {
    tx = world.getMap().getTileX(t);
    ty = world.getMap().getTileY(t);

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

bool game_item::pickup() {
    if (exploded) return false;

    switch (type) {
    case ITEM_NONE:
        destroy();
        return false;
    case ITEM_SKULL:
        world.playWave(WAV_SKULL);
        break;
    default:
        world.playWave(WAV_PICKUP);
        break;
    }

    destroy();
    return true;
}

byte_array game_item::toByteArray() {
    byte_array ba;

    ba.writeChar(tx);
    ba.writeChar(ty);
    ba.writeChar(type);
    ba.writeChar(exploded ? 1 : 0);

    return ba;
}

void game_item::explode() {
    exploded = true;
}

bool game_item::isAlive() {
    return !exploded;
}
