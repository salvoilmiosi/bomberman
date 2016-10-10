#ifndef __ENT_BROKEN_WALL_H__
#define __ENT_BROKEN_WALL_H__

#include "game_world.h"

class broken_wall : public entity {
private:
    uint8_t tx;
    uint8_t ty;

    uint8_t zone;

    int create_time;

public:
    broken_wall(game_world *world, uint16_t id, byte_array &ba);

public:
    void tick() {}

    void render(SDL_Renderer *renderer);

    void readFromByteArray(byte_array &ba);
};

#endif // __ENT_BROKEN_WALL_H__
