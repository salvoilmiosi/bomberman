#include "ent_broken_wall.h"
#include "ent_item.h"

broken_wall::broken_wall(game_world *world, tile *t) : entity(world, TYPE_BROKEN_WALL) {
    tx = world->getMap().getTileX(t);
    ty = world->getMap().getTileY(t);

    life_ticks = BROKEN_WALL_LIFE;

    drop_item = t->type == tile::TILE_ITEM;
    item_type = t->item_type;

    do_send_updates = false;
}

void broken_wall::tick() {
    if (life_ticks == 0) {
        if (drop_item) {
            world->addEntity(new game_item(world, world->getMap().getTile(tx,ty), item_type));
        }
        destroy();
    }
    --life_ticks;
}

void broken_wall::writeEntity(packet_ext &packet){
    packet.writeShort(2);
    packet.writeChar(tx);
    packet.writeChar(ty);
}
