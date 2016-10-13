#include "game_world.h"

#include "player.h"

#include <cstring>

#include "ent_bomb.h"
#include "ent_item.h"
#include "main.h"

game_world::game_world(uint8_t num_players) : server(this, num_players) {
    NUM_PLAYERS = num_players;
    restartGame();
}

game_world::~game_world() {
	for (auto ent : entities) {
		if (ent) delete ent;
	}
}

int game_world::startServer(uint16_t port) {
    server.openServer(port);

    if (server.isOpen()) {
        printf("Server listening on port %d\n", port);
    } else {
        fprintf(stderr, "Could not open server socket: %s\n", SDLNet_GetError());
        return 1;
    }

    return server.run();
}

void game_world::addEntity(entity *ent) {
    if (!ent) return;

    entities.insert(ent);

    server.sendAddPacket(ent);
}

void game_world::removeEntity(entity *ent) {
    if (!ent) return;

    server.sendRemovePacket(ent);

    ent->destroy();
}

void game_world::tick() {
    if (!round_started) return;

    --ticks_to_start;

    if (ticks_to_start == 0) {
        server.messageToAll(COLOR_RED, "Round %d, fight!", round_num);
        countdownEnd();
    } else if (ticks_to_start > 0 && ticks_to_start % TICKRATE == 0) {
        int seconds = ticks_to_start / TICKRATE;
        server.messageToAll(COLOR_GREEN, "Round %d begins in %d seconds", round_num, seconds);
    }

    int alive_players = 0;
    int num_players = 0;

    g_map.tick();

    auto it = entities.begin();
    while (it != entities.end()) {
        entity *ent = *it;
        if (ent && ent->isNotDestroyed()) {
            if (ent->getType() == TYPE_PLAYER) {
                player *p = dynamic_cast<player*>(ent);
                if (p->isAlive()) {
                    ++alive_players;
                }
                ++num_players;
            }
            ent->tick();
            ++it;
        } else {
            it = entities.erase(it);
            removeEntity(ent);
            delete ent;
        }
    }

    bool round_end = alive_players <= (NUM_PLAYERS == 1 ? 0 : 1);
    if (ticks_to_start < 0 && round_started && round_end) {
        if (alive_players == 0) {
            server.messageToAll(COLOR_RED, "DRAW!");
        }
        for (entity *ent : entities) {
            if (ent && ent->isNotDestroyed() && ent->getType() == TYPE_PLAYER) {
                player *p = dynamic_cast<player*>(ent);
                if (p->isAlive()) {
                    server.messageToAll(COLOR_RED, "%s won round %d!", p->getName(), round_num);
                    p->addVictory();
                    break;
                }
            }
        }
        round_started = false;
        if (num_players == alive_players) {
            server.messageToAll(COLOR_RED, "Match is forfeited.");
            server.closeServer();
            //restartGame();
        } else {
            startRound();
        }
    }
}

void game_world::writeEntitiesToPacket(packet_ext &packet, bool is_add) {
    for(entity *ent : entities) {
        if (ent && ent->isNotDestroyed() && (is_add || ent->doSendUpdates())) {
            ent->writeToPacket(packet);
        }
    }
}

void game_world::restartGame() {
    round_started = false;
    round_num = 0;
    ticks_to_start = 0;
}

void game_world::startRound() {
    if (round_started) return;
    if (ticks_to_start > 0) return;

    ++round_num;

    ticks_to_start = TICKRATE * COUNTDOWN_SECONDS;
    round_started = true;
}

void game_world::countdownEnd() {
    g_map.createMap(MAP_WIDTH, MAP_HEIGHT, NUM_PLAYERS, random_engine() % 4);

    for (entity *ent : entities) {
        if (ent && ent->isNotDestroyed()) {
            switch (ent->getType()) {
            case TYPE_PLAYER:
                {
                    player *p = (player *)ent;
                    point spawn_pt = g_map.getSpawnPt(p->getPlayerNum());
                    p->respawn(spawn_pt.x * TILE_SIZE, spawn_pt.y * TILE_SIZE);
                }
                break;
            default:
                ent->destroy();
                break;
            }
        }
    }
}

entity **game_world::findEntities(int tx, int ty, uint8_t type) {
    static entity *ents[SEARCH_SIZE];
    int i = 0;

    memset(ents, 0, sizeof(ents));
    for (entity *ent : entities) {
        if (ent && ent->isNotDestroyed() && (type == 0 ? true : ent->getType() == type) && ent->getTileX() == tx && ent->getTileY() == ty) {
            ents[i] = ent;
            ++i;
        }
    }

    return ents;
}

bool game_world::isWalkable(int tx, int ty, uint8_t flags) {
    tile *t = getMap().getTile(tx, ty);
    if (!t) return false;

    entity **ents = findEntities(tx, ty);
    for (uint8_t i = 0; i < SEARCH_SIZE; ++i) {
        entity *ent = ents[i];
        if (!ent) break;
        switch (ent->getType()) {
        case TYPE_BOMB:
            if (flags & WALK_IGNORE_BOMBS) {
                break;
            } else {
                bomb *b = dynamic_cast<bomb*>(ent);
                if (b && b->isFlying()) {
                    break;
                }
            }
            return false;
        case TYPE_ITEM:
            if (flags & WALK_BLOCK_ITEMS) {
                game_item *item = dynamic_cast<game_item *>(ent);
                if (item && item->isAlive()) {
                    return false;
                }
            }
            break;
        case TYPE_PLAYER:
            if (flags & WALK_BLOCK_PLAYERS) {
                player *p = dynamic_cast<player *>(ent);
                if (p && p->isAlive()) {
                    return false;
                }
            }
            break;
        case TYPE_BROKEN_WALL:
            return false;
        }
    }

    switch(t->type) {
    case TILE_WALL:
    case TILE_BREAKABLE:
    case TILE_ITEM:
        return false;
    default:
        return true;
    }
}

uint8_t game_world::getNextPlayerNum() {
    uint8_t nums[4];
    size_t i = 0;
    for (entity *ent : entities) {
        if (ent && ent->getType() == TYPE_PLAYER) {
            player *p = dynamic_cast<player*>(ent);
            nums[i] = p->getPlayerNum();
            ++i;
        }
    }
    uint8_t num = 0;
    while(true) {
        uint8_t j;
        for (j=0; j<i; ++j) {
            if (num == nums[j]) {
                ++num;
                break;
            }
        }
        if (j==i) {
            return num;
        }
    }
}
