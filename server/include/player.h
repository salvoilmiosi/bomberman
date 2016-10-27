#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "ent_movable.h"
#include "input_handler.h"
#include "ent_item.h"

#include <deque>
#include <list>

static const size_t     PLAYER_NAME_SIZE = 32;
static const int        PLAYER_DEATH_TICKS = TICKRATE / 3;
static const int        PLAYER_PUNCH_TICKS = TICKRATE / 4;
static const int        PLAYER_INVULNERABLE_TICKS = TICKRATE * 10;

static const uint8_t    PICKUP_HAS_PUNCH = 1 << 0;
static const uint8_t    PICKUP_HAS_KICK = 1 << 1;
static const uint8_t 	PICKUP_HAS_REDBOMB = 1 << 2;
static const uint8_t 	PICKUP_HAS_REMOCON = 1 << 3;

static const int 		PLAYER_KICK_TICKS = TICKRATE / 20;
static const int 		PLAYER_STUN_TICKS = TICKRATE;

static const float 		PLAYER_JUMP_SPEED = 600.f;
static const float 		PLAYER_Z_ACCEL = 800.f;

class player: public ent_movable {
private:
    friend class bomb;
    friend class game_item;

    input_handler *handler = nullptr;

    char player_name[PLAYER_NAME_SIZE];

    uint8_t player_num = 0xff;

    bool alive = false;
    bool spawned = false;

    int death_ticks;
    int invulnerable_ticks;

    float speed = 0.f;
    int explosion_size = 0;
    int num_bombs = 0;

    bool movement_keys_down[4] = {0};
    bool movement_keys_pressed[4] = {0};

    std::deque<uint8_t> movement_priority;

    std::deque<item_type> item_pickups;

    std::list<class bomb *> planted_bombs;

    std::list<class bomb *> kicked_bombs;

    float speedz = 0.f;

    bool moving;
    bool jumping;
    bool on_trampoline;

    direction player_direction;

    uint8_t pickups;

    int skull_effect;

    int punch_ticks;
    int skull_ticks;
    int kick_ticks;
    int stun_ticks;

    int victories;

public:
    player(game_world *world, input_handler *handler);

public:
    void tick();

public:
    void setHandler(input_handler *handle) {
        handler = handle;
    }

    void setName(const char *name);

    const char *getName() {
        return player_name;
    }

    bool isAlive() {
        return alive;
    }

    bool isFlying() {
        return jumping;
    }

    bool move(float dx, float dy);

    uint8_t getPlayerNum() {
        return player_num;
    }

    void setPlayerNum(uint8_t num) {
        player_num = num;
    }

    void resetScore() {
        victories = 0;
    }

    uint16_t getVictories() {
        return victories;
    }

	void addVictory();

    byte_array toByteArray();

public:
    void respawn(int tx, int ty);
    void kill(player *killer);
    void stun();
    void makeInvulnerable();

private:
    bool player_move(float dx, float dy);

    void explodedBomb(class bomb *b);
    void stoppedKick(class bomb *b);

    void handleInput();
    void spawnItems();

    void checkTrampoline();
};

#endif // __PLAYER_H__
