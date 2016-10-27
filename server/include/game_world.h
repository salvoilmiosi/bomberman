#ifndef __GAME_WORLD_H__
#define __GAME_WORLD_H__

#include <SDL2/SDL.h>

#include <set>

#include "game_map.h"
#include "game_server.h"

enum entity_type {
    TYPE_NONE,
    TYPE_PLAYER,
    TYPE_BOMB,
    TYPE_EXPLOSION,
    TYPE_BROKEN_WALL,
    TYPE_ITEM
};

static const size_t SEARCH_SIZE = 8;
static const int COUNTDOWN_SECONDS = 4;

static const uint8_t WALK_IGNORE_BOMBS = (1 << 0);
static const uint8_t WALK_BLOCK_PLAYERS = (1 << 1);
static const uint8_t WALK_BLOCK_ITEMS = (1 << 2);

class entity {
private:
    const entity_type type;

    uint16_t id;

    bool destroyed;

    int ticks_to_start;

protected:
    bool do_send_updates;

    class game_world *world;

public:
    entity(class game_world *world, const entity_type type) : type(type), world(world) {
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

    const entity_type getType() {
        return type;
    }

    virtual const uint8_t getTileX() = 0;
    virtual const uint8_t getTileY() = 0;

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

    bool round_started = false;
    int round_num = 0;

    int ticks_to_start = 0;

    int num_users;

public:
    game_world();
    virtual ~game_world();

public:
    void clear();

    int startServer(uint16_t port);

    void addBots(int num_bots) {
        server.addBots(num_bots);
    }

    void addEntity(entity *ent);
    void removeEntity(entity *ent);

    void tick();

    void writeEntitiesToPacket(packet_ext &packet, bool is_add = false);

    void writeMapToPacket(packet_ext &packet) {
        g_map.writeToPacket(packet);
    }

    void sendKillMessage(player *killer, player *p);

    void restartGame();

    bool startRound(int num_users);

    void playWave(uint8_t sound_id) {
        server.sendSoundPacket(sound_id);
    }

    const game_map &getMap() {
        return g_map;
    }

    entity **findEntities(int tx, int ty, entity_type type = TYPE_NONE);

    class ent_movable **findMovables(float fx, float fy, entity_type type = TYPE_BOMB);

    bool isWalkable(float fx, float fy, uint8_t flags = 0);

private:
    void countdownEnd();
};

#endif // __GAME_WORLD_H__
