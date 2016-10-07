#ifndef __ENT_ITEM_H__
#define __ENT_ITEM_H__

#include "game_world.h"

static const uint8_t ITEM_ADD_BOMB = 1;
static const uint8_t ITEM_ADD_LENGTH = 2;
static const uint8_t ITEM_ADD_SPEED = 3;

class game_item : public entity {
private:
    uint8_t tx, ty;

    uint8_t item_type;

    int create_time;
    int destroy_time;

    bool destroyed;

public:
    game_item(game_world *world, uint16_t id, packet_ext &packet);

public:
    void tick();

    void render(SDL_Renderer *renderer);

    void readFromPacket(packet_ext &packet);
};

#endif // __ENT_ITEM_H__
