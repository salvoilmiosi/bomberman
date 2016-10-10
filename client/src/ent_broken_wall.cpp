#include "ent_broken_wall.h"

#include "resources.h"

broken_wall::broken_wall(game_world *world, uint16_t id, byte_array &ba) : entity(world, TYPE_BROKEN_WALL, id) {
    readFromByteArray(ba);
    create_time = SDL_GetTicks();
}

void broken_wall::render(SDL_Renderer *renderer) {
    static const SDL_Rect src_rects_1[] = {
        TILE(0, 1), TILE(1, 1), TILE(2, 1), TILE(3, 1), TILE(0, 2), TILE(1, 2)
    };

    static const SDL_Rect src_rects_2[] = {
        TILE(3, 3), TILE(4, 3), TILE(0, 4), TILE(1, 4), TILE(2, 4), TILE(3, 4),
    };

    int ticks_since_create = SDL_GetTicks() - create_time;
    int frame = ticks_since_create * 6 / 666;
    if (frame > 5) frame = 5;

    SDL_Rect dst_rect = world->tileRect(tx, ty);

    const SDL_Rect *src_rects = src_rects_1;
    SDL_Texture *tileset_texture = tileset_1_texture;
    switch(zone) {
    case 0:
        src_rects = src_rects_1;
        tileset_texture = tileset_1_texture;
        break;
    case 1:
        src_rects = src_rects_2;
        tileset_texture = tileset_2_texture;
        break;
    default:
        return;
    }

    SDL_RenderCopy(renderer, tileset_texture, src_rects + frame, &dst_rect);
}

void broken_wall::readFromByteArray(byte_array &ba) {
    tx = ba.readChar();
    ty = ba.readChar();
    zone = ba.readChar();
}
