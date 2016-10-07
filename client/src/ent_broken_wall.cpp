#include "ent_broken_wall.h"

#include "resources.h"

broken_wall::broken_wall(game_world *world, uint16_t id, packet_ext &packet) : entity(world, TYPE_BROKEN_WALL, id) {
    readFromPacket(packet);

    create_time = SDL_GetTicks();
}

void broken_wall::render(SDL_Renderer *renderer) {
    static const SDL_Rect src_rects[] = {
        {0,  96, 16, 16},
        {16, 96, 16, 16},
        {32, 96, 16, 16},
        {48, 96, 16, 16},
        {64, 96, 16, 16},
        {80, 96, 16, 16}
    };

    int ticks_since_create = SDL_GetTicks() - create_time;
    int frame = ticks_since_create * 6 / 666;
    if (frame > 5) frame = 5;

    SDL_Rect dst_rect = world->tileRect(tx, ty);

    SDL_RenderCopy(renderer, tileset_texture, src_rects + frame, &dst_rect);
}

void broken_wall::readFromPacket(packet_ext &packet) {
    tx = packet.readChar();
    ty = packet.readChar();
}
