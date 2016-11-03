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

explosion::explosion(game_world *world, uint16_t id, byte_array &ba) : entity(world, TYPE_EXPLOSION, id) {
    readFromByteArray(ba);
    explode_time = SDL_GetTicks();
}

bomb::bomb(game_world *world, uint16_t id, byte_array &ba) : entity(world, TYPE_BOMB, id) {
    readFromByteArray(ba);
    create_time = SDL_GetTicks();
}

void bomb::tick() {
    interp.tick();
}

void bomb::render(SDL_Renderer *renderer) {
    position pos = interp.interpolate();

    fx = (pos.ix / 100.f) * TILE_SIZE;
    fy = (pos.iy / 100.f) * TILE_SIZE;
    fz = pos.iz;

    static const SDL_Rect src_rects_blue[] = {
        TILE(1, 0), TILE(2, 0), TILE(1, 0), TILE(0, 0)
    };

    static const SDL_Rect src_rects_red[] = {
        TILE(1, 1), TILE(2, 1), TILE(1, 1), TILE(0, 1)
    };

    static const SDL_Rect src_remocon_blue[] = {TILE(3, 0), TILE(4, 0)};

    static const SDL_Rect src_remocon_red[] = {TILE(3, 1), TILE(4, 1)};

    int time_since_create = SDL_GetTicks() - create_time;

    SDL_Rect dst_rect = {int(fx), int(fy), TILE_SIZE, TILE_SIZE};
    dst_rect.x += world->mapLeft();
    dst_rect.y += world->mapTop();
    dst_rect.y -= fz;

    const SDL_Rect *src_rects = src_rects_blue;
    int frame = 0;

    if (remocon) {
        frame = (time_since_create * 2 / 500) % 2;
        src_rects = red_bomb ? src_remocon_red : src_remocon_blue;
    } else {
        frame = (time_since_create * 4 / 1000) % 4;
        src_rects = red_bomb ? src_rects_red : src_rects_blue;
    }

    SDL_RenderCopy(renderer, explosions_texture, src_rects + frame, &dst_rect);
}

void explosion::render(SDL_Renderer *renderer) {
    #define EXPLOSION_ANIMATION(y) {TILE(4, y), TILE(3, y), TILE(2, y), TILE(1, y), TILE(0, y), TILE(1, y), TILE(2, y), TILE(3, y), TILE(4, y)}

    static const SDL_Rect src_rects_t[] = EXPLOSION_ANIMATION(2);
    static const SDL_Rect src_rects_v[] = EXPLOSION_ANIMATION(3);
    static const SDL_Rect src_rects_c[] = EXPLOSION_ANIMATION(4);
    static const SDL_Rect src_rects_b[] = EXPLOSION_ANIMATION(5);
    static const SDL_Rect src_rects_l[] = EXPLOSION_ANIMATION(6);
    static const SDL_Rect src_rects_h[] = EXPLOSION_ANIMATION(7);
    static const SDL_Rect src_rects_r[] = EXPLOSION_ANIMATION(8);

    int time_since_explode = SDL_GetTicks() - explode_time;
    int frame = (time_since_explode * 9 / 666) % 9;

    for (int i=len_l; i>0; --i) {
        SDL_Rect dst_rect = world->tileRect(x - i, y);
        SDL_RenderCopy(renderer, explosions_texture, (i==len_l && !trunc_l ? src_rects_l : src_rects_h) + frame, &dst_rect);
    }

    for (int i=len_t; i>0; --i) {
        SDL_Rect dst_rect = world->tileRect(x, y - i);
        SDL_RenderCopy(renderer, explosions_texture, (i==len_t && !trunc_t ? src_rects_t : src_rects_v) + frame, &dst_rect);
    }

    for (int i=len_r; i>0; --i) {
        SDL_Rect dst_rect = world->tileRect(x + i, y);
        SDL_RenderCopy(renderer, explosions_texture, (i==len_r && !trunc_r ? src_rects_r : src_rects_h) + frame, &dst_rect);
    }

    for (int i=len_b; i>0; --i) {
        SDL_Rect dst_rect = world->tileRect(x, y + i);
        SDL_RenderCopy(renderer, explosions_texture, (i==len_b && !trunc_b ? src_rects_b : src_rects_v) + frame, &dst_rect);
    }

    SDL_Rect dst_rect = world->tileRect(x, y);
    SDL_RenderCopy(renderer, explosions_texture, src_rects_c + frame, &dst_rect);
}

void bomb::readFromByteArray(byte_array &ba) {
    int ix = ba.readInt();
    int iy = ba.readInt();
    int iz = ba.readInt();
    player_id = ba.readShort();
    uint8_t flags = ba.readChar();
    red_bomb = (flags & (1 << 0)) != 0;
    remocon = (flags & (1 << 1)) != 0;

    position pos = {float(ix), float(iy), float(iz)};
    interp.addSnapshot(pos);
}

void explosion::readFromByteArray(byte_array &ba) {
    x = ba.readChar();
    y = ba.readChar();

    len_l = ba.readChar();
    len_t = ba.readChar();
    len_r = ba.readChar();
    len_b = ba.readChar();
    uint8_t trunc = ba.readChar();
    trunc_l = (trunc & (1 << 0)) != 0;
    trunc_t = (trunc & (1 << 1)) != 0;
    trunc_r = (trunc & (1 << 2)) != 0;
    trunc_b = (trunc & (1 << 3)) != 0;
}
