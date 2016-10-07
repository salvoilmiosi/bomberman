#include "ent_bomb.h"

#include "resources.h"

explosion::explosion(game_world *world, uint16_t id, packet_ext &packet) : entity(world, TYPE_EXPLOSION, id) {
    readFromPacket(packet);

    explode_time = SDL_GetTicks();
}

bomb::bomb(game_world *world, uint16_t id, packet_ext &packet) : entity(world, TYPE_BOMB, id) {
    readFromPacket(packet);

    create_time = SDL_GetTicks();
}

void bomb::render(SDL_Renderer *renderer) {
    static const SDL_Rect src_rects[] = {
        {96,  0, 16, 16},
        {112, 0, 16, 16},
        {96,  0, 16, 16},
        {80,  0, 16, 16}
    };

    int time_since_create = SDL_GetTicks() - create_time;
    int frame = ((time_since_create % 1000) * 4 / 1000) % 4;

    SDL_Rect dst_rect = world->tileRect(x, y);
    SDL_RenderCopy(renderer, tileset_texture, src_rects + frame, &dst_rect);
}

void explosion::render(SDL_Renderer *renderer) {
    static const SDL_Rect src_rects_c[] = {
        {64,  64, 16, 16},
        {112, 48, 16, 16},
        {96,  48, 16, 16},
        {80,  48, 16, 16},
        {64,  48, 16, 16},
        {80,  48, 16, 16},
        {96,  48, 16, 16},
        {112, 48, 16, 16},
        {64,  64, 16, 16}
    };

    static const SDL_Rect src_rects_l[] = {
        {0,  16, 16, 16},
        {16, 16, 16, 16},
        {32, 16, 16, 16},
        {48, 16, 16, 16},
        {64, 16, 16, 16},
        {48, 16, 16, 16},
        {32, 16, 16, 16},
        {16, 16, 16, 16},
        {0,  16, 16, 16}
    };

    static const SDL_Rect src_rects_t[] = {
        {64,  32, 16, 16},
        {80,  32, 16, 16},
        {96,  32, 16, 16},
        {112, 32, 16, 16},
        {112, 16, 16, 16},
        {112, 32, 16, 16},
        {96,  32, 16, 16},
        {80,  32, 16, 16},
        {64,  32, 16, 16}
    };

    static const SDL_Rect src_rects_r[] = {
        {48,  64, 16, 16},
        {48,  48, 16, 16},
        {16,  80, 16, 16},
        {16,  64, 16, 16},
        {16,  48, 16, 16},
        {16,  64, 16, 16},
        {16,  80, 16, 16},
        {48,  48, 16, 16},
        {48,  64, 16, 16}
    };

    static const SDL_Rect src_rects_b[] = {
        {48,  32, 16, 16},
        {64,  80, 16, 16},
        {112, 80, 16, 16},
        {96,  80, 16, 16},
        {80,  80, 16, 16},
        {96,  80, 16, 16},
        {112, 80, 16, 16},
        {64,  80, 16, 16},
        {48,  32, 16, 16}
    };

    static const SDL_Rect src_rects_h[] = {
        {32, 64, 16, 16},
        {32, 48, 16, 16},
        {0,  80, 16, 16},
        {0,  64, 16, 16},
        {0,  48, 16, 16},
        {0,  64, 16, 16},
        {0,  80, 16, 16},
        {32, 48, 16, 16},
        {32, 64, 16, 16}
    };

    static const SDL_Rect src_rects_v[] = {
        {48,  80, 16, 16},
        {32,  80, 16, 16},
        {112, 64, 16, 16},
        {96,  64, 16, 16},
        {80,  64, 16, 16},
        {96,  64, 16, 16},
        {112, 64, 16, 16},
        {32,  80, 16, 16},
        {48,  80, 16, 16}
    };

    int time_since_explode = SDL_GetTicks() - explode_time;
    int frame = (time_since_explode * 9 / 1000) % 9;

    for (int i=len_l; i>0; --i) {
        SDL_Rect dst_rect = world->tileRect(x - i, y);
        SDL_RenderCopy(renderer, tileset_texture, (i==len_l && !trunc_l ? src_rects_l : src_rects_h) + frame, &dst_rect);
    }

    for (int i=len_t; i>0; --i) {
        SDL_Rect dst_rect = world->tileRect(x, y - i);
        SDL_RenderCopy(renderer, tileset_texture, (i==len_t && !trunc_t ? src_rects_t : src_rects_v) + frame, &dst_rect);
    }

    for (int i=len_r; i>0; --i) {
        SDL_Rect dst_rect = world->tileRect(x + i, y);
        SDL_RenderCopy(renderer, tileset_texture, (i==len_r && !trunc_r ? src_rects_r : src_rects_h) + frame, &dst_rect);
    }

    for (int i=len_b; i>0; --i) {
        SDL_Rect dst_rect = world->tileRect(x, y + i);
        SDL_RenderCopy(renderer, tileset_texture, (i==len_b && !trunc_b ? src_rects_b : src_rects_v) + frame, &dst_rect);
    }

    SDL_Rect dst_rect = world->tileRect(x, y);
    SDL_RenderCopy(renderer, tileset_texture, src_rects_c + frame, &dst_rect);
}

void bomb::readFromPacket(packet_ext &packet) {
    x = packet.readChar();
    y = packet.readChar();
    player_id = packet.readShort();
}

void explosion::readFromPacket(packet_ext &packet) {
    x = packet.readChar();
    y = packet.readChar();
    len_l = packet.readChar();
    len_t = packet.readChar();
    len_r = packet.readChar();
    len_b = packet.readChar();
    uint8_t trunc = packet.readChar();
    trunc_l = (trunc & (1 << 0)) != 0;
    trunc_t = (trunc & (1 << 1)) != 0;
    trunc_r = (trunc & (1 << 2)) != 0;
    trunc_b = (trunc & (1 << 3)) != 0;
    bomb_id = packet.readShort();
}
