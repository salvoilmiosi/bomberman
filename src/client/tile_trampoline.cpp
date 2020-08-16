#include "tile_trampoline.h"

#include "resources.h"

void tile_trampoline::tick() {
    if (getData() > 0) {
        if (animation_time <= 0) {
            animation_time = SDL_GetTicks();
        }
    } else {
        animation_time = 0;
    }
}

SDL_Rect tile_trampoline::getSrcRect() {
    if (animation_time > 0) {
        int frame = ((SDL_GetTicks() - animation_time) * 7 / 400) % 7;
        return TILE(frame, 5);
    } else {
        return TILE(0, 5);
    }
}
