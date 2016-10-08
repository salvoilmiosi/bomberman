#ifndef __GAME_WORLD_H__
#define __GAME_WORLD_H__

#include <SDL2/SDL.h>

#include <set>

#include "game_map.h"
#include "game_server.h"

static const uint8_t TYPE_PLAYER = 1;
static const uint8_t TYPE_BOMB = 2;
static const uint8_t TYPE_EXPLOSION = 3;
static const uint8_t TYPE_BROKEN_WALL = 4;
static const uint8_t TYPE_ITEM = 5;

static const size_t SEARCH_SIZE = 8;

class entity {
private:
    const uint8_t type;

    uint16_t id;

    bool destroyed;

    int ticks_to_start;

protected:
    bool do_send_updates;

    class game_world *world;

public:
    entity(class game_world *world, const uint8_t type) : type(type), world(world) {
        static uint16_t max_id = 1;
        id = max_id++;
        destroyed = false;
        do_send_updates = true;
    }

    virtual ~entity() {};

    virtual void tick() = 0;

    uint16_t getID() {
        return id;
    }

    uint8_t getType() {
        return type;
    }

    virtual uint8_t getTileX() const = 0;
    virtual uint8_t getTileY() const = 0;

    bool isNotDestroyed() {
        return !destroyed;
    }

    void destroy() {
        destroyed = true;
    }

    void writeToPacket(packet_ext &packet) {
        packet.writeShort(getID());
        packet.writeChar(getType());
        writeEntity(packet);
    }

    bool doSendUpdates() {
        return do_send_updates;
    }

private:
    virtual void writeEntity(packet_ext &packet) = 0;
};

class game_world {
private:
    game_server server;

    std::set <entity *> entities;
    game_map g_map;

    bool game_started;
    bool round_started;
    int round_num;

    int num_players;
    int alive_players;

    int ticks_to_start;

public:
    game_world();
    virtual ~game_world();

public:
    int startServer();

    void addEntity(entity *ent);
    void removeEntity(entity *ent);

    void tick();

    void playerDied(player *dead);

    void writeEntitiesToPacket(packet_ext &packet, bool is_add = false);

    void writeMapToPacket(packet_ext &packet) {
        g_map.writeToPacket(packet);
    }

    void startRound(int num_players);

    const game_map &getMap() {
        return g_map;
    }

    bool gameHasStarted() {
        return game_started;
    }

    entity **findEntities(int tx, int ty, uint8_t type = 0);

    bool isWalkable(int tx, int ty, bool ignore_bombs = false);

private:
    void countdownEnd();
};

#endif // __GAME_WORLD_H__
