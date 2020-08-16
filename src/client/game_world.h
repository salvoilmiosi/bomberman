#ifndef __GAME_WORLD_H__
#define __GAME_WORLD_H__

#include "game_map.h"

#include <map>
#include <deque>
#include <memory>

enum entity_type {
	TYPE_NONE,
	TYPE_PLAYER,
	TYPE_BOMB,
	TYPE_EXPLOSION,
	TYPE_BROKEN_WALL,
	TYPE_ITEM
};

enum direction {
	DIR_UP,
	DIR_DOWN,
	DIR_LEFT,
	DIR_RIGHT,
};

typedef std::shared_ptr<class entity> entity_ptr;

class entity {
private:
	const entity_type ent_type;
	const uint16_t id;

	friend class game_world;

	bool removed = false;

protected:
	class game_world *world;

public:
	entity(class game_world *world, const entity_type type, uint16_t id) : ent_type(type), id(id), world(world) {}

	virtual ~entity() {};

	virtual void tick() = 0;

	virtual void render(SDL_Renderer *renderer) = 0;

	static entity_ptr newObjFromByteArray(class game_world *world, uint16_t id, entity_type type, byte_array &ba);

	uint16_t getID() {
		return id;
	}

	char getType() {
		return ent_type;
	}

	void flagRemoved() {
		removed = true;
	}

	bool isRemoved() {
		return removed;
	}

	virtual void readFromByteArray(byte_array &ba) = 0;
};

class game_world {
private:
	std::map <uint16_t, entity_ptr, std::greater<uint16_t>> s_entities;
	std::deque<entity_ptr> ents_to_add;

	game_map g_map;

	bool toBeCleared = false;

public:
	game_world();
	virtual ~game_world();

public:
	void clear();
	void flagClear() {
		toBeCleared = true;
	}

	void addEntity(entity_ptr ent);

	void render(SDL_Renderer *renderer);

	void tick();

	entity_ptr findID(uint16_t id);

	void handleMapPacket(byte_array &packet) {
		g_map.readFromByteArray(packet);
	}

	int mapLeft() {
		return g_map.left();
	}

	int mapTop() {
		return g_map.top();
	}

	SDL_Rect tileRect(int x, int y);
};


#endif // __GAME_WORLD_H__
