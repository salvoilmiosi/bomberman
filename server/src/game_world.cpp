#include "game_world.h"

#include "player.h"

#include <cstring>

game_world::game_world() : server(this) {
    game_started = false;
    round_started = false;
    round_num = 0;
    alive_players = 0;
    ticks_to_start = 0;
}

game_world::~game_world() {
	for (auto ent : entities) {
		if (ent) delete ent;
	}
}

int game_world::startServer() {
    if (server.isOpen()) {
        printf("Server listening on port %d\n", PORT);
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
    if (game_started) {
        if (ticks_to_start == 0) {
            server.messageToAll("Good luck, have fun!");
            countdownEnd();
        }
        if (ticks_to_start > 0 && ticks_to_start % TICKRATE == 0) {
            int seconds = ticks_to_start / TICKRATE;
            server.messageToAll("Round %d begins in %d seconds", round_num, seconds);
        }
        --ticks_to_start;
    }
    auto it = entities.begin();
    while (it != entities.end()) {
        entity *ent = *it;
        if (ent && ent->isNotDestroyed()) {
            ent->tick();
            ++it;
        } else {
            it = entities.erase(it);
            removeEntity(ent);
            delete ent;
        }
    }
}

void game_world::writeEntitiesToPacket(packet_ext &packet, bool is_add) {
    packet.writeShort(entities.size());
    for(entity *ent : entities) {
        if (ent && ent->isNotDestroyed() && (is_add || ent->doSendUpdates())) {
            ent->writeToPacket(packet);
        }
    }
}

void game_world::playerDied(player *dead) {
    if (dead->isAlive()) return;
    if (!round_started) return;

    --alive_players;

    if (alive_players <= 1) {
        if (alive_players == 0) {
            server.messageToAll("DRAW!");
        }
        for (entity *ent : entities) {
            if (ent && ent->isNotDestroyed() && ent->getType() == TYPE_PLAYER) {
                player *p = dynamic_cast<player*>(ent);
                if (p->isAlive()) {
                    server.messageToAll("%s won round %d!", p->getName(), round_num);
                    break;
                }
            }
        }
        round_started = false;
        startRound(num_players);
    }
}

void game_world::startRound(int numplayers) {
    if (round_started) return;
    if (ticks_to_start > 0) return;
    game_started = true;

    ++round_num;
    alive_players = 0;

    num_players = numplayers;
    ticks_to_start = TICKRATE * 4;
}

void game_world::countdownEnd() {
    g_map.createMap(MAP_WIDTH, MAP_HEIGHT, num_players);
    round_started = true;

    for (entity *ent : entities) {
        if (ent && ent->isNotDestroyed()) {
            switch (ent->getType()) {
            case TYPE_PLAYER:
                {
                    player *p = (player *)ent;
                    point spawn_pt = g_map.getSpawnPt(p->getPlayerNum());
                    p->respawn(spawn_pt.x * TILE_SIZE, spawn_pt.y * TILE_SIZE);
                    ++alive_players;
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

bool game_world::isWalkable(int tx, int ty, bool ignore_bombs) {
    tile *t = getMap().getTile(tx, ty);
    if (!t) return false;

    entity **ents = findEntities(tx, ty);
    for (uint8_t i = 0; i < SEARCH_SIZE; ++i) {
        entity *ent = ents[i];
        if (!ent) break;
        switch (ent->getType()) {
        case TYPE_BOMB:
            if (ignore_bombs) {
                break;
            }
        case TYPE_BROKEN_WALL:
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
