#include "player.h"

#include "ent_bomb.h"
#include "ent_item.h"
#include "main.h"

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <algorithm>

player::player(game_world *world, input_handler *handler, uint8_t player_num) : entity(world, TYPE_PLAYER), handler(handler), player_num(player_num) {
    if (!handler) {
        fprintf(stderr, "Player must have a handler\n");
    }

    memset(player_name, 0, NAME_SIZE);

    do_send_updates = false;

    victories = 0;
}

void player::respawn(float x, float y) {
    fx = x;
    fy = y;

    spawned = true;
    alive = true;

    can_kick = false;
    can_punch = false;

    speed = 5.f;
    explosion_size = 2;
    num_bombs = 1;

    moving = false;
    direction = 1;

    punch_ticks = 0;

    skull_effect = 0;
    skull_ticks = 0;

    item_pickups.clear();

    do_send_updates = true;
}

void player::kill() {
    if (!alive) return;

    alive = false;
    death_ticks = PLAYER_DEATH_TICKS;
}

void player::setName(const char *name) {
    strncpy(player_name, name, NAME_SIZE);
}

void player::handleInput() {
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

    float to_fx = fx;
    float to_fy = fy;

    float check_fx = fx;
    float check_fy = fy;

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
                world->addEntity(new bomb(world, this));
                --num_bombs;
            }
        }
    }

    if (can_punch && handler->isPressed(USR_PUNCH)) {
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
            b->punch(direction);
        }
    }

    int move_speed = speed;
    if (skull_effect == SKULL_RAPID_PACE) {
        move_speed = 30.f;
    } else if (skull_effect == SKULL_SLOW_PACE) {
        move_speed = 2.f;
    }

    while(! movement_priority.empty()) {
        uint8_t prio = movement_priority.front();
        if (movement_keys_down[prio]) {
            switch (prio) {
            case 0:
                to_fy -= move_speed;
                check_fy = to_fy - TILE_SIZE / 2;
                break;
            case 1:
                to_fy += move_speed;
                check_fy = to_fy + TILE_SIZE / 2;
                break;
            case 2:
                to_fx -= move_speed;
                check_fx = to_fx - TILE_SIZE / 2;
                break;
            case 3:
                to_fx += move_speed;
                check_fx = to_fx + TILE_SIZE / 2;
                break;
            }

            direction = prio;
            moving = true;

            int to_tx = check_fx / TILE_SIZE + 0.5f;
            int to_ty = check_fy / TILE_SIZE + 0.5f;

            if ((to_tx == getTileX() && to_ty == getTileY() &&
                 !world->isWalkable(getTileX(), getTileY())) ||
                world->isWalkable(to_tx, to_ty)) {
                fx = to_fx;
                fy = to_fy;
            }

            switch (direction) {
            case 0:
            case 1:
                fx += (getTileX() * TILE_SIZE - fx) * MOVE_TILE;
                break;
            case 2:
            case 3:
                fy += (getTileY() * TILE_SIZE - fy) * MOVE_TILE;
                break;
            }

            if (can_kick) {
                if (to_tx != getTileX() || to_ty != getTileY()) {
                    entity **ents = world->findEntities(to_tx, to_ty, TYPE_BOMB);
                    if (*ents) {
                        bomb *b = dynamic_cast<bomb *>(*ents);
                        b->kick(direction);
                    }
                }
            }

            break;
        } else {
            moving = false;
            movement_priority.pop_front();
        }
    }
}

void player::tick() {
    if (alive) {
        handleInput();
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
            }
            --skull_ticks;
        } else {
            skull_effect = 0;
        }
        if (punch_ticks > 0) {
            --punch_ticks;
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
        uint8_t item_type = item_pickups.front();

        world->addEntity(new game_item(world, tiles[i], item_type));
        item_pickups.pop_front();
        ++i;
    }
}

void player::writeEntity(packet_ext &packet) {
    packet.writeShort(PLAYERNAME_SIZE + 13);
    packet.writeInt(fx);
    packet.writeInt(fy);
    packet.writeString(player_name, PLAYERNAME_SIZE);
    packet.writeChar(player_num);

    uint8_t flags = 0;
    flags |= (1 << 0) * alive;
    flags |= (1 << 1) * spawned;
    flags |= (1 << 2) * moving;
    flags |= (1 << 3) * (punch_ticks > 0);
    flags |= (1 << 4) * (skull_ticks > 0);
    packet.writeChar(flags);
    packet.writeChar(direction);
}
