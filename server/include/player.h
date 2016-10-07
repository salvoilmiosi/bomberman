#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "game_world.h"
#include "input_handler.h"

#include <deque>

static const size_t USER_NAME_SIZE = 32;

class player: public entity {
private:
    friend class bomb;

    input_handler *handler = nullptr;

    uint32_t color = 0;

    char player_name[USER_NAME_SIZE];

    bool alive = false;
    bool spawned = false;

    float speed = 0.f;
    int explosion_size = 0;
    int num_bombs = 0;

    bool movement_keys_down[4] = {0};
    bool movement_keys_pressed[4] = {0};

    std::deque<uint8_t> movement_priority;

    float fx = 0.f;
    float fy = 0.f;

public:
    player(game_world *world, input_handler *handler);

public:
    void respawn(float x, float y);
    void kill();

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

private:
    void handleInput();
};

#endif // __PLAYER_H__
