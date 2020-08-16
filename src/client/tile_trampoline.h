#ifndef __TILE_TRAMPOLINE_H__
#define __TILE_TRAMPOLINE_H__

#include "game_map.h"

class tile_trampoline : public tile_entity {
private:
    int animation_time = 0;

public:
    tile_trampoline(tile *t_tile) : tile_entity(t_tile) {
    }

public:
    void tick();

    SDL_Rect getSrcRect();
};

#endif // __TILE_TRAMPOLINE_H__
