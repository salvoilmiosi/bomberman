#ifndef __GAME_MAP_H__
#define __GAME_MAP_H__

#include "packet_io.h"

#include <SDL2/SDL.h>

static const int TILE_SIZE = 48;

struct tile {
    enum tile_type {
        TILE_FLOOR = 0, TILE_SPAWN, TILE_WALL, TILE_BREAKABLE, TILE_ITEM,
    };
    tile_type type;
};

class game_map {
private:
    tile *tiles = nullptr;

    int width = 0;
    int height = 0;

public:
    game_map();
    virtual ~game_map();

public:
    tile *getTile(int x, int y);

    void render(SDL_Renderer *renderer);

    void readFromPacket(packet_ext &packet);

    int left();
    int top();
};

#endif // __GAME_MAP_H__
