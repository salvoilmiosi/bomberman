#include "game_world.h"

#include "player.h"

#include <cstring>

game_world::game_world() : server(this) {
    game_started = false;
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

void game_world::startRound(int num_players) {
    if (game_started) return;

    g_map.createMap(MAP_WIDTH, MAP_HEIGHT, num_players);

    int num = 0;

    for (entity *ent : entities) {
        if (ent && ent->isNotDestroyed() && ent->getType() == TYPE_PLAYER) {
            player *p = (player *)ent;
            point spawn_pt = g_map.getSpawnPt(num);
            p->respawn(spawn_pt.x * TILE_SIZE, spawn_pt.y * TILE_SIZE);
            ++num;
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
