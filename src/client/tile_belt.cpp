#include "tile_belt.h"

#include "resources.h"
#include "game_world.h"

#include <map>
#include <vector>

SDL_Rect tile_belt::getSrcRect() {
    uint16_t data = getData();
    direction belt_direction = static_cast<direction>((data & 0xc00) >> 10);
    bool is_breakable = (data & 0x1000) != 0;

    SDL_Rect src_rect;
    if (is_breakable) {
        static const SDL_Rect belt_breakables[] = {
            TILE(3, 0), TILE(4, 0), TILE(5, 0), TILE(6, 0),
            TILE(3, 1), TILE(4, 1), TILE(5, 1), TILE(6, 1),
            TILE(3, 2), TILE(4, 2), TILE(5, 2), TILE(6, 2),
        };

        int frame = (SDL_GetTicks() * 12 / 2000) % 12;
        src_rect = belt_breakables[frame];
    } else {
        int frame = (SDL_GetTicks() * 4 / 500) % 4;
        static const std::map<direction, std::vector<SDL_Rect> > belt_rects = {
            {DIR_UP, {TILE(0, 3), TILE(1, 3), TILE(2, 3), TILE(3, 3)}},
            {DIR_DOWN, {TILE(4, 3), TILE(5, 3), TILE(6, 3), TILE(7, 3)}},
            {DIR_LEFT, {TILE(0, 4), TILE(1, 4), TILE(2, 4), TILE(3, 4)}},
            {DIR_RIGHT, {TILE(4, 4), TILE(5, 4), TILE(6, 4), TILE(7, 4)}},
        };
        src_rect = belt_rects.at(belt_direction)[frame];
    }
    return src_rect;
}
