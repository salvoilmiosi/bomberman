#include "tile_belt.h"

#include "ent_movable.h"

tile_belt::tile_belt(tile &t_tile, game_map *g_map, direction belt_direction) : tile_entity(SPECIAL_BELT, t_tile, g_map), belt_direction(belt_direction) {
	tx = g_map->getTileX(t_tile);
	ty = g_map->getTileY(t_tile);

	uint16_t data = (belt_direction & 0x3) << 10;

	switch(t_tile.type) {
	case TILE_BREAKABLE:
		is_breakable = true;
		item = static_cast<item_type>(t_tile.data & 0xff);
		data |= 0x1000;
		break;
	default:
		break;
	}

	setData(data);
}

void tile_belt::tick() {
	auto ents = g_map->getWorld()->findMovables(tx * TILE_SIZE, ty * TILE_SIZE);
	float speedx = 0;
	float speedy = 0;
	switch (belt_direction) {
	case DIR_UP:
		speedy = -BELT_SPEED / TICKRATE;
		break;
	case DIR_DOWN:
		speedy = BELT_SPEED / TICKRATE;
		break;
	case DIR_LEFT:
		speedx = -BELT_SPEED / TICKRATE;
		break;
	case DIR_RIGHT:
		speedx = BELT_SPEED / TICKRATE;
		break;
	}
	for (auto &ent : ents) {
		ent->move(speedx, speedy);
	}
}

bool tile_belt::bombHit() {
	if (is_breakable) {
		game_world *world = g_map->getWorld();
		world->addEntity(std::make_shared<broken_wall>(world, t_tile, item));
		is_breakable = false;
		setData((belt_direction & 0x3) << 10);
		return true;
	} else {
		return false;
	}
}
