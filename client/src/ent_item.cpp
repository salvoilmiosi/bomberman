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
        {0,  112, 16, 16},
        {16, 112, 16, 16},
        {32, 112, 16, 16},
        {48, 112, 16, 16},
        {64,  112, 16, 16},
        {80,  112, 16, 16},
        {96,  112, 16, 16}
    };

    static const SDL_Rect src_rects_addbomb[] = {
        {0,  128, 16, 16},
        {0, 144, 16, 16}
    };

    static const SDL_Rect src_rects_addlen[] = {
        {16, 128, 16, 16},
        {16, 144, 16, 16}
    };

    static const SDL_Rect src_rects_addspeed[] = {
        {16, 160, 16, 16},
        {16, 176, 16, 16}
    };

    SDL_Rect dst_rect = world->tileRect(tx, ty);

    if (destroyed) {
        int time_since_destroy = SDL_GetTicks() - destroy_time;

        int frame = time_since_destroy * 7 / 666;
        if (frame > 6) frame = 6;

        SDL_RenderCopy(renderer, tileset_texture, src_rects_d + frame, &dst_rect);
    } else {
        int time_since_create = SDL_GetTicks() - create_time;

        int frame = (time_since_create * 2 / 100) % 2;

        const SDL_Rect *src_rects = nullptr;
        switch (item_type) {
        case ITEM_ADD_BOMB:
            src_rects = src_rects_addbomb;
            break;
        case ITEM_ADD_LENGTH:
            src_rects = src_rects_addlen;
            break;
        case ITEM_ADD_SPEED:
            src_rects = src_rects_addspeed;
            break;
        }
        if (src_rects != nullptr) {
            SDL_RenderCopy(renderer, tileset_texture, src_rects + frame, &dst_rect);
        }
    }
}

void game_item::readFromPacket(packet_ext &packet) {
    tx = packet.readChar();
    ty = packet.readChar();
    item_type = packet.readChar();
    destroyed = packet.readChar() != 0;
}
