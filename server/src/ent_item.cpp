#include "ent_item.h"

#include "main.h"
#include "player.h"

game_item::game_item(game_world *world, tile *t, uint8_t it_type) : entity(world, TYPE_ITEM), item_type(it_type) {
    tx = world->getMap().getTileX(t);
    ty = world->getMap().getTileY(t);

    destroyed = false;
    life_ticks = TICKRATE * 2 / 3;
}

void game_item::tick() {
    if (destroyed) {
        if (life_ticks == 0) {
            destroy();
        }
        --life_ticks;
    }
}

void game_item::pickup(player *p) {
    switch (item_type) {
    case ITEM_BOMB:
        ++p->num_bombs;
        break;
    case ITEM_FIRE:
        ++p->explosion_size;
        break;
    case ITEM_ROLLERBLADE:
        p->speed += 1.f;
        break;
    case ITEM_KICK:
        p->can_kick = true;
        break;
    case ITEM_PUNCH:
        p->can_punch = true;
        break;
    case ITEM_SKULL:
        p->skull_ticks = SKULL_LIFE;
        p->skull_effect = random_engine() % 5 + 1;
        break;
    }
    destroy();
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
