#include "player.h"

#include <cstring>

player::player(Uint16 id, packet_ext &packet) : game_obj(TYPE_PLAYER, id) {
    is_self = false;
    readFromPacket(packet);

    death_animation = false;

    damage_time = 0xffffffff;
    heal_time   = 0xffffffff;
}

void player::tick() {
    interp.tick();

    interp.interpolate(&x, &position::ix);
    interp.interpolate(&y, &position::iy);

    if (health_points <= 0 && !death_animation) {
        death_animation = true;
        death_time = SDL_GetTicks();
    }

    if (death_animation && health_points > 0) {
        death_animation = false;
    }
}

void player::render(SDL_Renderer *renderer) {
    int squaresize = 30;
    if (death_animation) {
        float x = (SDL_GetTicks() - death_time) / 50.f;
        float a = 5.f;
        squaresize = squaresize + a*a - (x - a) * (x - a);
    }

    int rr = r;
    int gg = g;
    int bb = b;

    if (SDL_GetTicks() > damage_time) {
        float x = 1.f - (SDL_GetTicks() - damage_time) / 200.f;
        if (x > 0) {
            rr += (255.f - rr) * x;
            gg += (-gg) * x;
            bb += (-bb) * x;
        } else {
            damage_time = 0xffffffff;
        }
    }

    if (SDL_GetTicks() > heal_time) {
        float x = 1.f - (SDL_GetTicks() - heal_time) / 500.f;
        if (x > 0) {
            rr += (-rr) * x;
            gg += (-gg) * x;
            bb += (255.f - bb) * x;
        } else {
            heal_time = 0xffffffff;
        }
    }

    if (squaresize > 0) {
        if (is_self) {
            SDL_Rect rect = {(int)x - squaresize / 2 - 2, (int)y - squaresize / 2 - 2, squaresize + 4, squaresize + 4};
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            SDL_RenderFillRect(renderer, &rect);
        }
        SDL_Rect rect = {(int)x - squaresize / 2, (int)y - squaresize / 2, squaresize, squaresize};
        SDL_SetRenderDrawColor(renderer, rr, gg, bb, 255);
        SDL_RenderFillRect(renderer, &rect);
    }
}

void player::setName(const char *name) {
    strncpy(player_name, name, NAME_SIZE);
}

const char *player::getName() {
    return player_name;
}

void player::readFromPacket(packet_ext &packet) {
    int xx = packet.readInt();
    int yy = packet.readInt();
    position pos = {(float)xx, (float)yy};
    interp.addSnapshot(pos);

    setName(packet.readString());
    Uint32 col = packet.readInt();
    r = (col & 0xff000000) >> 24;
    g = (col & 0x00ff0000) >> 16;
    b = (col & 0x0000ff00) >> 8;

    short new_hp = packet.readShort();
    if (new_hp < health_points) {
        damage_time = SDL_GetTicks();
    } else if (new_hp > health_points) {
        heal_time = SDL_GetTicks();
    }
    health_points = new_hp;
}
