#include "ent_item.h"

#include "resources.h"

game_item::game_item(game_world *world, uint16_t id, packet_ext &packet) : entity(world, TYPE_ITEM, id) {
    readFromPacket(packet);

    create_time = SDL_GetTicks();
    destroy_time = 0;
}

void game_item::tick() {
    if (destroyed) {
        if (destroy_time == 0) {
            destroy_time = SDL_GetTicks();
        }
    }
}

void game_item::render(SDL_Renderer *renderer) {
    static const SDL_Rect src_rects_d[] = {
        TILE(0, 7), TILE(1, 7), TILE(2, 7), TILE(3, 7), TILE(4, 7), TILE(5, 7), TILE(6, 7)
    };

    static const SDL_Rect item_rects[] = {
        {0,0,0,0}, TILE(0, 0), TILE(1, 0), TILE(1, 2), TILE(2, 2), TILE(4, 2), TILE(5, 0)
    };

    SDL_Rect dst_rect = world->tileRect(tx, ty);

    if (destroyed) {
        int time_since_destroy = SDL_GetTicks() - destroy_time;

        int frame = time_since_destroy * 7 / 666;
        if (frame > 6) frame = 6;

        SDL_RenderCopy(renderer, tileset_texture, src_rects_d + frame, &dst_rect);
    } else if (item_type > 0) {
        int time_since_create = SDL_GetTicks() - create_time;

        int frame = (time_since_create * 2 / 100) % 2;

        SDL_Rect src_rect = item_rects[item_type];
        src_rect.y += frame * 16;

        SDL_RenderCopy(renderer, items_texture, &src_rect, &dst_rect);
    }
}

void game_item::readFromPacket(packet_ext &packet) {
    tx = packet.readChar();
    ty = packet.readChar();
    item_type = packet.readChar();
    destroyed = packet.readChar() != 0;
}
