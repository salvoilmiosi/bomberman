#ifndef __ENT_BOMB_H__
#define __ENT_BOMB_H__

#include "game_world.h"
#include "interpolator.h"

class explosion: public entity {
private:
    uint8_t x, y;
    uint16_t bomb_id;

    int explode_time;

    uint8_t len_l, len_t, len_r, len_b;
    bool trunc_l, trunc_t, trunc_r, trunc_b;

public:
    explosion(game_world *world, uint16_t id, packet_ext &packet);

public:
    void tick() {}

    void render(SDL_Renderer *renderer);

    void readFromPacket(packet_ext &packet);
};

class bomb: public entity {
public:
    struct position {
        float ix, iy, iz;
    };

private:
    float fx;
    float fy;
    float fz;

    uint16_t player_id;

    int create_time;

    interpolator<position> interp;

public:
    bomb(game_world *world, uint16_t id, packet_ext &packet);

public:
    void tick();

    void render(SDL_Renderer *renderer);

    void readFromPacket(packet_ext &packet);
};

#endif // __ENT_BOMB_H__
