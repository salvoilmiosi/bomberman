#include "game_world.h"

#include <cstring>

#include "player.h"
#include "ent_bomb.h"
#include "ent_broken_wall.h"
#include "ent_item.h"

#include "tile_trampoline.h"
#include "tile_belt.h"

entity_ptr entity::newObjFromByteArray(game_world *world, uint16_t id, entity_type type, byte_array &ba) {
	switch(type) {
	case TYPE_PLAYER:
		return std::make_shared<player>(world, id, ba);
	case TYPE_BOMB:
		return std::make_shared<bomb>(world, id, ba);
	case TYPE_EXPLOSION:
		return std::make_shared<explosion>(world, id, ba);
	case TYPE_BROKEN_WALL:
		return std::make_shared<broken_wall>(world, id, ba);
	case TYPE_ITEM:
		return std::make_shared<game_item>(world, id, ba);
	default:
		return nullptr;
	}
}

tile_entity *tile_entity::newTileEntity(tile *t_tile) {
	special_type type = getSpecialType(t_tile->data);

	switch (type) {
	case SPECIAL_TRAMPOLINE:
		return new tile_trampoline(t_tile);
	case SPECIAL_BELT:
		return new tile_belt(t_tile);
	default:
		return nullptr;
	}
}

game_world::game_world() {}

game_world::~game_world() {
	clear();
}

void game_world::clear() {
	s_entities.clear();
	g_map.clear();
}

void game_world::addEntity(entity_ptr ent) {
	ents_to_add.push_back(ent);
}

void game_world::removeEntity(entity_ptr ent) {
	if (!ent) return;

	ent->flagRemoved();
}

void game_world::render(SDL_Renderer *renderer) {
	g_map.render(renderer);

	for (auto &it : s_entities) {
		it.second->render(renderer);
	}
}

void game_world::tick() {
	g_map.tick();

	auto it = s_entities.begin();
	while(it != s_entities.end()) {
		if (it->second && !it->second->isRemoved()) {
			it->second->tick();
			++it;
		} else {
			it = s_entities.erase(it);
		}
	}

	while(!ents_to_add.empty()) {
		auto &ent = ents_to_add.front();
		s_entities[ent->getID()] = ent;
		ents_to_add.pop_front();
	}
}

entity_ptr game_world::findID(uint16_t id) {
	try {
		return s_entities.at(id);
	} catch (std::out_of_range) {
		return nullptr;
	}
}

SDL_Rect game_world::tileRect(int x, int y) {
	SDL_Rect rect = {x * TILE_SIZE + g_map.left(), y * TILE_SIZE + g_map.top(), TILE_SIZE, TILE_SIZE};
	return rect;
}
