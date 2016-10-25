#ifndef __ENT_ITEM_H__
#define __ENT_ITEM_H__

#include "game_world.h"

enum item_type {
    ITEM_NONE,
    ITEM_BOMB,
    ITEM_FIRE,
    ITEM_ROLLERBLADE,
    ITEM_KICK,
    ITEM_PUNCH,
    ITEM_SKULL,
    ITEM_FULL_FIRE,
    ITEM_REDBOMB,
    ITEM_REMOCON
};

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
