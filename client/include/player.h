#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "game_world.h"

#include <string>

#include "interpolator.h"

static const size_t NAME_SIZE = 32;

class player: public entity {
public:
    struct position {
        float ix, iy, iz;
    };

private:
    float x, y, z;

    int player_num;

    bool is_self;
    bool alive;
    bool spawned;

    int walk_time;
    int last_walk_frame;

    direction player_direction;

    bool moving;
    bool punching;
    bool diseased;
    bool jumping;
    bool invulnerable;
    bool stunned;

    std::string player_name;

    int death_time;

    interpolator<position> interp;

public:
    player(game_world *world, uint16_t id, byte_array &ba);

public:
    void tick();

    void render(SDL_Renderer *renderer);

    void setName(const std::string &name);
    const std::string &getName();

    void setSelf(bool s) {
        is_self = s;
    }

    void readFromByteArray(byte_array &ba);
};

#endif // __PLAYER_H__
