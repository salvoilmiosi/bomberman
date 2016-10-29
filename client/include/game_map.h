#ifndef __GAME_MAP_H__
#define __GAME_MAP_H__

#include "packet_io.h"

#include <SDL2/SDL.h>
#include <map>

static const int TILE_SIZE = 48;

enum tile_type {
    TILE_FLOOR,
    TILE_SPAWN,
    TILE_WALL,
    TILE_BREAKABLE,
    TILE_SPECIAL,
};

enum map_zone {
    ZONE_RANDOM,
    ZONE_NORMAL,
    ZONE_WESTERN,
    ZONE_BOMB,
    ZONE_JUMP,
    ZONE_BELT,
    ZONE_DUEL,
    ZONE_POWER,
    ZONE_SPEED,
};

enum special_type {
    SPECIAL_NONE,
    SPECIAL_TRAMPOLINE,
    SPECIAL_BELT,
    SPECIAL_ITEM_SPAWNER,
};

struct tile {
    tile_type type;
    uint16_t data;
};

class tile_entity {
private:
    special_type type;

    tile *t_tile;

    bool used = false;

    friend class game_map;

public:
    tile_entity(tile *t_tile);

    virtual ~tile_entity() {}

    virtual void tick() = 0;

    virtual SDL_Rect getSrcRect() = 0;

    static tile_entity *newTileEntity(tile *t_tile);

    static special_type getSpecialType(uint16_t data) {
        return static_cast<special_type>((data & 0xe000) >> 13);
    }

protected:
    uint16_t getData() {
        return t_tile->data & 0x1fff;
    }
};

class game_map {
private:
    tile *tiles = nullptr;

    int width = 0;
    int height = 0;

    map_zone zone;

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

    static SDL_Texture *getTileset(map_zone zone);

    void readFromByteArray(byte_array &packet);

    int left();
    int top();
};

#endif // __GAME_MAP_H__
