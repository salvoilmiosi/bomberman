#include "bullet.h"

#include <cstdio>

bullet::bullet(Uint16 id, packet_ext &packet) : game_obj(TYPE_BULLET, id) {
    readFromPacket(packet);
}

void bullet::tick() {
    interp.tick();

    interp.interpolate(&x, &position::ix);
    interp.interpolate(&y, &position::iy);
}

void bullet::render(SDL_Renderer *renderer) {
    SDL_Rect rect = {(int)x - 5, (int)y - 5, 10, 10};
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_RenderFillRect(renderer, &rect);
}

void bullet::readFromPacket(packet_ext &packet) {
    owner_id = packet.readShort();
    int xx = packet.readInt();
    int yy = packet.readInt();
    position pos = {(float)xx, (float)yy};
    interp.addSnapshot(pos);
}
