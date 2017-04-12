#include "game_map.h"

void game_map::readFromByteArray(byte_array &packet) {
	packet.readChar(); // num_breakables

	int8_t num_items = packet.readChar();
	while (num_items > 0) {
		packet.readChar();
		packet.readChar();
		--num_items;
	}

	int8_t num_players = packet.readChar();
	while (num_players > 0) {
		packet.readChar();
		packet.readChar();
		--num_players;
	}

	short w = packet.readShort();
	short h = packet.readShort();
	zone = static_cast<map_zone>(packet.readChar());

	if (w * h > width * height) {
		clear();
		tiles.resize(w * h);
	}

	width = w;
	height = h;

	for (auto it : specials) {
		if (it.second) {
			it.second->used = false;
		}
	}

	for (int y=0; y<h; ++y) {
		for (int x=0; x<w; ++x) {
			uint8_t type = packet.readChar();
			uint16_t data = packet.readShort();
			packet.readChar(); // randomize

			try {
				tile &t = getTile(x, y);
				t.type = static_cast<tile_type>(type);
				t.data = data;
				if (t.type == TILE_SPECIAL) {
					uint8_t ent_type = tile_entity::getSpecialType(data);
					auto it = specials.find(getTileID(t));
					special_ptr ent;
					if (it == specials.end()) {
						ent = tile_entity::newTileEntity(&t);
						if (ent) {
							ent->used = true;
							specials.insert({getTileID(t), ent});
						}
					} else {
						ent = it->second;
						if (!ent || ent->type != ent_type) {
							ent = tile_entity::newTileEntity(&t);
						}
						if (ent) {
							ent->used = true;
							it->second = ent;
						}
					}
				}
			} catch (std::out_of_range) {
			}
		}
	}

	auto it = specials.begin();
	while (it != specials.end()) {
		auto &ent = it->second;
		if (ent && ent->used) {
			++it;
		} else {
			it = specials.erase(it);
		}
	}
}