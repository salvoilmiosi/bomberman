#ifndef __GAME_WORLD_H__
#define __GAME_WORLD_H__

#include <SDL2/SDL.h>

#include <set>
#include <deque>
#include <memory>
#include <vector>

#include "game_map.h"
#include "game_server.h"

enum entity_type {
	TYPE_NONE,
	TYPE_PLAYER,
	TYPE_BOMB,
	TYPE_EXPLOSION,
	TYPE_BROKEN_WALL,
	TYPE_ITEM
};

enum item_type {
	ITEM_NONE,
	ITEM_BOMB,
	ITEM_FIRE,
	ITEM_ROLLERBLADE,
	ITEM_KICK,
	ITEM_PUNCH,
	ITEM_SKULL,
	ITEM_FULL_FIRE,
	ITEM_REDBOMB,
	ITEM_REMOCON
};

enum disease_effect {
	SKULL_NONE,
	SKULL_RAPID_PACE,
	SKULL_SLOW_PACE,
	SKULL_DIARRHEA,
	SKULL_LOW_POWER,
	SKULL_CONSTIPATION
};

enum direction {
	DIR_UP,
	DIR_DOWN,
	DIR_LEFT,
	DIR_RIGHT,
};

static const int COUNTDOWN_SECONDS = 4;

static const uint8_t WALK_IGNORE_BOMBS = (1 << 0);
static const uint8_t WALK_BLOCK_PLAYERS = (1 << 1);
static const uint8_t WALK_BLOCK_ITEMS = (1 << 2);

class entity {
private:
	const entity_type type;

	uint16_t id;

	bool destroyed;

protected:
	bool do_send_updates;

	class game_world *world;

public:
	entity(class game_world *world, const entity_type type) : type(type), world(world) {
		static uint16_t max_id = 1;
		id = max_id++;
		destroyed = false;
		do_send_updates = true;
	}

	virtual ~entity() {};

	virtual void tick() = 0;

	uint16_t getID() {
		return id;
	}

	const entity_type getType() {
		return type;
	}

	virtual const uint8_t getTileX() = 0;
	virtual const uint8_t getTileY() = 0;

	bool isNotDestroyed() {
		return !destroyed;
	}

	void destroy() {
		destroyed = true;
	}

	void writeToPacket(packet_ext &packet) {
		packet.writeShort(getID());
		packet.writeChar(getType());
		packet.writeByteArray(toByteArray());
	}

	bool doSendUpdates() {
		return do_send_updates;
	}

	virtual byte_array toByteArray() = 0;
};

typedef std::shared_ptr<entity> entity_ptr;

class game_world {
private:
	game_server server;

	std::set <entity_ptr> s_entities;
	std::deque <entity_ptr> ents_to_add;

	game_map g_map;

	bool round_started = false;
	int round_num = 0;

	int ticks_to_start = 0;

	int num_users;

	map_zone selected_map_zone = ZONE_RANDOM;

public:
	game_world();
	virtual ~game_world();

public:
	void clear();

	int startServer(uint16_t port);

	void addBots(int num_bots) {
		server.addBots(num_bots);
	}

	void addEntity(entity_ptr ent);
	void removeEntity(entity_ptr ent);

	void tick();

	void writeEntitiesToPacket(packet_ext &packet, bool is_add = false);

	void writeMapToPacket(packet_ext &packet) {
		g_map.writeToPacket(packet);
	}

	void sendKillMessage(player *killer, player *p);

	void restartGame();

	bool startRound(int num_users);

	bool roundStarted() {
		return round_started;
	}

	void playWave(uint8_t sound_id) {
		server.sendSoundPacket(sound_id);
	}

	const game_map &getMap() {
		return g_map;
	}

	void setZone(map_zone zone) {
		selected_map_zone = zone;
	}

	std::vector<entity_ptr> findEntities(int tx, int ty, entity_type type = TYPE_NONE);

	std::vector<std::shared_ptr<class ent_movable>> findMovables(float fx, float fy, entity_type type = TYPE_NONE);

	bool isWalkable(float fx, float fy, uint8_t flags = 0);

	uint8_t getStartPickups();
	std::map<item_type, uint8_t> getStartItems();

private:
	void countdownEnd();
};

#endif // __GAME_WORLD_H__
