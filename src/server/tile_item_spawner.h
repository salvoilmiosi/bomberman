#ifndef __TILE_ITEM_H__
#define __TILE_ITEM_H__

#include "game_map.h"
#include "ent_item.h"

class tile_item_spawner : public tile_entity {
private:
    item_type item;

public:
    tile_item_spawner(tile &t_tile, game_map &g_map, item_type item) : tile_entity(SPECIAL_ITEM_SPAWNER, t_tile, g_map), item(item) {
        setData(item);
    }

    void tick() {
        if (item != ITEM_NONE) {
            game_world &world = g_map.getWorld();
            world.addEntity(std::make_shared<game_item>(world, t_tile, item));
        }
        item = ITEM_NONE;
    }
};

#endif // __TILE_ITEM_H__
