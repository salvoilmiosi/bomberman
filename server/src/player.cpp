#include "player.h"

#include "ent_bomb.h"
#include "ent_item.h"

#include <cstdio>
#include <cstring>
#include <cstdlib>

uint8_t player::max_playernum = 0;

player::player(game_world *world, input_handler *handler) : entity(world, TYPE_PLAYER), handler(handler) {
    if (!handler) {
        fprintf(stderr, "Player must have a handler\n");
    }

    memset(player_name, 0, NAME_SIZE);

    player_num = max_playernum;
    max_playernum = (max_playernum + 1) % 4;

    do_send_updates = false;
}

void player::respawn(float x, float y) {
    fx = x;
    fy = y;

    spawned = true;
    alive = true;

    speed = 3.f;
    explosion_size = 1;
    num_bombs = 1;

    moving = false;
    direction = 1;

    do_send_updates = true;
}

void player::kill() {
    alive = false;
}

void player::setName(const char *name) {
    strncpy(player_name, name, NAME_SIZE);
}

bool player::isWalkable(int tx, int ty) {
    tile *t = world->getMap().getTile(tx, ty);
    if (!t) return false;

    entity **ents = world->findEntities(tx, ty, TYPE_BOMB);
    for (uint8_t i = 0; i < SEARCH_SIZE; ++i) {
        entity *ent = ents[i];
        if (ent) {
            return false;
        }
    }

    switch(t->type) {
    case tile::TILE_WALL:
    case tile::TILE_BREAKABLE:
    case tile::TILE_ITEM:
        return false;
    default:
        return true;
    }
}

void player::handleInput() {
    // how the fuck do I make this better
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

    while(! movement_priority.empty()) {
        uint8_t prio = movement_priority.front();
        if (movement_keys_down[prio]) {
            switch (prio) {
            case 0:
                to_fy -= speed;
                check_fy = to_fy;
                break;
            case 1:
                to_fy += speed;
                check_fy = to_fy + 32;
                break;
            case 2:
                to_fx -= speed;
                check_fx = to_fx - 24;
                break;
            case 3:
                to_fx += speed;
                check_fx = to_fx + 24;
                break;
            }

            int to_tx = check_fx / TILE_SIZE + 0.5f;
            int to_ty = check_fy / TILE_SIZE + 0.5f;

            bool walk_from = isWalkable(getTileX(), getTileY());
            bool walk_to = false;
            if ((!walk_from && to_tx == getTileX() && to_ty == getTileY()) || (walk_to = isWalkable(to_tx, to_ty))) {
                fx = to_fx;
                fy = to_fy;
            }

            direction = prio;
            moving = true;
            break;
        } else {
            moving = false;
            movement_priority.pop_front();
        }
    }

    if (handler->isPressed(USR_FIRE)) {
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
}

void player::tick() {
    if (alive) {
        handleInput();
        entity **ents = world->findEntities(getTileX(), getTileY(), TYPE_ITEM);
        for (uint8_t i=0; i < SEARCH_SIZE; ++i) {
            if (!ents[i]) break;
            game_item *item = (game_item *)(ents[i]);
            switch (item->getItemType()) {
            case ITEM_ADD_BOMB:
                ++num_bombs;
                break;
            case ITEM_ADD_LENGTH:
                ++explosion_size;
                break;
            case ITEM_ADD_SPEED:
                speed += 2.f;
                break;
            default:
                break;
            }
            item->destroy();
        }
    }
}

void player::writeEntity(packet_ext &packet) {
    packet.writeShort(NAME_SIZE + 16);
    packet.writeInt(fx);
    packet.writeInt(fy);
    packet.writeString(player_name, NAME_SIZE);
    packet.writeChar(player_num);

    uint8_t flags = 0;
    flags |= (1 << 0) * alive;
    flags |= (1 << 1) * spawned;
    flags |= (1 << 2) * moving;
    packet.writeChar(flags);
    packet.writeChar(direction);
}
