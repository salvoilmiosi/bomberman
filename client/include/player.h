#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "game_obj.h"

#include "interpolator.h"

#include <cstdlib>

static const size_t NAME_SIZE = 32;

class player: public game_obj {
private:
    struct position {
        float ix, iy;
    };

    float x, y;
    Uint8 r, g, b;

    bool is_self;

    char player_name[NAME_SIZE];

    interpolator<position> interp;

    short health_points;

    bool death_animation;
    Uint32 death_time;
    Uint32 damage_time;
    Uint32 heal_time;

public:
    player(Uint16 id, packet_ext &packet);

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
