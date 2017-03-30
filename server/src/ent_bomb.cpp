#include "ent_bomb.h"
#include "ent_explosion.h"
#include "ent_item.h"

#include "player.h"
#include "main.h"
#include "game_sound.h"

bomb::bomb(game_world *world, player *p) : ent_movable(world, TYPE_BOMB), planter(p) {
	fx = p->getTileX() * TILE_SIZE;
	fy = p->getTileY() * TILE_SIZE;
	fz = 0.f;

	life_ticks = BOMB_LIFE;

	if (p->skull_effect == SKULL_LOW_POWER) {
		explode_size = 1;
	} else {
		explode_size = p->explosion_size;
	}

	piercing = p->pickups & PICKUP_HAS_REDBOMB;
	remocon = p->pickups & PICKUP_HAS_REMOCON;

	kicked = false;
	flying = false;

	do_send_updates = false;

	speedx = 0.f;
	speedy = 0.f;

	world->playWave(WAV_PLANT);
}

bomb::bomb(game_world *world, int tx, int ty) : ent_movable(world, TYPE_BOMB), planter(nullptr) {
	fx = tx * TILE_SIZE;
	fy = ty * TILE_SIZE;
	fz = BOMB_HEIGHT;

	flying = true;
	speedx = 0.f;
	speedy = 0.f;
	speedz = 0.f;

	explode_size = 2;

	life_ticks = BOMB_LIFE;

	piercing = false;
	remocon = false;
	kicked = false;
	do_send_updates = true;

	int dir = random_engine() % 4;
	switch(dir) {
	case 0:
		speedx = 1.f;
		break;
	case 1:
		speedx = -1.f;
		break;
	case 2:
		speedy = 1.f;
		break;
	case 3:
		speedy = -1.f;
		break;
	}
}

bool bomb::kick(player *p, float dx, float dy) {
	if (kicked || flying) return false;

	speedx = 0;
	speedy = 0;
	kick_ticks = 0;

	if (kicker) {
		kicker->stoppedKick(this);
	}
	kicker = p;

	if (dy < 0) {
		speedy = -KICK_SPEED / TICKRATE;
	} else if (dy > 0) {
		speedy = KICK_SPEED / TICKRATE;
	} else if (dx < 0) {
		speedx = -KICK_SPEED / TICKRATE;
	} else if (dx > 0) {
		speedx = KICK_SPEED / TICKRATE;
	} else {
		return false;
	}

	kicked = true;
	do_send_updates = true;

	return true;
}

bool bomb::punch(player *p) {
	if (kicked) kicked = false;
	if (flying) return false;

	if (kicker) {
		kicker->stoppedKick(this);
		kicker = nullptr;
	}

	world->playWave(WAV_PUNCH);

	speedx = 0;
	speedy = 0;

	switch (p->player_direction) {
	case DIR_UP:
		speedy = -PUNCH_SPEED / TICKRATE;
		break;
	case DIR_DOWN:
		speedy = PUNCH_SPEED / TICKRATE;
		break;
	case DIR_LEFT:
		speedx = -PUNCH_SPEED / TICKRATE;
		break;
	case DIR_RIGHT:
		speedx = PUNCH_SPEED / TICKRATE;
		break;
	default:
		return false;
	}

	speedz = PUNCH_SPEED_Z / TICKRATE;

	flying = true;
	do_send_updates = true;

	return true;
}

void bomb::stopKick() {
	kicked = false;
	fx = getTileX() * TILE_SIZE;
	fy = getTileY() * TILE_SIZE;
	if (kick_ticks > 1) {
		world->playWave(WAV_HARDHIT);
	}
	if (kicker) {
		kicker->stoppedKick(this);
		kicker = nullptr;
	}
}

bool bomb::move(float dx, float dy) {
	return std_move(dx, dy, WALK_BLOCK_PLAYERS | WALK_BLOCK_ITEMS);
}

void bomb::tick() {
	if (kicked) {
		if (kicked) {
			if (!move(speedx, speedy)) {
				stopKick();
			} else {
				++kick_ticks;
				if (kick_ticks % 5 == 0) {
					world->playWave(WAV_SLIDE);
				}
			}
		}
	} else if (flying) {
		if (speedx > 1 || speedx < -1) fx += speedx;
		if (speedy > 1 || speedy < -1) fy += speedy;
		fz += speedz;

		speedz -= PUNCH_Z_ACCEL / (TICKRATE * TICKRATE);

		if (speedx > 0 && fx > TILE_SIZE * MAP_WIDTH) {
			fx -= TILE_SIZE * MAP_WIDTH;
		}
		if (speedx < 0 && fx < 0) {
			fx += TILE_SIZE * MAP_WIDTH;
		}
		if (speedy > 0 && fy > TILE_SIZE * MAP_HEIGHT) {
			fy -= TILE_SIZE * MAP_HEIGHT;
		}
		if (speedy < 0 && fy < 0) {
			fy += TILE_SIZE * MAP_HEIGHT;
		}

		if (fz <= 0) {
			fz = 0.f;
			world->playWave(WAV_BOUNCE);
			float to_fx = getTileX() * TILE_SIZE;
			float to_fy = getTileY() * TILE_SIZE;
			if (world->isWalkable(to_fx, to_fy, WALK_BLOCK_PLAYERS | WALK_BLOCK_ITEMS)) {
				fx = to_fx;
				fy = to_fy;
				flying = false;
			} else {
				auto ents = world->findMovables(fx, fy, TYPE_PLAYER);
				for (auto &ent : ents) {
					std::dynamic_pointer_cast<player>(ent)->stun();
				}
				if (speedx > 0) speedx = PUNCH_SPEED_BOUNCE / TICKRATE;
				else if (speedx < 0) speedx = -PUNCH_SPEED_BOUNCE / TICKRATE;
				if (speedy > 0) speedy = PUNCH_SPEED_BOUNCE / TICKRATE;
				else if (speedy < 0) speedy = -PUNCH_SPEED_BOUNCE / TICKRATE;
				speedz = PUNCH_SPEED_Z_BOUNCE / TICKRATE;
			}
		}
	}

	if (!remocon) {
		if (life_ticks <= 0 && ! isFlying()) {
			explode();
		}
		--life_ticks;
	}
}

void bomb::explode() {
	if (exploded) return;
	if (flying) return;

	if (planter) {
		planter->explodedBomb(this);
	}
	if (kicker) {
		kicker->stoppedKick(this);
	}

	exploded = true;
	destroy();
	world->addEntity(std::make_shared<explosion>(world, this));
}

byte_array bomb::toByteArray() {
	byte_array ba;

	ba.writeInt(fx);
	ba.writeInt(fy);
	ba.writeInt(fz);
	if (planter) {
		ba.writeShort(planter->getID());
	} else {
		ba.writeShort(0);
	}
	uint8_t flags = 0;
	flags |= (1 << 0) * piercing;
	flags |= (1 << 1) * remocon;
	ba.writeChar(flags);

	return ba;
}
