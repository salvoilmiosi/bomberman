#ifndef __GAME_OBJ_H__
#define __GAME_OBJ_H__

#include <SDL2/SDL.h>

#include "packet_io.h"

static const int TYPE_PLAYER = 1;
static const int TYPE_BULLET = 2;

class game_obj {
private:
    const unsigned char obj_type;
    const Uint16 id;

public:
    game_obj(const unsigned char type, Uint16 id) : obj_type(type), id(id) {}

    virtual ~game_obj() {};

    virtual void tick() = 0;

    virtual void render(SDL_Renderer *renderer) = 0;

    virtual void readFromPacket(packet_ext &packet) = 0;

    static game_obj *newObjFromPacket(Uint16 id, packet_ext &packet);

    Uint16 getID() {
        return id;
    }

    char getType() {
        return obj_type;
    }
};

#endif // __GAME_OBJ_H__
