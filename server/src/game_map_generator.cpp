#include "game_map.h"

#include "random.h"

#include "game_world.h"

#include "tile_item_spawner.h"


void game_map::createMap(int w, int h, map_zone m_zone) {
	clear();

	if (m_zone == ZONE_RANDOM) {
		m_zone = static_cast<map_zone>(rand_num(8) + 1);
	}

	width = w;
	height = h;
	zone = m_zone;

	tiles.resize(w * h);

	switch (zone) {
	case ZONE_WESTERN:
	case ZONE_DUEL:
		spawn_pts = {{6, 5}, {width - 7, 5}, {6, height - 6}, {width - 7, height - 6}};
		break;
	default:
		spawn_pts = {{2, 1}, {width - 3, 1}, {2, height - 2}, {width - 3, height - 2}};
		break;
	}

	for (int y=0; y<height; ++y) {
		for (int x=0; x<width; ++x) {
			tile &t = getTile(x, y);
			if (x<=1 || x>=width-2 || y==0 || y==height-1) {
				t.type = TILE_WALL;
				switch (zone) {
				case ZONE_RANDOM:
				case ZONE_SPEED: // TODO create speed zone tileset
				case ZONE_NORMAL:
					if (x == 0 || x == width-1) {
						t.data = 1;
					} else {
						t.data = 0;
					}
					break;
				case ZONE_WESTERN:
					if (x == 0 || x == width - 1) {
						if (y == 0) {
							t.data = 1;
						} else if (y == height - 2) {
							t.data = 2;
						} else if (y == height - 1) {
							t.data = 3;
						} else {
							t.data = 4;
						}
					} else if (x == 1 || x == width - 2) {
						if (y == 0) {
							t.data = 5;
						} else if (y == height - 2) {
							t.data = 6;
						} else if (y == height - 1) {
							t.data = 7;
						} else if (y == 1) {
							t.data = 8;
						} else {
							t.data = 9;
						}
					} else if (y == 0) {
						t.data = 10;
					} else if (y == height - 1) {
						t.data = 11;
					}
					if (x >= width - 2) {
						t.data |= 0x80;
					}
					break;
				case ZONE_BOMB:
					if (x == 0 || x == width - 1) {
						if (y == 0) {
							t.data = 1;
						} else if (y == height - 1) {
							t.data = 3;
						} else {
							t.data = 2;
						}
					} else if (x == 1 || x == width - 2) {
						if (y == 0) {
							t.data = 4;
						} else if (y == height - 1) {
							t.data = 6;
						} else {
							t.data = 5;
						}
					} else if (y == 0) {
						t.data = 9;
					} else if (y == height - 1) {
						t.data = 8;
					}
					if (x >= width - 2) {
						t.data |= 0x80;
					}
					break;
				case ZONE_JUMP:
					if (x == 0) {
						static const uint8_t sky_left[] = {2, 3, 4, 5, 6, 9, 10, 11, 3, 4, 13, 11, 3};
						t.data = sky_left[y % 13];
					} else if (x == width - 1) {
						static const uint8_t sky_right[] = {12, 9, 10, 11, 3, 6, 9, 4, 12, 12, 13, 11, 2};
						t.data = sky_right[y % 13];
					} else if (x == 1 || x == width - 2) {
						if (y == 0) {
							t.data = 8;
						} else if (y == 1) {
							t.data = 7;
						} else if (y == height - 1) {
							t.data = 21;
						} else {
							t.data = 14;
						}
					} else if (y == 0) {
						t.data = 1;
					} else if (y == height - 1) {
						t.data = 22;
					}
					if (x >= width - 2) {
						t.data |= 0x80;
					}
					break;
				case ZONE_BELT:
					if (x == 0 || x  == width - 1) {
						if (y == 0 || y == height - 1) {
							t.data = 5;
						} else if ((y % 2) == 1) {
							t.data = 3;
						} else {
							t.data = 6;
						}
					} else if (x == 1 || x == width - 2) {
						if (y == 0 || y == height - 1) {
							t.data = 1;
						} else if ((y % 2) == 1) {
							t.data = 4;
						} else {
							t.data = 7;
						}
					} else if (y == 0 || y == height - 1) {
						t.data = 2;
						if ((x > (width / 2) || x == (width / 2 - 1))) {
							t.data |= 0x80;
						}
					}
					if (x >= width - 2) {
						t.data |= 0x80;
					}
					break;
				case ZONE_DUEL:
					if (x == 0 || x == width - 1) {
						if (y == 0) {
							t.data = 8;
						} else if (y == height - 1) {
							t.data = 3;
						} else {
							t.data = (y % 2 == 1) ? 6 : 12;
						}
					} else if (x == 1 || x == width - 2) {
						if (y == 0) {
							t.data = 1;
						} else {
							t.data = (y % 2 == 1) ? 7 : 13;
						}
					} else if (y == 0) {
						if (x <= 4 || x >= 11) {
							t.data = 2;
						} else {
							t.data = x + 13;
						}
					}
					if (y == height - 1 && x > 0) {
						t.data = (x % 2 == 1) ? 4 : 5;
					} else if (x >= width - 2) {
						t.data |= 0x80;
					}
					break;
				case ZONE_POWER:
					if (x == 0 || x == width - 1) {
						if (y <= 1 || y >= height - 2) {
							t.data = 4;
						} else {
							t.data = (y % 2) == 0 ? 12 : 8;
						}
					} else if (x == 1 || x == width - 2) {
						if (y == 0 || y == height - 1) {
							t.data = 1;
						} else if (y == 1 || y == height - 2) {
							t.data = 5;
						} else {
							t.data = (y % 2) == 0 ? 13 : 9;
						}
					} else if (y == 0 || y == height - 1) {
						if (x == 2 || x == width - 3) {
							t.data = 2;
						} else {
							t.data = (x % 2) == 0 ? 3 : 7;
						}
					}
					if (x >= width - 3) {
						t.data |= 0x80;
					}
					if (y >= height - 2) {
						t.data |= 0x40;
					}
					break;
				}
			} else if (x % 2 == 1 && y % 2 == 0) {
				t.type = TILE_WALL;
			} else if (t.type == TILE_FLOOR) {
				t.randomize = true;
			}
		}
	}

	static const std::map<map_zone, int> breakables_per_zone = {
		{ZONE_NORMAL, 80}, {ZONE_WESTERN, 65}, {ZONE_BOMB, 60}, {ZONE_JUMP, 55}, {ZONE_BELT, 55}, {ZONE_SPEED, 80}
	};

	try {
		num_breakables = breakables_per_zone.at(zone);
	} catch (std::out_of_range) {
		num_breakables = 0;
	}

	static const std::map<map_zone, item_map> item_numbers_per_zone = {
		{ZONE_NORMAL, {{ITEM_BOMB, 7}, {ITEM_FIRE, 5}, {ITEM_ROLLERBLADE, 3}, {ITEM_KICK, 3}, {ITEM_PUNCH, 3}, {ITEM_SKULL, 2}}},
		{ZONE_WESTERN, {{ITEM_BOMB, 4}, {ITEM_FIRE, 3}, {ITEM_ROLLERBLADE, 2}, {ITEM_FULL_FIRE, 2}, {ITEM_KICK, 3}, {ITEM_PUNCH, 3}, {ITEM_SKULL, 1}}},
		{ZONE_BOMB, {{ITEM_BOMB, 3}, {ITEM_FIRE, 5}, {ITEM_ROLLERBLADE, 3}, {ITEM_KICK, 3}, {ITEM_PUNCH, 3}, {ITEM_SKULL, 2}}},
		{ZONE_JUMP, {{ITEM_BOMB, 4}, {ITEM_FIRE, 4}, {ITEM_ROLLERBLADE, 3}, {ITEM_KICK, 3}, {ITEM_PUNCH, 3}, {ITEM_SKULL, 2}}},
		{ZONE_BELT, {{ITEM_BOMB, 5}, {ITEM_FIRE, 5}, {ITEM_ROLLERBLADE, 3}, {ITEM_KICK, 3}, {ITEM_PUNCH, 3}}},
		{ZONE_SPEED, {{ITEM_BOMB, 6}, {ITEM_FIRE, 6}, {ITEM_FULL_FIRE, 2}, {ITEM_KICK, 3}, {ITEM_PUNCH, 3}, {ITEM_REMOCON, 2}, {ITEM_SKULL, 1}}},
	};

	try {
		num_items = item_numbers_per_zone.at(zone);
	} catch (std::out_of_range) {
		num_items.clear();
	}

	switch (zone) {
	case ZONE_DUEL:
		createDuelMap();
		break;
	default:
		break;
	}
}

void game_map::createDuelMap() {
	bool corner_breakables[] = {
		true, false, false,
		true, false, true, false, true,
		false, true, true,
		true, true,
	};
	bool *break_it = corner_breakables;

	for (int y = 2; y <= 5; ++y) {
		for (int x = 4; x <= 8; ++x) {
			tile &t1 = getTile(x, y);
			tile &t2 = getTile(width - 1 - x, y);
			tile &t3 = getTile(x, height - 1 - y);
			tile &t4 = getTile(width - 1 - x, height - 1 - y);
			if (t1.type == TILE_FLOOR) {
				if (*break_it) {
					t1.type = TILE_BREAKABLE;
					t2.type = TILE_BREAKABLE;
					t3.type = TILE_BREAKABLE;
					t4.type = TILE_BREAKABLE;
				}
				++break_it;
			}
		}
	}

	static const item_type border_items_h[] = {
		ITEM_BOMB, ITEM_BOMB, ITEM_ROLLERBLADE, ITEM_FULL_FIRE, ITEM_ROLLERBLADE, ITEM_BOMB, ITEM_BOMB,
	};

	static const item_type border_items_v[] = {
		ITEM_BOMB, ITEM_KICK, ITEM_PUNCH, ITEM_BOMB,
	};

	const item_type *item_it = border_items_h;
	for (int x = 2; x <= width - 3; x += 2) {
		addSpecial(std::make_shared<tile_item_spawner>(getTile(x, 1), *this, *item_it));
		addSpecial(std::make_shared<tile_item_spawner>(getTile(x, height - 2), *this, *item_it));

		++item_it;
	}

	item_it = border_items_v;
	for (int y = 3; y <= height - 4; y += 2) {
		addSpecial(std::make_shared<tile_item_spawner>(getTile(2, y), *this, *item_it));
		addSpecial(std::make_shared<tile_item_spawner>(getTile(width - 3, height - y - 1), *this, *item_it));

		++item_it;
	}
}