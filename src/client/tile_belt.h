#ifndef __TILE_BELT_H__
#define __TILE_BELT_H__

#include "game_map.h"

static const uint8_t BELT_DIR_UP = 0;
static const uint8_t BELT_DIR_DOWN = 1;
static const uint8_t BELT_DIR_LEFT = 2;
static const uint8_t BELT_DIR_RIGHT = 3;

class tile_belt: public tile_entity {
public:
    tile_belt(tile *t_tile) : tile_entity(t_tile) {}

public:
    void tick() {}

    SDL_Rect getSrcRect();
};

#endif // __TILE_BELT_H__
