#ifndef __GAME_WORLD_H__
#define __GAME_WORLD_H__

#include "game_map.h"

#include <map>

static const int TYPE_PLAYER = 1;
static const int TYPE_BOMB = 2;
static const int TYPE_EXPLOSION = 3;
static const int TYPE_BROKEN_WALL = 4;
static const int TYPE_ITEM = 5;

class entity {
private:
    const unsigned char ent_type;
    const uint16_t id;

protected:
    class game_world *world;

public:
    entity(class game_world *world, const unsigned char type, uint16_t id) : ent_type(type), id(id), world(world) {}

    virtual ~entity() {};

    virtual void tick() = 0;

    virtual void render(SDL_Renderer *renderer) = 0;

    static entity *newObjFromPacket(class game_world *world, uint16_t id, packet_ext &packet);

    uint16_t getID() {
        return id;
    }

    char getType() {
        return ent_type;
    }

    virtual void readFromByteArray(byte_array &ba) = 0;
};

class game_world {
private:
    std::map <uint16_t, entity *, std::greater<uint16_t> > entities;

    game_map g_map;

public:
    game_world();
    virtual ~game_world();

public:
    void addEntity(entity *ent);
    void removeEntity(entity *ent);

    void render(SDL_Renderer *renderer);

    void tick();

    entity *findID(uint16_t id);

    void handleMapPacket(packet_ext &packet) {
        g_map.readFromPacket(packet);
    }

    int mapLeft() {
        return g_map.left();
    }

    int mapTop() {
        return g_map.top();
    }

    SDL_Rect tileRect(int x, int y);
};


#endif // __GAME_WORLD_H__
