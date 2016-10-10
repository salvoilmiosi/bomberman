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

static const uint8_t SKULL_RAPID_PACE = 1;
static const uint8_t SKULL_SLOW_PACE = 2;
static const uint8_t SKULL_DIARRHEA = 3;
static const uint8_t SKULL_LOW_POWER = 4;
static const uint8_t SKULL_CONSTIPATION = 5;

static const int SKULL_LIFE = TICKRATE * 20;

static const int ITEM_EXPLOSION_LIFE = TICKRATE * 2 / 3;

class game_item : public entity {
private:
    uint8_t tx, ty;

    int life_ticks;
    bool destroyed;

    uint8_t item_type;

public:
    game_item(game_world *world, tile *t, uint8_t it_type);

public:
    void tick();

    byte_array toByteArray();

    uint8_t getTileX() const {
        return tx;
    }

    uint8_t getTileY() const {
        return ty;
    }

    uint8_t getItemType() const {
        return item_type;
    }

    void pickup(class player *p);
    void explode();
};

#endif // __ENT_ITEM_H__
