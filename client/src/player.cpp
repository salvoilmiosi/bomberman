#include "player.h"

#include <cstring>

player::player(game_world *world, uint16_t id, packet_ext &packet) : entity(world, TYPE_PLAYER, id) {
    is_self = false;
    readFromPacket(packet);
}

void player::tick() {
    interp.tick();

    x = (interp.interpolate(&position::ix) / 100.f + 0.5f) * TILE_SIZE + world->mapLeft();
    y = (interp.interpolate(&position::iy) / 100.f + 0.5f) * TILE_SIZE + world->mapTop();
}

void player::render(SDL_Renderer *renderer) {
    if (!spawned) return;

    static const int squaresize = 30;

    SDL_Rect rect_border = {(int)x - squaresize / 2 - 2, (int)y - squaresize / 2 - 2, squaresize + 4, squaresize + 4};
    if (!alive) {
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    } else if (is_self) {
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    } else {
        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
    }
    SDL_RenderFillRect(renderer, &rect_border);

    SDL_Rect rect = {(int)x - squaresize / 2, (int)y - squaresize / 2, squaresize, squaresize};
    SDL_SetRenderDrawColor(renderer, r, g, b, 255);
    SDL_RenderFillRect(renderer, &rect);
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
    uint32_t col = packet.readInt();
    r = (col & 0xff000000) >> 24;
    g = (col & 0x00ff0000) >> 16;
    b = (col & 0x0000ff00) >> 8;

    alive = packet.readChar() != 0;
    spawned = packet.readChar() != 0;
}
