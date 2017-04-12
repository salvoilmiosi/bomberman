#include "game_map.h"

#include <string>
#include <algorithm>
#include <fstream>

#include "random.h"

#include "tile_trampoline.h"
#include "tile_belt.h"
#include "tile_item_spawner.h"

game_map::game_map(game_world &world) : world(&world) {}

game_map::~game_map() {
	clear();
}

const char *game_map::getZoneName(map_zone zone) {
	try {
		return zone_by_id.at(zone);
	} catch (std::out_of_range) {
		return "unknown";
	}
}

void game_map::clear() {
	tiles.clear();
	specials.clear();
	width = 0;
	height = 0;
}

void game_map::tick() {
	for (auto ent : specials) {
		if (ent.second) ent.second->tick();
	}
}

bool game_map::loadMap(map_zone m_zone) {
	if (m_zone == ZONE_RANDOM) {
		m_zone = static_cast<map_zone>(rand_num(8) + 1);
	}

	zone = m_zone;

	std::string filename = "zones/";
	filename += getZoneName(m_zone);
	filename += ".zml";

	std::ifstream ifs(filename, std::ios::binary | std::ios::in | std::ios::ate);

	if (ifs.fail()) {
		return false;
	}

	size_t len = ifs.tellg();
	ifs.seekg(0, std::ios::beg);

	std::vector<char> data(len, '\0');

	ifs.read(&data[0], len);
	ifs.close();

	byte_array ba(reinterpret_cast<const uint8_t *>(data.data()), len);
	return readFromByteArray(ba);
}

void game_map::randomize(size_t num_players) {
	rand_shuffle(spawn_pts);

	while (spawn_pts.size() > num_players) {
		spawn_pts.pop_back();
	}

	switch (zone) {
	case ZONE_WESTERN:
	case ZONE_DUEL:
		for (int y = 5; y < height-5; ++y) {
			for (int x = 6; x < width-6; ++x) {
				tile &t = getTile(x, y);
				if (t.type == TILE_FLOOR) {
					t.type = TILE_SPAWN;
				}
			}
		}
		break;
	default:
		for (const point &pt : spawn_pts) {
			for (int y = pt.y - 1; y <= pt.y + 1; ++y) {
				for (int x = pt.x - 1; x <= pt.x + 1; ++x) {
					tile &t = getTile(x, y);
					if (t.type == TILE_FLOOR) {
						t.type = TILE_SPAWN;
					}
				}
			}
		}
		break;
	}

	std::vector<tile *> floor_tiles;
	for (tile &t : tiles) {
		if (t.type == TILE_FLOOR && t.randomize) {
			floor_tiles.push_back(&t);
		}
	}

	rand_shuffle(floor_tiles);

	std::vector<tile *> breakables;
	for (int i=0; i<num_breakables; ++i) {
		if (floor_tiles.empty()) {
			break;
		}
		tile *t = floor_tiles.back();
		t->type = TILE_BREAKABLE;
		breakables.push_back(t);
		floor_tiles.pop_back();
	}

	rand_shuffle(breakables);

	for (auto item_pair : num_items) {
		for (int i=0; i<item_pair.second; ++i) {
			tile *t = breakables.back();
			if (t) {
				t->data = item_pair.first;
			}
			breakables.pop_back();
			if (breakables.empty()) {
				goto end_items;
			}
		}
	}
	end_items:

	switch (zone) {
	case ZONE_JUMP:
	{
		int num_trampolines = 12 + rand_num(4);
		for (int i=0; i<num_trampolines; ++i) {
			tile *t = floor_tiles.back();
			addSpecial(std::make_shared<tile_trampoline>(*t, *this));
			floor_tiles.pop_back();
		}
		break;
	}
	case ZONE_BELT:
		for (int x = 4; x <= width - 6; ++x) {
			addSpecial(std::make_shared<tile_belt>(getTile(x, 3), *this, DIR_RIGHT));
		}
		for (int x = 5; x <= width - 5; ++x) {
			addSpecial(std::make_shared<tile_belt>(getTile(x, height - 4), *this, DIR_LEFT));
		}
		for (int y = 4; y <= height - 4; ++y) {
			addSpecial(std::make_shared<tile_belt>(getTile(4, y), *this, DIR_UP));
		}
		for (int y = 3; y <= height - 5; ++y) {
			addSpecial(std::make_shared<tile_belt>(getTile(width - 5, y), *this, DIR_DOWN));
		}
		break;
	default:
		break;
	}
}

tile &game_map::getTile(int x, int y) {
	if (x < 0 || x >= width) {
		throw std::out_of_range("");
	}
	if (y < 0 || y >= height) {
		throw std::out_of_range("");
	}
	return tiles[width * y + x];
}

int game_map::getTileX(const tile &t) const {
	return (&t-&tiles.front()) % width;
}

int game_map::getTileY(const tile &t) const {
	return (&t-&tiles.front()) / width;
}

special_ptr game_map::getSpecial(const tile &t) const {
	if (t.type != TILE_SPECIAL) return nullptr;

	try {
		return specials.at(&t);
	} catch (std::out_of_range) {
		return nullptr;
	}
}

point game_map::getSpawnPt(unsigned int num) {
	if (num >= spawn_pts.size()) {
		throw std::out_of_range(nullptr);
	}
	return spawn_pts[num];
}

void game_map::writeToPacket(byte_array &packet) {
	packet.writeChar(num_breakables);
	packet.writeChar(num_items.size());
	for (auto &p : num_items) {
		packet.writeChar(static_cast<uint8_t>(p.first));
		packet.writeChar(p.second);
	}
	packet.writeChar(spawn_pts.size());
	for (auto &p : spawn_pts) {
		packet.writeChar(p.x);
		packet.writeChar(p.y);
	}

	packet.writeShort(width);
	packet.writeShort(height);
	packet.writeChar(static_cast<uint8_t>(zone));
	for (auto &t : tiles) {
		switch (t.type) {
		case TILE_BREAKABLE:
			packet.writeChar(TILE_BREAKABLE);
			packet.writeShort(0);
			packet.writeChar(t.randomize);
			break;
		default:
			packet.writeChar(t.type);
			packet.writeShort(t.data);
			packet.writeChar(t.randomize);
			break;
		}
	}
}

bool game_map::readFromByteArray(byte_array &packet) {
	num_breakables = packet.readChar();

	num_items.clear();
	int8_t num_items_n = packet.readChar();
	while (num_items_n > 0) {
		uint8_t item_t = packet.readChar();
		uint8_t item_n = packet.readChar();
		num_items[static_cast<item_type>(item_t)] = item_n;
		--num_items_n;
	}

	spawn_pts.clear();
	int8_t num_players = packet.readChar();
	while (num_players > 0) {
		uint8_t x = packet.readChar();
		uint8_t y = packet.readChar();
		spawn_pts.push_back({x, y});
		--num_players;
	}

	width = packet.readShort();
	height = packet.readShort();
	zone = static_cast<map_zone>(packet.readChar());

	tiles.clear();
	tiles.resize(width * height);

	specials.clear();

	for (tile &t : tiles) {
		uint8_t type = packet.readChar();
		uint16_t data = packet.readShort();
		bool randomize = packet.readChar();

		t.type = static_cast<tile_type>(type);
		t.data = data;
		t.randomize = randomize;
		if (t.type == TILE_SPECIAL) {
			addSpecial(newTileEntity(t));
		}
	}

	return true;
}

special_ptr game_map::newTileEntity(tile &t) {
	special_type type = tile_entity::getSpecialType(t.data);

	switch (type) {
	case SPECIAL_ITEM_SPAWNER:
		return std::make_shared<tile_item_spawner>(t, *this);
	default:
		return nullptr;
	}
}