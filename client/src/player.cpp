#include "player.h"

#include <cstring>

#include "resources.h"

player::player(game_world *world, uint16_t id, packet_ext &packet) : entity(world, TYPE_PLAYER, id) {
    is_self = false;
    readFromPacket(packet);
}

void player::tick() {
    interp.tick();

    x = (interp.interpolate(&position::ix) / 100.f + 0.5f) * TILE_SIZE;
    y = (interp.interpolate(&position::iy) / 100.f + 0.5f) * TILE_SIZE;

    moving = interp.interpolate(&position::imoving) > 0.5f;
    direction = interp.interpolate(&position::idirection) + 0.5f;

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
        y += 4;
        break;
    case 3:
        x += 6;
        y += 4;
        break;
    }

    if (y > 6) {
        y -= 4;
        x += 4;
    }

    SDL_Rect rect = {x * 16, y * 24, 16, 24};
    return rect;
}

void player::render(SDL_Renderer *renderer) {
    if (!spawned) return;

    int spx = 0;
    int spy = 0;

    if (alive) {
        if (moving) {
            int frame = ((SDL_GetTicks() % 1000) * 4 / 1000) % 4;
            switch (frame) {
            case 0:
                spx = 1;
                break;
            case 1:
                spx = 0;
                break;
            case 2:
                spx = -1;
                break;
            case 3:
                spx = 0;
                break;
            }
        }
        switch(direction) {
        case 0: // up
            spx += 1;
            break;
        case 1: // down
            spx += 4;
            break;
        case 2: // left
            spx += 1;
            spy += 1;
            break;
        case 3: // right
            spx += 4;
            spy += 1;
            break;
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

    SDL_RenderCopy(renderer, players_texture, &src_rect, &dst_rect);
}

void player::setName(const char *name) {
    strncpy(player_name, name, NAME_SIZE);
}

const char *player::getName() {
    return player_name;
}

void player::readFromPacket(packet_ext &packet) {
    int ix = packet.readInt();
    int iy = packet.readInt();

    setName(packet.readString());

    player_num = packet.readChar();

    uint8_t flags = packet.readChar();

    alive = (flags & (1 << 0)) != 0;
    spawned = (flags & (1 << 1)) != 0;

    bool imoving = (flags & (1 << 2)) != 0;
    uint8_t idirection = packet.readChar();

    position pos = {float(ix), float(iy), float(imoving), float(idirection)};
    interp.addSnapshot(pos);
}
