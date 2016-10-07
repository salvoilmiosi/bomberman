#include "ent_item.h"

#include "main.h"

game_item::game_item(game_world *world, tile *t) : entity(world, TYPE_ITEM) {
    tx = world->getMap().getTileX(t);
    ty = world->getMap().getTileY(t);

    destroyed = false;
    life_ticks = TICKRATE * 2 / 3;

    item_type = (random_engine() % 3) + ITEM_ADD_BOMB;
}

void game_item::tick() {
    if (destroyed) {
        if (life_ticks == 0) {
            destroy();
        }
        --life_ticks;
    }
}

void game_item::writeEntity(packet_ext &packet) {
    packet.writeShort(4);
    packet.writeChar(tx);
    packet.writeChar(ty);
    packet.writeChar(item_type);
    packet.writeChar(destroyed ? 1 : 0);
}

void game_item::explode() {
    destroyed = true;
}
