#ifndef __BULLET_H__
#define __BULLET_H__

#include "game_obj.h"
#include "interpolator.h"

class bullet: public game_obj {
private:
    struct position {
        float ix, iy;
    };

    float x, y;

    interpolator<position> interp;

    Uint16 owner_id;

public:
    bullet(Uint16 id, packet_ext &packet);

public:
    void tick();

    void render(SDL_Renderer *renderer);

    void readFromPacket(packet_ext &packet);
};

#endif // __BULLET_H__
