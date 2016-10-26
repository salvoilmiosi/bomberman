#ifndef __TILE_TRAMPOLINE_H__
#define __TILE_TRAMPOLINE_H__

#include "game_map.h"

static const int TRAMPOLINE_JUMP_TICKS = TICKRATE * 5 / 2;

class tile_trampoline : public tile_entity {
private:
    int jump_ticks = 0;

public:
    tile_trampoline(tile *t_tile, game_map *g_map) : tile_entity(SPECIAL_TRAMPOLINE, t_tile, g_map) {
        setData(0);
    }

    void tick() {
        setData(jump_ticks > 0);
        if (jump_ticks > 0) {
            --jump_ticks;
        }
    }

    bool jump() {
        if (jump_ticks <= 0) {
            jump_ticks = TRAMPOLINE_JUMP_TICKS;
            return true;
        }
        return false;
    }

    bool isWalkable() {
        return true;
    }

    bool bombHit() {
        return false;
    }
};

#endif // __TILE_TRAMPOLINE_H__
