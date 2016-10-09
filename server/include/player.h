#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "game_world.h"
#include "input_handler.h"

#include <deque>

static const size_t PLAYERNAME_SIZE = 32;
static const float MOVE_TILE = 0.3f;
static const int PLAYER_DEATH_TICKS = TICKRATE / 3;
static const int PLAYER_PUNCH_TICKS = TICKRATE / 4;

class player: public entity {
private:
    friend class bomb;
    friend class game_item;

    input_handler *handler = nullptr;

    char player_name[PLAYERNAME_SIZE];

    uint8_t player_num;

    bool alive = false;
    bool spawned = false;

    int death_ticks;

    float speed = 0.f;
    int explosion_size = 0;
    int num_bombs = 0;

    bool movement_keys_down[4] = {0};
    bool movement_keys_pressed[4] = {0};

    std::deque<uint8_t> movement_priority;

    std::deque<uint8_t> item_pickups;

    float fx = 0.f;
    float fy = 0.f;

    bool moving;
    uint8_t direction;

    bool can_kick;
    bool can_punch;
    int skull_effect;

    int punch_ticks;
    int skull_ticks;

public:
    player(game_world *world, input_handler *handler, uint8_t player_num);

public:
    void respawn(float x, float y);
    void kill();

    bool isAlive() {
        return alive;
    }

    void setName(const char *name);
    const char *getName() {
        return player_name;
    }

    void tick();

    uint8_t getTileX() const {
        return fx / TILE_SIZE + 0.5f;
    }

    uint8_t getTileY() const {
        return fy / TILE_SIZE + 0.5f;
    }

    void writeEntity(packet_ext &packet);

    uint8_t getPlayerNum() {
        return player_num;
    }

private:
    void handleInput();
    void spawnItems();
};

#endif // __PLAYER_H__
