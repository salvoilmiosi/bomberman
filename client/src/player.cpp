#include "player.h"

#include <cstring>

#include "resources.h"
#include "main.h"
#include "chat.h"

player::position operator + (const player::position &a, const player::position &b) {
    player::position pos = {a.ix + b.ix, a.iy + b.iy, a.iz + b.iz, a.imoving + b.imoving, a.idirection + b.idirection, a.ipunching + b.ipunching};
    return pos;
}

player::position operator - (const player::position &a, const player::position &b) {
    player::position pos = {a.ix - b.ix, a.iy - b.iy, a.iz - b.iz, a.imoving - b.imoving, a.idirection - b.idirection, a.ipunching - b.ipunching};
    return pos;
}

player::position operator * (const float &f, const player::position &p) {
    player::position pos = {p.ix * f, p.iy * f, p.iz * f, p.imoving * f, p.idirection * f, p.ipunching * f};
    return pos;
}

player::player(game_world *world, uint16_t id, byte_array &ba) : entity(world, TYPE_PLAYER, id) {
    readFromByteArray(ba);
    is_self = false;
}

void player::tick() {
    position ip = interp.interpolate();

    interp.tick();

    x = (ip.ix / 100.f + 0.5f) * TILE_SIZE;
    y = (ip.iy / 100.f + 0.5f) * TILE_SIZE;
    z = ip.iz;

    moving = ip.imoving > 0.5f;
    direction = ip.idirection + 0.5f;
    punching = ip.ipunching > 0.5f;

    if (spawned && !alive) {
        if (death_time == 0) {
            death_time = SDL_GetTicks();
        }
    } else {
        death_time = 0;
    }
}

static inline SDL_Rect getSprite(int x, int y, int player_num) {
    switch(player_num) {
    case 0:
        break;
    case 1:
        x += 6;
        break;
    case 2:
        x += 6;
        y += 4;
        break;
    case 3:
        y += 4;
        break;
    }

    if (y > 6) {
        y -= 4;
        x += 3;
    }

    SDL_Rect rect = {x * 16, y * 24, 16, 24};
    return rect;
}

void player::render(SDL_Renderer *renderer) {
    if (!spawned) return;

    int spx = 0;
    int spy = 0;

    SDL_RendererFlip flip = SDL_FLIP_NONE;

    static const int8_t walk_frames[4] = {1, 0, -1, 0};

    if (alive) {
        if (punching) {
            spy = 3;
            switch(direction) {
            case 0:
                spx = 1;
                break;
            case 1:
                break;
            case 2:
                spx = 2;
                flip = SDL_FLIP_HORIZONTAL;
                break;
            case 3:
                spx = 2;
                break;
            }
        } else {
            if (moving) {
                int frame = ((SDL_GetTicks() % 666) * 4 / 666) % 4;
                spx = walk_frames[frame];
            }
            switch(direction) {
            case 0:
                spx += 1;
                break;
            case 1:
                spx += 4;
                break;
            case 2:
                spx += 1;
                spy += 1;
                break;
            case 3:
                spx += 4;
                spy += 1;
                break;
            }
        }
    } else {
        int time_since_death = SDL_GetTicks() - death_time;
        spx = time_since_death * 6 / 1000;
        if (spx > 5) spx = 5;
        spy = 2;
    }

    SDL_Rect src_rect = getSprite(spx, spy, player_num);

    SDL_Rect dst_rect = {0, 0, 48, 72};

    dst_rect.x = world->mapLeft() + (int)x - TILE_SIZE / 2;
    dst_rect.y = world->mapTop() + (int)y - TILE_SIZE;
    dst_rect.y -= (int) z;

    bool render_black = false;
    if (diseased) {
        int frame = (SDL_GetTicks() * 3 / 1000) % 2;
        render_black = frame == 0;
    }

    if (alive && render_black) {
        SDL_SetTextureColorMod(players_texture, 0, 0, 0);
    } else {
        SDL_SetTextureColorMod(players_texture, 0xff, 0xff, 0xff);
    }

    SDL_RenderCopyEx(renderer, players_texture, &src_rect, &dst_rect, 0, nullptr, flip);

    int name_x = dst_rect.x + (dst_rect.w - (int)strlen(player_name) * CHAR_W) / 2;
    int name_y = dst_rect.y - CHAR_H;
    renderText(name_x, name_y, renderer, player_name, is_self ? 0xffff00ff : 0xffffffff);
}

void player::setName(const char *name) {
    strncpy(player_name, name, NAME_SIZE);
}

const char *player::getName() {
    return player_name;
}

void player::readFromByteArray(byte_array &ba) {
    int ix = ba.readInt();
    int iy = ba.readInt();
    int iz = ba.readInt();

    setName(ba.readString());

    player_num = ba.readChar();

    uint16_t flags = ba.readShort();

    alive = (flags & (1 << 0)) != 0;
    spawned = (flags & (1 << 1)) != 0;

    bool imoving = (flags & (1 << 2)) != 0;
    bool ipunching = (flags & (1 << 3)) != 0;

    diseased = (flags & (1 << 4)) != 0;

    //uint8_t ijumping = (flags & (1 << 5)) != 0;

    uint8_t idirection = (flags & 0xc000) >> 14;

    position pos = {float(ix), float(iy), float(iz), float(imoving), float(idirection), float(ipunching)};
    interp.addSnapshot(pos);
}
