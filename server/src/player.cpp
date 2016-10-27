#include "player.h"

#include "ent_bomb.h"
#include "ent_item.h"
#include "main.h"

#include "tile_trampoline.h"
#include "game_sound.h"

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <algorithm>

player::player(game_world *world, input_handler *handler) : ent_movable(world, TYPE_PLAYER), handler(handler) {
    memset(player_name, 0, PLAYER_NAME_SIZE);

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

    pickups = 0;

    speed = 300.f;
    explosion_size = 2;
    num_bombs = 1;

    moving = false;
    direction = 1;

    punch_ticks = 0;
    invulnerable_ticks = 0;
    stun_ticks = 0;

    skull_effect = 0;
    skull_ticks = 0;

    item_pickups.clear();
    planted_bombs.clear();
    kicked_bombs.clear();

    do_send_updates = true;
}

void player::kill(player *killer) {
    if (!alive) return;
    if (jumping) return;
    if (invulnerable_ticks > 0) return;

    alive = false;
    death_ticks = PLAYER_DEATH_TICKS;

    world->playWave(WAV_DEATH);

    world->sendKillMessage(killer, this);
}

void player::makeInvulnerable() {
    invulnerable_ticks = PLAYER_INVULNERABLE_TICKS;
}

void player::setName(const char *name) {
    strncpy(player_name, name, PLAYER_NAME_SIZE);
}

void player::handleInput() {
    if (!handler)
        return;

    movement_keys_down[0] = handler->isDown(USR_UP);
    movement_keys_down[1] = handler->isDown(USR_DOWN);
    movement_keys_down[2] = handler->isDown(USR_LEFT);
    movement_keys_down[3] = handler->isDown(USR_RIGHT);

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
            entity **ents = world->findEntities(getTileX(), getTileY(), TYPE_BOMB);
            bool found_bomb = false;
            for (uint8_t i=0; i<SEARCH_SIZE; ++i) {
                if (ents[i]) {
                    found_bomb = true;
                    break;
                }
            }
            if (!found_bomb) {
                bomb *b = new bomb(world, this);
                world->addEntity(b);
                if (pickups & PICKUP_HAS_REMOCON) {
                    planted_bombs.push_back(b);
                }
                --num_bombs;
            }
        }
    }

    if (pickups & PICKUP_HAS_PUNCH && handler->isPressed(USR_PUNCH)) {
        punch_ticks = PLAYER_PUNCH_TICKS;
        moving = false;

        int bx = getTileX();
        int by = getTileY();

        switch(direction) {
        case 0:
            --by;
            break;
        case 1:
            ++by;
            break;
        case 2:
            --bx;
            break;
        case 3:
            ++bx;
            break;
        }

        entity **ents = world->findEntities(bx, by, TYPE_BOMB);
        for (uint8_t i=0; i<SEARCH_SIZE; ++i) {
            entity *ent = ents[i];
            if (!ent) break;
            bomb *b = dynamic_cast<bomb*>(ent);
            b->punch(this);
        }
    }

    if (pickups & PICKUP_HAS_REMOCON && ! planted_bombs.empty() && handler->isPressed(USR_DETONATE)) {
        bomb *b = planted_bombs.front();
        if (b) {
            b->explode();
        }
    }

    if (pickups & PICKUP_HAS_KICK && ! kicked_bombs.empty() && handler->isPressed(USR_STOP_KICK)) {
        bomb *b = kicked_bombs.back();
        if (b) {
            b->stopKick();
        }
    }

    float move_speed = speed;
    if (skull_effect == SKULL_RAPID_PACE) {
        move_speed = 1000.f;
    } else if (skull_effect == SKULL_SLOW_PACE) {
        move_speed = 120.f;
    }
    move_speed /= TICKRATE;

    while(! movement_priority.empty()) {
        uint8_t prio = movement_priority.front();
        if (movement_keys_down[prio]) {
            switch (prio) {
            case 0:
                move(0, -move_speed);
                break;
            case 1:
                move(0, move_speed);
                break;
            case 2:
                move(-move_speed, 0);
                break;
            case 3:
                move(move_speed, 0);
                break;
            }

            direction = prio;
            moving = true;
            break;
        } else {
            moving = false;
            movement_priority.pop_front();
        }
    }
}

void player::move(float dx, float dy) {
    uint8_t start_tx = getTileX();
    uint8_t start_ty = getTileY();

    uint8_t end_tx, end_ty;

    if (ent_movable::move(dx, dy)) {
        end_tx = getTileX();
        end_ty = getTileY();
    } else {
        return false;
    }

    if (end_tx != start_tx || end_ty != start_ty) {
        if (alive && pickups & PICKUP_HAS_KICK) {
            entity **ents = world->findEntities(to_tx, to_ty, TYPE_BOMB);
            if (*ents) {
                if (kick_ticks <= 0) {
                    bomb *b = dynamic_cast<bomb *>(*ents);
                    if (b->kick(this)) {
                        kicked_bombs.push_back(b);
                    }
                } else {
                    --kick_ticks;
                }
            } else {
                kick_ticks = PLAYER_KICK_TICKS;
            }
        }
    }

    return true;
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

void player::checkTrampoline() {
    if (jumping) return;

    tile *walkingon = world->getMap().getTile(getTileX(), getTileY());
    if (walkingon->type == TILE_SPECIAL) {
        tile_entity *ent = world->getMap().getSpecial(walkingon);
        if (ent && ent->getType() == SPECIAL_TRAMPOLINE) {
            tile_trampoline *tramp = dynamic_cast<tile_trampoline *>(ent);
            if (! on_trampoline && tramp->jump()) {
                world->playWave(WAV_JUMP);
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

        entity **ents = world->findEntities(getTileX(), getTileY());
        for (uint8_t i=0; i < SEARCH_SIZE; ++i) {
            entity *ent = ents[i];
            if (!ent) break;
            if (ent->getType() != TYPE_ITEM) continue;
            game_item *item = dynamic_cast<game_item*>(ent);
            item_pickups.push_back(item->getItemType());
            item->pickup(this);
        }
        if (skull_ticks > 0) {
            for (uint8_t i=0; i < SEARCH_SIZE; ++i) {
                entity *ent = ents[i];
                if (!ent) break;
                if (ent->getType() != TYPE_PLAYER) continue;
                player *p = dynamic_cast<player*>(ent);
                if (p == this) continue;
                if (p->skull_ticks > 0) continue;
                p->skull_ticks = SKULL_LIFE;
                p->skull_effect = skull_effect;
                world->playWave(WAV_SKULL);
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
    std::vector<tile *> tiles;
    for (int ty = 0; ty < MAP_HEIGHT; ++ty) {
        for (int tx = 0; tx < MAP_WIDTH; ++tx) {
            if (world->isWalkable(tx, ty)) {
                tiles.push_back(world->getMap().getTile(tx, ty));
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

        world->addEntity(new game_item(world, tiles[i], type));
        item_pickups.pop_front();
        ++i;
    }
}

byte_array player::toByteArray() {
    byte_array ba;

    ba.writeInt(fx);
    ba.writeInt(fy);
    ba.writeInt(fz);

    ba.writeString(player_name, PLAYER_NAME_SIZE);
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
    flags |= (direction & 0x3) << 14;
    ba.writeShort(flags);

    return ba;
}
