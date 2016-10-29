#include "ent_broken_wall.h"

#include "resources.h"

#include <map>
#include <vector>

broken_wall::broken_wall(game_world *world, uint16_t id, byte_array &ba) : entity(world, TYPE_BROKEN_WALL, id) {
    readFromByteArray(ba);
    create_time = SDL_GetTicks();
}

void broken_wall::render(SDL_Renderer *renderer) {
    static std::map<map_zone, std::vector<SDL_Rect> > src_rect_per_zone = {
        {ZONE_NORMAL, {TILE(0, 1), TILE(1, 1), TILE(2, 1), TILE(3, 1), TILE(0, 2), TILE(1, 2)}},
        {ZONE_WESTERN, {TILE(3, 3), TILE(4, 3), TILE(0, 4), TILE(1, 4), TILE(2, 4), TILE(3, 4)}},
        {ZONE_BOMB, {TILE(0, 3), TILE(1, 3), TILE(2, 3), TILE(3, 3), TILE(4, 3), TILE(5, 3)}},
        {ZONE_JUMP, {TILE(0, 4), TILE(1, 4), TILE(2, 4), TILE(3, 4), TILE(4, 4), TILE(5, 4)}},
        {ZONE_BELT, {TILE(0, 5), TILE(1, 5), TILE(2, 5), TILE(3, 5), TILE(4, 5), TILE(5, 5)}},
        {ZONE_DUEL, {TILE(0, 4), TILE(1, 4), TILE(2, 4), TILE(3, 4), TILE(4, 4), TILE(5, 4)}},
    }; // TODO create power and speed zone tilesets

    auto rect_vector_it = src_rect_per_zone.find(zone);
    if (rect_vector_it == src_rect_per_zone.end()) {
       	rect_vector_it = src_rect_per_zone.find(ZONE_NORMAL);
    }

    int ticks_since_create = SDL_GetTicks() - create_time;
    int frame = ticks_since_create * 6 / 666;
    if (frame > 5) frame = 5;

    SDL_Rect dst_rect = world->tileRect(tx, ty);

    SDL_Rect src_rect = rect_vector_it->second[frame];
    SDL_RenderCopy(renderer, game_map::getTileset(zone), &src_rect, &dst_rect);
}

void broken_wall::readFromByteArray(byte_array &ba) {
    tx = ba.readChar();
    ty = ba.readChar();
    zone = static_cast<map_zone>(ba.readChar());
}
