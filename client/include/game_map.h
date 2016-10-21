#ifndef __GAME_MAP_H__
#define __GAME_MAP_H__

#include "packet_io.h"

#include <SDL2/SDL.h>
#include <map>

static const int TILE_SIZE = 48;

static const uint8_t TILE_FLOOR = 0;
static const uint8_t TILE_WALL = 2;
static const uint8_t TILE_BREAKABLE = 3;
static const uint8_t TILE_SPECIAL = 5;

static const uint8_t ZONE_NORMAL = 0;
static const uint8_t ZONE_WESTERN = 1;
static const uint8_t ZONE_BOMB = 2;
static const uint8_t ZONE_JUMP = 3;

static const uint8_t SPECIAL_TRAMPOLINE = 1;

struct tile {
    uint8_t type;
    uint8_t data;
};

class tile_entity {
private:
    uint8_t type;

    tile *t_tile;

    bool used = false;

    friend class game_map;

public:
    tile_entity(tile *t_tile);

    virtual ~tile_entity() {}

    virtual void tick() = 0;

    virtual SDL_Rect getSrcRect() = 0;

    static tile_entity *newTileEntity(tile *t_tile);

protected:
    uint8_t getData() {
        return t_tile->data & 0x1f;
    }
};

class game_map {
private:
    tile *tiles = nullptr;

    int width = 0;
    int height = 0;
    int zone = 0;

    std::map<tile *, tile_entity *> specials;

public:
    game_map();
    virtual ~game_map();

public:
    void clear();

    tile *getTile(int x, int y);

    int getTileX(tile *t);
    int getTileY(tile *t);

    void tick();

    void render(SDL_Renderer *renderer);

    void readFromByteArray(byte_array &packet);

    int left();
    int top();
};

#endif // __GAME_MAP_H__
