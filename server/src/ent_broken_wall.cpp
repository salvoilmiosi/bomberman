#include "ent_broken_wall.h"
#include "ent_item.h"

broken_wall::broken_wall(game_world &world, tile &t, item_type type) : entity(world, TYPE_BROKEN_WALL) {
    tx = world.getMap().getTileX(t);
    ty = world.getMap().getTileY(t);
    zone = world.getMap().getZone();

    life_ticks = BROKEN_WALL_LIFE;

    if (t.type == TILE_BREAKABLE) {
        item = static_cast<item_type>(t.data);
    }
    if (item == ITEM_NONE) {
        item = type;
    }

    do_send_updates = false;
}

void broken_wall::tick() {
    if (life_ticks == 0) {
        if (item != ITEM_NONE) {
            world.addEntity(std::make_shared<game_item>(world, world.getMap().getTile(tx,ty), item));
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
