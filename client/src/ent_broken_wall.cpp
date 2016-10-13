#include "ent_broken_wall.h"

#include "resources.h"

broken_wall::broken_wall(game_world *world, uint16_t id, byte_array &ba) : entity(world, TYPE_BROKEN_WALL, id) {
    readFromByteArray(ba);
    create_time = SDL_GetTicks();
}

void broken_wall::render(SDL_Renderer *renderer) {
    static const SDL_Rect src_rects[] = {
        TILE(0, 1), TILE(1, 1), TILE(2, 1), TILE(3, 1), TILE(0, 2), TILE(1, 2),
        TILE(3, 3), TILE(4, 3), TILE(0, 4), TILE(1, 4), TILE(2, 4), TILE(3, 4),
        TILE(0, 3), TILE(1, 3), TILE(2, 3), TILE(3, 3), TILE(4, 3), TILE(5, 3),
        TILE(0, 4), TILE(1, 4), TILE(2, 4), TILE(3, 4), TILE(4, 4), TILE(5, 4),
    };

    int ticks_since_create = SDL_GetTicks() - create_time;
    int frame = ticks_since_create * 6 / 666;
    if (frame > 5) frame = 5;

    SDL_Rect dst_rect = world->tileRect(tx, ty);

    SDL_Texture *tilesets[] = {
        tileset_1_texture, tileset_2_texture, tileset_3_texture, tileset_4_texture
    };

    const SDL_Rect *start_rect = src_rects + zone * 6;
    SDL_RenderCopy(renderer, tilesets[zone], start_rect + frame, &dst_rect);
}

void broken_wall::readFromByteArray(byte_array &ba) {
    tx = ba.readChar();
    ty = ba.readChar();
    zone = ba.readChar();
}
