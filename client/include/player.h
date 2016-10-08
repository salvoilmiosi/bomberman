#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "game_world.h"

#include <cstdlib>

#include "interpolator.h"
#include "healthbar.h"

static const size_t NAME_SIZE = 32;

class player: public entity {
public:
    struct position {
        float ix, iy;
        float imoving;
        float idirection;
        float ipunching;
    };

private:

    float x, y;

    int player_num;

    bool is_self;
    bool alive;
    bool spawned;

    bool moving;
    uint8_t direction;
    bool punching;

    char player_name[NAME_SIZE];

    int death_time;

    interpolator<position> interp;

public:
    player(game_world *world, uint16_t id, packet_ext &packet);

public:
    void tick();

    void render(SDL_Renderer *renderer);

    void setName(const char *name);
    const char *getName();

    void readFromPacket(packet_ext &packet);

    void setSelf(bool s) {
        is_self = s;
    }
};

#endif // __PLAYER_H__
