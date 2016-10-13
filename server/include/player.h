#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "game_world.h"
#include "input_handler.h"

#include <deque>

static const size_t PLAYERNAME_SIZE = 32;
static const float MOVE_TILE = 0.3f;
static const int PLAYER_DEATH_TICKS = TICKRATE / 3;
static const int PLAYER_PUNCH_TICKS = TICKRATE / 4;

static const uint8_t PICKUP_HAS_PUNCH = 1 << 0;
static const uint8_t PICKUP_HAS_KICK = 1 << 1;
static const uint8_t PICKUP_HAS_REDBOMB = 1 << 2;
static const uint8_t PICKUP_HAS_REMOCON = 1 << 3;

static const int KICK_TICKS = TICKRATE / 20;

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

    std::deque<class bomb *> planted_bombs;

    float fx = 0.f;
    float fy = 0.f;
    float fz = 0.f;

    float speedz = 0.f;

    bool moving;
    bool jumping;
    uint8_t direction;

    uint8_t pickups;

    int skull_effect;

    int punch_ticks;
    int skull_ticks;
    int kick_ticks;

    int victories;

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

    byte_array toByteArray();

    uint8_t getPlayerNum() {
        return player_num;
    }

    void addVictory() {
        ++victories;
    }

    uint16_t getVictories() {
        return victories;
    }

private:
    void explodedBomb(class bomb *b);
    void handleInput();
    void spawnItems();

    void checkTrampoline();
};

#endif // __PLAYER_H__
