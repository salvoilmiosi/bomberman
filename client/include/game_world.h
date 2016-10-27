#ifndef __GAME_WORLD_H__
#define __GAME_WORLD_H__

#include "game_map.h"

#include <map>

enum entity_type {
    TYPE_NONE,
    TYPE_PLAYER,
    TYPE_BOMB,
    TYPE_EXPLOSION,
    TYPE_BROKEN_WALL,
    TYPE_ITEM
};

enum direction {
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT,
};

class entity {
private:
    const entity_type ent_type;
    const uint16_t id;

    friend class game_world;

protected:
    class game_world *world;

public:
    entity(class game_world *world, const entity_type type, uint16_t id) : ent_type(type), id(id), world(world) {}

    virtual ~entity() {};

    virtual void tick() = 0;

    virtual void render(SDL_Renderer *renderer) = 0;

    static entity *newObjFromByteArray(class game_world *world, uint16_t id, entity_type type, byte_array &ba);

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
    void clear();

    void addEntity(entity *ent);
    void removeEntity(entity *ent);

    void render(SDL_Renderer *renderer);

    void tick();

    entity *findID(uint16_t id);

    void handleMapPacket(byte_array &packet) {
        g_map.readFromByteArray(packet);
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
