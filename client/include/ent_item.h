#ifndef __ENT_ITEM_H__
#define __ENT_ITEM_H__

#include "game_world.h"

static const uint8_t ITEM_BOMB = 1;
static const uint8_t ITEM_FIRE = 2;
static const uint8_t ITEM_ROLLERBLADE = 3;
static const uint8_t ITEM_KICK = 4;
static const uint8_t ITEM_PUNCH = 5;
static const uint8_t ITEM_SKULL = 6;
static const uint8_t ITEM_FULL_FIRE = 7;
static const uint8_t ITEM_REDBOMB = 8;
static const uint8_t ITEM_REMOCON = 9;

class game_item : public entity {
private:
    uint8_t tx, ty;

    uint8_t item_type;

    int create_time;
    int destroy_time;

    bool exploded;

public:
    game_item(game_world *world, uint16_t id, byte_array &ba);

public:
    void tick();

    void render(SDL_Renderer *renderer);

    void readFromByteArray(byte_array &ba);
};

#endif // __ENT_ITEM_H__
