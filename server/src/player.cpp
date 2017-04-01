#include "player.h"

#include "ent_bomb.h"
#include "ent_item.h"
#include "main.h"

#include "tile_trampoline.h"
#include "game_sound.h"

#include <cstdio>
#include <cstdlib>
#include <algorithm>

player::player(game_world &world, input_handler *handler) : ent_movable(world, TYPE_PLAYER), handler(handler) {
	do_send_updates = false;

	victories = 0;
}

void player::respawn(int tx, int ty) {
	fx = tx * TILE_SIZE;
	fy = ty * TILE_SIZE;
	fz = 0;
	speedz = 0;

	spawned = true;
	alive = true;
	on_trampoline = false;

	pickups = world.getStartPickups();

	speed = 300.f;
	explosion_size = 2;
	num_bombs = 1;

	moving = false;
	player_direction = DIR_DOWN;

	punch_ticks = 0;
	invulnerable_ticks = 0;
	stun_ticks = 0;

	skull_effect = 0;
	skull_ticks = 0;

	item_pickups.clear();
	planted_bombs.clear();
	kicked_bombs.clear();

	auto start_items = world.getStartItems();
	for (auto item : start_items) {
		for (int i=0; i<item.second; ++i) {
			pickupItem(item.first, false);
		}
	}

	do_send_updates = true;
}

void player::kill(player *killer) {
	if (!alive) return;
	if (jumping) return;
	if (invulnerable_ticks > 0) return;

	alive = false;
	death_ticks = PLAYER_DEATH_TICKS;

	world.playWave(WAV_DEATH);

	world.sendKillMessage(killer, this);
}

void player::makeInvulnerable() {
	invulnerable_ticks = PLAYER_INVULNERABLE_TICKS;
}

void player::handleInput() {
	if (!handler)
		return;

	movement_keys_down[DIR_UP] = handler->isDown(USR_UP);
	movement_keys_down[DIR_DOWN] = handler->isDown(USR_DOWN);
	movement_keys_down[DIR_LEFT] = handler->isDown(USR_LEFT);
	movement_keys_down[DIR_RIGHT] = handler->isDown(USR_RIGHT);

	for (int i=0; i<4; ++i) {
		if (movement_keys_down[i]) {
			if (!movement_keys_pressed[i]) {
				movement_keys_pressed[i] = true;
				movement_priority.push_front(i);
			}
		} else {
			movement_keys_pressed[i] = false;
		}
	}

	if (jumping) return;
	if (stun_ticks > 0) return;

	if (punch_ticks > 0) return;

	if (skull_effect != SKULL_CONSTIPATION && (skull_effect == SKULL_DIARRHEA || handler->isPressed(USR_PLANT))) {
		if (num_bombs > 0) {
			auto ents = world.findEntities(getTileX(), getTileY(), TYPE_BOMB);
			if (ents.empty()) {
				auto b = std::make_shared<bomb>(world, this);
				world.addEntity(b);
				if (pickups & PICKUP_HAS_REMOCON) {
					planted_bombs.push_back(b.get());
				}
				--num_bombs;
			}
		}
	}

	if (pickups & PICKUP_HAS_PUNCH && handler->isPressed(USR_PUNCH)) {
		punch_ticks = PLAYER_PUNCH_TICKS;
		moving = false;

		float bx = fx;
		float by = fy;

		static const float PUNCH_DISTANCE = TILE_SIZE * 2 / 3;

		switch(player_direction) {
		case DIR_UP:
			by -= PUNCH_DISTANCE;
			break;
		case DIR_DOWN:
			by += PUNCH_DISTANCE;
			break;
		case DIR_LEFT:
			bx -= PUNCH_DISTANCE;
			break;
		case DIR_RIGHT:
			bx += PUNCH_DISTANCE;
			break;
		}

		auto mov_ents = world.findMovables(bx, by, TYPE_BOMB);
		for (auto &ent : mov_ents) {
			bomb *b = dynamic_cast<bomb*>(ent.get());
			b->punch(this);
		}
	}

	if (pickups & PICKUP_HAS_REMOCON && ! planted_bombs.empty() && handler->isPressed(USR_DETONATE)) {
		auto b = planted_bombs.front();
		if (b) {
			b->explode();
		}
	}

	if (pickups & PICKUP_HAS_KICK && ! kicked_bombs.empty() && handler->isPressed(USR_STOP_KICK)) {
		auto b = kicked_bombs.back();
		if (b) {
			b->stopKick();
		}
	}

	float move_speed = speed;
	if (skull_effect == SKULL_SLOW_PACE) {
		move_speed = 120.f;
	} else if (skull_effect == SKULL_RAPID_PACE || pickups & PICKUP_HAS_SPEED) {
		move_speed = 960.f;
	}
	move_speed /= TICKRATE;

	while(! movement_priority.empty()) {
		uint8_t prio = movement_priority.front();
		if (movement_keys_down[prio]) {
			direction new_dir = static_cast<direction>(prio);
			switch (new_dir) {
			case DIR_UP:
				player_move(0, -move_speed);
				break;
			case DIR_DOWN:
				player_move(0, move_speed);
				break;
			case DIR_LEFT:
				player_move(-move_speed, 0);
				break;
			case DIR_RIGHT:
				player_move(move_speed, 0);
				break;
			}

			player_direction = new_dir;
			moving = true;
			break;
		} else {
			moving = false;
			movement_priority.pop_front();
		}
	}
}

bool player::move(float dx, float dy) {
	return std_move(dx, dy);
}

bool player::player_move(float dx, float dy) {
	if (move(dx, dy)) {
		return true;
	}

	float bx = fx;
	float by = fy;
	if (dx < 0) {
		bx -= TILE_SIZE;
	} else if (dx > 0) {
		bx += TILE_SIZE;
	} else if (dy < 0) {
		by -= TILE_SIZE;
	} else if (dy > 0) {
		by += TILE_SIZE;
	} else {
		return false;
	}

	if (alive && pickups & PICKUP_HAS_KICK) {
		auto mov_ents = world.findMovables(bx, by, TYPE_BOMB);
		if (!mov_ents.empty()) {
			if (kick_ticks <= 0) {
				auto b = std::dynamic_pointer_cast<bomb>(mov_ents.front());
				if (b && b->kick(this, dx, dy)) {
					kicked_bombs.push_back(b.get());
				}
			} else {
				--kick_ticks;
			}
		} else {
			kick_ticks = PLAYER_KICK_TICKS;
		}
	}

	return false;
}

void player::explodedBomb(bomb *b) {
	++num_bombs;

	auto it = std::find(planted_bombs.begin(), planted_bombs.end(), b);
	if (it != planted_bombs.end()) {
		planted_bombs.erase(it);
	}

	stoppedKick(b);
}

void player::stoppedKick(bomb *b) {
	auto it = std::find(kicked_bombs.begin(), kicked_bombs.end(), b);
	if (it != kicked_bombs.end()) {
		kicked_bombs.erase(it);
	}
}

void player::addVictory() {
	++victories;
	makeInvulnerable();
}

void player::stun() {
	stun_ticks = PLAYER_STUN_TICKS;
}

void player::pickupItem(item_type type, bool add_to_pickups) {
	switch (type) {
	case ITEM_NONE:
		break;
	case ITEM_BOMB:
		++num_bombs;
		if (num_bombs > 10) {
			num_bombs = 10;
		}
		break;
	case ITEM_FIRE:
		++explosion_size;
		if (explosion_size > 10) {
			explosion_size = 10;
		}
		break;
	case ITEM_ROLLERBLADE:
		speed += 80.f;
		break;
	case ITEM_KICK:
		pickups |= PICKUP_HAS_KICK;
		break;
	case ITEM_PUNCH:
		pickups |= PICKUP_HAS_PUNCH;
		break;
	case ITEM_SKULL:
		skull_ticks = SKULL_LIFE;
		skull_effect = random_engine() % 5 + 1;
		break;
	case ITEM_FULL_FIRE:
		explosion_size = 10;
		break;
	case ITEM_REDBOMB:
		pickups |= PICKUP_HAS_REDBOMB;
		break;
	case ITEM_REMOCON:
		pickups |= PICKUP_HAS_REMOCON;
		break;
	}

	if (add_to_pickups) {
		item_pickups.push_back(type);
	}
}
void player::checkTrampoline() {
	if (jumping) return;

	const tile &walkingon = world.getMap().getTile(getTileX(), getTileY());
	if (walkingon.type == TILE_SPECIAL) {
		special_ptr ent = world.getMap().getSpecial(walkingon);
		if (ent && ent->getType() == SPECIAL_TRAMPOLINE) {
			auto tramp = std::dynamic_pointer_cast<tile_trampoline>(ent);
			if (! on_trampoline && tramp->jump()) {
				world.playWave(WAV_JUMP);
				jumping = true;

				speedz = PLAYER_JUMP_SPEED / TICKRATE;
				fz = 0.f;
			}
			on_trampoline = true;
			return;
		}
	}
	on_trampoline = false;
}

void player::tick() {
	if (alive) {
		handleInput();
		checkTrampoline();

		if (jumping) {
			fz += speedz;
			speedz -= PLAYER_Z_ACCEL / (TICKRATE * TICKRATE);
			if (fz <= 0.f) {
				jumping = false;
				fz = 0.f;
			}
		}

		auto ents = world.findEntities(getTileX(), getTileY(), TYPE_ITEM);
		for (auto &ent : ents) {
			auto item = std::dynamic_pointer_cast<game_item>(ent);
			if (item->pickup()) {
				pickupItem(item->getItemType());
			}
		}
		if (skull_ticks > 0) {
			auto movs = world.findMovables(fx, fy, TYPE_PLAYER);
			for (auto &ent : movs) {
				auto p = std::dynamic_pointer_cast<player>(ent);
				if (p.get() == this) continue;
				if (!p->isAlive()) continue;
				if (p->skull_ticks > 0) continue;
				p->skull_ticks = SKULL_LIFE;
				p->skull_effect = skull_effect;
				world.playWave(WAV_SKULL);
			}
			--skull_ticks;
		} else {
			skull_effect = 0;
		}
		if (punch_ticks > 0) {
			--punch_ticks;
		}
		if (invulnerable_ticks > 0) {
			--invulnerable_ticks;
		}
		if (stun_ticks > 0) {
			--stun_ticks;
		}
	} else {
		if (death_ticks == 0) {
			spawnItems();
			do_send_updates = false;
		}
		--death_ticks;
	}
}

void player::spawnItems() {
	std::vector<const tile *> tiles;
	for (int ty = 0; ty < MAP_HEIGHT; ++ty) {
		for (int tx = 0; tx < MAP_WIDTH; ++tx) {
			if (world.isWalkable(tx * TILE_SIZE, ty * TILE_SIZE)) {
				tiles.push_back(& world.getMap().getTile(tx, ty));
			}
		}
	}

	std::shuffle(tiles.begin(), tiles.end(), random_engine);

	uint32_t i = 0;
	while(!item_pickups.empty()) {
		if (i >= tiles.size()) {
			break;
		}
		item_type type = item_pickups.front();

		world.addEntity(std::make_shared<game_item>(world, *tiles[i], type));
		item_pickups.pop_front();
		++i;
	}
}

byte_array player::toByteArray() {
	byte_array ba;

	ba.writeInt(fx);
	ba.writeInt(fy);
	ba.writeInt(fz);

	ba.writeString(player_name.c_str(), MAX_NAME_SIZE);
	ba.writeChar(player_num);

	uint16_t flags = 0;
	flags |= (1 << 0) * alive;
	flags |= (1 << 1) * spawned;
	flags |= (1 << 2) * moving;
	flags |= (1 << 3) * (punch_ticks > 0);
	flags |= (1 << 4) * (skull_ticks > 0);
	flags |= (1 << 5) * jumping;
	flags |= (1 << 6) * (invulnerable_ticks > 0);
	flags |= (1 << 7) * (stun_ticks > 0);
	flags |= (player_direction & 0x3) << 14;
	ba.writeShort(flags);

	return ba;
}
