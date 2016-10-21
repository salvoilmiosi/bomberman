#include "game_world.h"

#include <cstring>

#include "player.h"
#include "ent_bomb.h"
#include "ent_broken_wall.h"
#include "ent_item.h"

#include "tile_trampoline.h"

entity *entity::newObjFromByteArray(game_world *world, uint16_t id, uint8_t type, byte_array &ba) {
    switch(type) {
    case TYPE_PLAYER:
        return new player(world, id, ba);
    case TYPE_BOMB:
        return new bomb(world, id, ba);
    case TYPE_EXPLOSION:
        return new explosion(world, id, ba);
    case TYPE_BROKEN_WALL:
        return new broken_wall(world, id, ba);
    case TYPE_ITEM:
        return new game_item(world, id, ba);
    default:
        return nullptr;
    }
}

tile_entity *tile_entity::newTileEntity(tile *t_tile) {
    uint8_t type = (t_tile->data & 0xe0) >> 5;

    switch (type) {
    case SPECIAL_TRAMPOLINE:
        return new tile_trampoline(t_tile);
    default:
        return nullptr;
    }
}

game_world::game_world() {}

game_world::~game_world() {
    clear();
}

void game_world::clear() {
	for (auto it : entities) {
		if (it.second) delete it.second;
	}
	entities.clear();
	g_map.clear();
}

void game_world::addEntity(entity *ent) {
    entities[ent->getID()] = ent;
}

void game_world::removeEntity(entity *ent) {
    if (ent == nullptr) return;

    entities.erase(ent->getID());
    delete ent;
}

void game_world::render(SDL_Renderer *renderer) {
    g_map.render(renderer);

    for (auto it : entities) {
        it.second->render(renderer);
    }
}

void game_world::tick() {
    g_map.tick();

    for (auto it : entities) {
        if (it.second) {
            it.second->tick();
        }
    }
}

entity *game_world::findID(uint16_t id) {
    auto it = entities.find(id);
    if (it != entities.end()) {
        return it->second;
    } else {
        return nullptr;
    }
}

SDL_Rect game_world::tileRect(int x, int y) {
    SDL_Rect rect = {x * TILE_SIZE + g_map.left(), y * TILE_SIZE + g_map.top(), TILE_SIZE, TILE_SIZE};
    return rect;
}
