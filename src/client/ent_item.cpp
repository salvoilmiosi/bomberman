#include "ent_item.h"

#include "resources.h"

game_item::game_item(game_world *world, uint16_t id, byte_array &ba) : entity(world, TYPE_ITEM, id) {
    readFromByteArray(ba);
    create_time = SDL_GetTicks();
    destroy_time = 0;
}

void game_item::tick() {
    if (exploded) {
        if (destroy_time == 0) {
            destroy_time = SDL_GetTicks();
        }
    }
}

void game_item::render(SDL_Renderer *renderer) {
    static const SDL_Rect src_rects_d[] = {
        TILE(0, 6), TILE(1, 6), TILE(2, 6), TILE(3, 6), TILE(4, 6), TILE(5, 6), TILE(6, 6)
    };

    static const SDL_Rect item_rects[] = {
        {0,0,0,0}, TILE(0, 0), TILE(1, 0), TILE(1, 2), TILE(2, 2),
        TILE(4, 2), TILE(5, 0), TILE(3, 0), TILE(2, 0), TILE(7, 0)
    };

    SDL_Rect dst_rect = world->tileRect(tx, ty);

    if (exploded) {
        int time_since_destroy = SDL_GetTicks() - destroy_time;

        int frame = time_since_destroy * 7 / 666;
        if (frame > 6) frame = 6;

        SDL_RenderCopy(renderer, items_texture, src_rects_d + frame, &dst_rect);
    } else if (item_type > 0) {
        int time_since_create = SDL_GetTicks() - create_time;

        int frame = (time_since_create * 2 / 100) % 2;

        SDL_Rect src_rect = item_rects[item_type];
        src_rect.y += frame * 16;

        SDL_RenderCopy(renderer, items_texture, &src_rect, &dst_rect);
    }
}

void game_item::readFromByteArray(byte_array &ba) {
    tx = ba.readChar();
    ty = ba.readChar();
    item_type = ba.readChar();
    exploded = ba.readChar() != 0;
}
