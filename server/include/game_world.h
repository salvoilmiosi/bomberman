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
static const int COUNTDOWN_SECONDS = 4;

static const uint8_t WALK_IGNORE_BOMBS = (1 << 0);
static const uint8_t WALK_BLOCK_PLAYERS = (1 << 1);
static const uint8_t WALK_BLOCK_ITEMS = (1 << 2);

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
        packet.writeByteArray(toByteArray());
    }

    bool doSendUpdates() {
        return do_send_updates;
    }

    virtual byte_array toByteArray() = 0;
};

class game_world {
private:
    game_server server;

    std::set <entity *> entities;
    game_map g_map;

    bool round_started;
    int round_num;

    int ticks_to_start;

    uint8_t NUM_PLAYERS;

public:
    game_world(uint8_t num_players);
    virtual ~game_world();

public:
    int startServer(uint16_t port);

    void addEntity(entity *ent);
    void removeEntity(entity *ent);

    void tick();

    void writeEntitiesToPacket(packet_ext &packet, bool is_add = false);

    void writeMapToPacket(packet_ext &packet) {
        g_map.writeToPacket(packet);
    }

    void restartGame();

    void startRound();

    const game_map &getMap() {
        return g_map;
    }

    entity **findEntities(int tx, int ty, uint8_t type = 0);

    bool isWalkable(int tx, int ty, uint8_t flags = 0);

    uint8_t getNextPlayerNum();

private:
    void countdownEnd();
};

#endif // __GAME_WORLD_H__
