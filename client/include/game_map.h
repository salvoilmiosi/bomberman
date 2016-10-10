#ifndef __GAME_MAP_H__
#define __GAME_MAP_H__

#include "packet_io.h"

#include <SDL2/SDL.h>

static const int TILE_SIZE = 48;

static const uint8_t TILE_FLOOR = 0;
static const uint8_t TILE_SPAWN = 1;
static const uint8_t TILE_WALL = 2;
static const uint8_t TILE_BREAKABLE = 3;

struct tile {
    uint8_t type;
    uint8_t data;
};

class game_map {
private:
    tile *tiles = nullptr;

    int width = 0;
    int height = 0;
    int zone = 0;

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
