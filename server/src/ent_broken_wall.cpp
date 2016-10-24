#include "ent_broken_wall.h"
#include "ent_item.h"

broken_wall::broken_wall(game_world *world, tile *t) : entity(world, TYPE_BROKEN_WALL) {
    tx = world->getMap().getTileX(t);
    ty = world->getMap().getTileY(t);
    zone = world->getMap().getZone();

    life_ticks = BROKEN_WALL_LIFE;

    drop_item = t->type == TILE_ITEM;
    if (drop_item) {
        type = static_cast<item_type>(t->data);
    }

    do_send_updates = false;
}

void broken_wall::tick() {
    if (life_ticks == 0) {
        if (drop_item) {
            world->addEntity(new game_item(world, world->getMap().getTile(tx,ty), type));
        }
        destroy();
    }
    --life_ticks;
}

byte_array broken_wall::toByteArray() {
    byte_array ba;

    ba.writeChar(tx);
    ba.writeChar(ty);
    ba.writeChar(zone);

    return ba;
}
