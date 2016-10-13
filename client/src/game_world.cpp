#include "game_world.h"

#include <cstring>

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
