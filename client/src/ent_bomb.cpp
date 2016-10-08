#include "ent_bomb.h"

#include "resources.h"

bomb::position operator + (const bomb::position &a, const bomb::position &b) {
    bomb::position pos = {a.ix + b.ix, a.iy + b.iy, a.iz + b.iz};
    return pos;
}

bomb::position operator - (const bomb::position &a, const bomb::position &b) {
    bomb::position pos = {a.ix - b.ix, a.iy - b.iy, a.iz - b.iz};
    return pos;
}

bomb::position operator * (const float &f, const bomb::position &p) {
    bomb::position pos = {f * p.ix, f * p.iy, f * p.iz};
    return pos;
}

explosion::explosion(game_world *world, uint16_t id, packet_ext &packet) : entity(world, TYPE_EXPLOSION, id) {
    readFromPacket(packet);

    explode_time = SDL_GetTicks();
}

bomb::bomb(game_world *world, uint16_t id, packet_ext &packet) : entity(world, TYPE_BOMB, id) {
    readFromPacket(packet);

    create_time = SDL_GetTicks();
}

void bomb::tick() {
    interp.tick();

    position pos = interp.interpolate();

    fx = (pos.ix / 100.f) * TILE_SIZE;
    fy = (pos.iy / 100.f) * TILE_SIZE;
    fz = pos.iz;
}

void bomb::render(SDL_Renderer *renderer) {
    static const SDL_Rect src_rects[] = {
        TILE(6, 0), TILE(7, 0), TILE(6, 0), TILE(5, 0)
    };

    int time_since_create = SDL_GetTicks() - create_time;
    int frame = ((time_since_create % 1000) * 4 / 1000) % 4;

    SDL_Rect dst_rect = {int(fx), int(fy), TILE_SIZE, TILE_SIZE};
    dst_rect.x += world->mapLeft();
    dst_rect.y += world->mapTop();
    dst_rect.y -= fz;

    SDL_RenderCopy(renderer, tileset_texture, src_rects + frame, &dst_rect);
}

void explosion::render(SDL_Renderer *renderer) {
    static const SDL_Rect src_rects_c[] = {
        TILE(4, 4), TILE(7, 3), TILE(6, 3), TILE(5, 3), TILE(4, 3),
        TILE(5, 3), TILE(6, 3), TILE(7, 3), TILE(4, 4)
    };

    static const SDL_Rect src_rects_l[] = {
        TILE(0, 1), TILE(1, 1), TILE(2, 1), TILE(3, 1), TILE(4, 1),
        TILE(3, 1), TILE(2, 1), TILE(1, 1), TILE(0, 1)
    };

    static const SDL_Rect src_rects_t[] = {
        TILE(4, 2), TILE(5, 2), TILE(6, 2), TILE(7, 2), TILE(7, 1),
        TILE(7, 2), TILE(6, 2), TILE(5, 2), TILE(4, 2)
    };

    static const SDL_Rect src_rects_r[] = {
        TILE(3, 4), TILE(3, 3), TILE(1, 5), TILE(1, 4), TILE(1, 3),
        TILE(1, 4), TILE(1, 5), TILE(3, 3), TILE(3, 4)
    };

    static const SDL_Rect src_rects_b[] = {
        TILE(3, 2), TILE(4, 5), TILE(7, 5), TILE(6, 5), TILE(5, 5),
        TILE(6, 5), TILE(7, 5), TILE(4, 5), TILE(3, 2)
    };

    static const SDL_Rect src_rects_h[] = {
        TILE(2, 4), TILE(2, 3), TILE(0, 5), TILE(0, 4), TILE(0, 3),
        TILE(0, 4), TILE(0, 5), TILE(2, 3), TILE(2, 4)
    };

    static const SDL_Rect src_rects_v[] = {
        TILE(3, 5), TILE(2, 5), TILE(7, 4), TILE(6, 4), TILE(5, 4),
        TILE(6, 4), TILE(7, 4), TILE(2, 5), TILE(3,5)
    };

    int time_since_explode = SDL_GetTicks() - explode_time;
    int frame = (time_since_explode * 9 / 666) % 9;

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
    int ix = packet.readInt();
    int iy = packet.readInt();
    int iz = packet.readInt();
    player_id = packet.readShort();

    position pos = {float(ix), float(iy), float(iz)};
    interp.addSnapshot(pos);
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
