#include "game_map.h"

#include <cstring>
#include <cstdio>

#include "main.h"
#include "resources.h"

tile_entity::tile_entity(tile *t_tile) : t_tile(t_tile) {
	type = getSpecialType(t_tile->data);
}

game_map::game_map() {
}

game_map::~game_map() {
	clear();
}

void game_map::clear() {
	specials.clear();
	tiles.clear();
	width = 0;
	height = 0;
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

int game_map::getTileX(const tile &t) {
	return (&t - &tiles.front()) % width;
}

int game_map::getTileY(const tile &t) {
	return (&t - &tiles.front()) / width;
}

int game_map::getTileID(const tile &t) {
	return &t - &tiles.front();
}

void game_map::tick() {
	for (auto &it : specials) {
		if (it.second) it.second->tick();
	}
}

void game_map::render(SDL_Renderer *renderer) {
	SDL_Rect src_rect = {0, 0, 16, 16};
	SDL_Rect dst_rect = {0, 0, TILE_SIZE, TILE_SIZE};

	int frame = 0;

	for (int y=0; y<height; ++y) {
		for (int x=0; x<width; ++x) {
			const tile &t = getTile(x, y);
			bool t_up_exists = y >= 0;
			const tile &t_up = t_up_exists ? t : getTile(x, y-1);

			switch (t.type) {
			case TILE_SPECIAL:
				try {
					auto it = specials.at(getTileID(t));
					if (it) {
						src_rect = it->getSrcRect();
						break;
					}
				} catch (std::out_of_range) {}
				// treat as TILE_FLOOR if tile entity not found
			case TILE_SPAWN:
			case TILE_FLOOR:
				switch (zone) {
				case ZONE_RANDOM:
				case ZONE_NORMAL:
				case ZONE_SPEED: // TODO create speed zone tileset
					if (t_up_exists && (t_up.type == TILE_WALL || t_up.type == TILE_BREAKABLE)) {
						src_rect = TILE(4, 0);
					} else {
						src_rect = TILE(3, 0);
					}
					break;
				case ZONE_WESTERN:
					if (t_up_exists && t_up.type == TILE_WALL && t_up.data == 0) {
						src_rect = TILE(1, 3);
					} else if (t_up_exists && t_up.type == TILE_BREAKABLE) {
						src_rect = TILE(2, 3);
					} else {
						src_rect = TILE(3, 2);
					}
					break;
				case ZONE_BOMB:
					if (t_up_exists && t_up.type == TILE_WALL) {
						src_rect = TILE(4, 1);
					} else if (t_up_exists && t_up.type == TILE_BREAKABLE) {
						src_rect = TILE(5, 1);
					} else {
						src_rect = TILE(3, 1);
					}
					break;
				case ZONE_JUMP:
					src_rect = TILE(1, 2);
					break;
				case ZONE_BELT:
					if (t_up_exists && t_up.type == TILE_WALL && t_up.data == 0) {
						src_rect = TILE(2, 2);
					} else {
						src_rect = TILE(7, 0);
					}
					break;
				case ZONE_DUEL:
					if (t_up_exists && t_up.type == TILE_WALL && t_up.data == 0) {
						src_rect = TILE(2, 2);
					} else {
						src_rect = TILE(3, 2);
					}
					break;
				case ZONE_POWER:
					if (t_up_exists && t_up.type == TILE_WALL && t_up.data == 0) {
						src_rect = TILE(3, 2);
					} else {
						src_rect = TILE(2, 2);
					}
					break;
				}
				break;
			case TILE_WALL:
				switch (zone) {
				case ZONE_RANDOM:
				case ZONE_NORMAL:
				case ZONE_SPEED: // TODO create speed zone tileset
					src_rect = TILE(t.data == 1 ? 0 : 1, 0);
					break;
				case ZONE_WESTERN:
					src_rect = TILE((t.data & 0x3f) % 5, (t.data & 0x3f) / 5);
					break;
				case ZONE_BOMB:
					src_rect = TILE((t.data & 0x3f) % 8, (t.data & 0x3f) / 8);
					break;
				case ZONE_JUMP:
					src_rect = TILE((t.data & 0x3f) % 7, (t.data & 0x3f) / 7);
					break;
				case ZONE_BELT:
					src_rect = TILE((t.data & 0x3f) % 3, (t.data & 0x3f) / 3);
					break;
				case ZONE_DUEL:
					src_rect = TILE((t.data & 0x3f) % 6, (t.data & 0x3f) / 6);
					break;
				case ZONE_POWER:
					src_rect = TILE((t.data & 0x3f) % 4, (t.data & 0x3f) / 4);
					break;
				}
				break;
			case TILE_BREAKABLE:
				switch (zone) {
				case ZONE_RANDOM:
				case ZONE_NORMAL:
				case ZONE_SPEED: // TODO create speed zone tileset
					src_rect = TILE(2, 0);
					break;
				case ZONE_WESTERN:
					if (t_up_exists && (t_up.type == TILE_BREAKABLE || (t_up.type == TILE_WALL && t_up.data == 0))) {
						src_rect = TILE(4, 2);
					} else {
						src_rect = TILE(2, 2);
					}
					break;
				case ZONE_BOMB:
					frame = (SDL_GetTicks() * 4 / 2000) % 4;
					if (t_up_exists && t_up.type == TILE_WALL) {
						src_rect = TILE(frame + 4, 2);
					} else {
						src_rect = TILE(frame, 2);
					}
					break;
				case ZONE_JUMP:
				{
					static const SDL_Rect jump_breakables[] = {
						TILE(2, 2), TILE(3, 2), TILE(4, 2), TILE(5, 2), TILE(6, 2),
						TILE(2, 3), TILE(3, 3), TILE(4, 3), TILE(5, 3),
					};

					frame = (SDL_GetTicks() * 9 / 2000) % 9;
					src_rect = jump_breakables[frame];
					break;
				}
				case ZONE_BELT:
				{
					static const SDL_Rect belt_breakables[] = {
						TILE(3, 0), TILE(4, 0), TILE(5, 0), TILE(6, 0),
						TILE(3, 1), TILE(4, 1), TILE(5, 1), TILE(6, 1),
						TILE(3, 2), TILE(4, 2), TILE(5, 2), TILE(6, 2),
					};

					frame = (SDL_GetTicks() * 12 / 2000) % 12;
					src_rect = belt_breakables[frame];
					break;
				}
				case ZONE_DUEL:
					src_rect = TILE(3, 1);
					break;
				case ZONE_POWER:
					break; // There are no power zone breakables
				}
				break;
			}
			dst_rect.x = TILE_SIZE * x + left();
			dst_rect.y = TILE_SIZE * y + top();

			SDL_RendererFlip flip = SDL_FLIP_NONE;
			if (t.data & 0x80) {
				flip = (SDL_RendererFlip) (flip | SDL_FLIP_HORIZONTAL);
			}
			if (t.data & 0x40) {
				flip = (SDL_RendererFlip) (flip | SDL_FLIP_VERTICAL);
			}

			SDL_RenderCopyEx(renderer, getTileset(zone), &src_rect, &dst_rect, 0, 0, flip);
		}
	}
}

int game_map::getZoneByName(const std::string &name) {
	try {
		return zone_by_name.at(name);
	} catch (std::out_of_range) {
		return -1;
	}
}

SDL_Texture *game_map::getTileset(map_zone zone) {
	static std::map<map_zone, SDL_Texture *> tilesets = {
		{ZONE_NORMAL, tileset_1_texture},
		{ZONE_WESTERN, tileset_2_texture},
		{ZONE_BOMB, tileset_3_texture},
		{ZONE_JUMP, tileset_4_texture},
		{ZONE_BELT, tileset_5_texture},
		{ZONE_DUEL, tileset_6_texture},
		{ZONE_POWER, tileset_7_texture},
	};

	try {
		return tilesets.at(zone);
	} catch (std::out_of_range) {
		return tilesets.at(ZONE_NORMAL);
	} 
}

int game_map::left() {
	return (WINDOW_WIDTH - width * TILE_SIZE) / 2;
}

int game_map::top() {
	return (WINDOW_HEIGHT - height * TILE_SIZE) / 2;
}
