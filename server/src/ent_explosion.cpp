#include "ent_explosion.h"

#include "ent_bomb.h"
#include "ent_item.h"
#include "ent_broken_wall.h"

#include "player.h"
#include "game_sound.h"

explosion::explosion(game_world &world, bomb *b) : entity(world, TYPE_EXPLOSION) {
	planter = b->planter;

	tx = b->getTileX();
	ty = b->getTileY();

	life_ticks = EXPLOSION_LIFE;

	int explode_size = b->explode_size;

	piercing = b->piercing;

	trunc_l = false;
	trunc_t = false;
	trunc_r = false;
	trunc_b = false;

	auto ents = world.findEntities(tx, ty, TYPE_ITEM);
	for (auto &ent : ents) {
		std::dynamic_pointer_cast<game_item>(ent)->explode();
	}

	do_send_updates = false;

	tile &t = world.getMap().getTile(tx, ty);
	switch (t.type) {
	case TILE_WALL:
		return;
	case TILE_BREAKABLE:
		world.addEntity(std::make_shared<broken_wall>(world, t));
		t.type = TILE_FLOOR;
		return;
	default:
		break;
	}

	len_l = destroyTiles(-explode_size, 0, &trunc_l);
	len_t = destroyTiles(0, -explode_size, &trunc_t);
	len_r = destroyTiles(explode_size, 0, &trunc_r);
	len_b = destroyTiles(0, explode_size, &trunc_b);

	world.playWave(WAV_EXPLODE);
}

uint8_t explosion::destroyTiles(int dx, int dy, bool *trunc) {
	int x = tx;
	int y = ty;
	uint8_t i = 0;
	while(true) {
		if (dx < 0) {
			--x;
			++dx;
		} else if (dx > 0) {
			++x;
			--dx;
		} else if (dy < 0) {
			--y;
			++dy;
		} else if (dy > 0) {
			++y;
			--dy;
		} else {
			break;
		}

		auto tile_ents = world.findEntities(x, y);
		for (auto &ent : tile_ents) {
			switch (ent->getType()) {
			case TYPE_BOMB:
			{
				auto b = std::dynamic_pointer_cast<bomb>(ent);
				if (! b->isNotDestroyed()) {
					continue;
				}
				if (!b->isFlying()) {
					return i + 1;
				}
				break;
			}
			case TYPE_ITEM:
			{
				std::dynamic_pointer_cast<game_item>(ent)->explode();
				if (!piercing) {
					*trunc = true;
					return i;
				}
				break;
			}
			default:
				break;
			}
		}

		try {
			tile &t = world.getMap().getTile(x, y);
			switch (t.type) {
			case TILE_BREAKABLE:
				world.addEntity(std::make_shared<broken_wall>(world, t));
				t.type = TILE_FLOOR;
				if (piercing) {
					break;
				} else {
					*trunc = true;
				}
			case TILE_WALL:
				*trunc = true;
				return i;
			case TILE_SPECIAL:
			{
				special_ptr special = world.getMap().getSpecial(t);
				if (!special) break;
				if (special->bombHit()) {
					if (piercing) {
						break;
					} else {
						*trunc = true;
						return i;
					}
				} else {
					break;
				}
			}
			default:
				break;
			}
		} catch (std::out_of_range) {
			*trunc = true;
			break;
		}
		++i;
	}
	return i;
}

void explosion::checkEntities(int dx, int dy) {
	int x = tx;
	int y = ty;
	while(true) {
		if (dx < 0) {
			--x;
			++dx;
		} else if (dx > 0) {
			++x;
			--dx;
		} else if (dy < 0) {
			--y;
			++dy;
		} else if (dy > 0) {
			++y;
			--dy;
		} else {
			break;
		}

		auto ents = world.findEntities(x, y);
		for (auto &ent : ents) {
			switch(ent->getType()) {
			case TYPE_PLAYER:
				std::dynamic_pointer_cast<player>(ent)->kill(planter);
				break;
			case TYPE_BOMB:
				if (EXPLOSION_LIFE - life_ticks >= EXPLOSION_CHAIN_DELAY) {
					auto b = std::dynamic_pointer_cast<bomb>(ent);
					if (!b->isFlying()) {
						b->explode();
					}
				}
				break;
			default:
				break;
			}
		}
	}
}

void explosion::tick() {
	auto ents = world.findEntities(tx, ty, TYPE_PLAYER);
	for (auto &ent : ents) {
		std::dynamic_pointer_cast<player>(ent)->kill(planter);
	}

	checkEntities(-len_l, 0);
	checkEntities(0, -len_t);
	checkEntities(len_r, 0);
	checkEntities(0, len_b);

	if (life_ticks == 0) {
		destroy();
	}
	--life_ticks;
}

byte_array explosion::toByteArray() {
	byte_array ba;

	ba.writeChar(tx);
	ba.writeChar(ty);
	ba.writeChar(len_l);
	ba.writeChar(len_t);
	ba.writeChar(len_r);
	ba.writeChar(len_b);

	uint8_t trunc = 0;
	trunc |= (1 << 0) * trunc_l;
	trunc |= (1 << 1) * trunc_t;
	trunc |= (1 << 2) * trunc_r;
	trunc |= (1 << 3) * trunc_b;

	ba.writeChar(trunc);

	return ba;
}
