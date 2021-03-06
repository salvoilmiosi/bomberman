#ifndef __GAME_MAP_H__
#define __GAME_MAP_H__

#include "packet_io.h"

#include <SDL2/SDL.h>
#include <map>
#include <vector>
#include <memory>

static const int TILE_SIZE = 48;
static const float SERVER_TILE_SIZE = 100.f;

enum tile_type {
	TILE_FLOOR,
	TILE_SPAWN,
	TILE_WALL,
	TILE_BREAKABLE,
	TILE_SPECIAL,
};

enum map_zone {
	ZONE_RANDOM,
	ZONE_NORMAL,
	ZONE_WESTERN,
	ZONE_BOMB,
	ZONE_JUMP,
	ZONE_BELT,
	ZONE_DUEL,
	ZONE_POWER,
	ZONE_SPEED,
};

static const std::map<std::string, map_zone> zone_by_name = {
	{"random", ZONE_RANDOM},
	{"normal", ZONE_NORMAL},
	{"western", ZONE_WESTERN},
	{"bomb", ZONE_BOMB},
	{"jump", ZONE_JUMP},
	{"belt", ZONE_BELT},
	{"duel", ZONE_DUEL},
	{"power", ZONE_POWER},
	{"speed", ZONE_SPEED},
};

enum special_type {
	SPECIAL_NONE,
	SPECIAL_TRAMPOLINE,
	SPECIAL_BELT,
	SPECIAL_ITEM_SPAWNER,
};

struct tile {
	tile_type type;
	uint16_t data;

	tile() {
		type = TILE_FLOOR;
		data = 0;
	}
};

typedef std::shared_ptr<class tile_entity> special_ptr;

class tile_entity {
private:
	special_type type;

	tile *t_tile;

	bool used = false;

	friend class game_map;

public:
	tile_entity(tile *t_tile);

	virtual ~tile_entity() {}

	virtual void tick() = 0;

	virtual SDL_Rect getSrcRect() = 0;

	static special_ptr newTileEntity(tile *t_tile);

	static special_type getSpecialType(uint16_t data) {
		return static_cast<special_type>((data & 0xe000) >> 13);
	}

protected:
	uint16_t getData() {
		return t_tile->data & 0x1fff;
	}
};

class game_map {
private:
	std::vector<tile> tiles;

	int width = 0;
	int height = 0;

	map_zone zone;

	std::map<int, special_ptr> specials;

public:
	game_map();
	virtual ~game_map();

public:
	static int getZoneByName(const std::string &name);

	void clear();

	tile &getTile(int x, int y);

	int getTileX(const tile &t);
	int getTileY(const tile &t);

	int getTileID(const tile &t);

	void tick();

	void render(SDL_Renderer *renderer);

	static SDL_Texture *getTileset(map_zone zone);

	void readFromByteArray(byte_array &packet);

	int left();
	int top();
};

#endif // __GAME_MAP_H__
