#ifndef __SCORE_H__
#define __SCORE_H__

#include "packet_io.h"
#include "player.h"

static const int SCOREBOARD_WIDTH = 400;

static const int MAX_INFO_SIZE = 16;

static const uint8_t SCORE_SPECTATOR = 0;
static const uint8_t SCORE_PLAYER    = 1;
static const uint8_t SCORE_BOT       = 2;

class score {
private:
    class game_client *client;

    bool is_shown;

    int ticks_to_send_score;

    struct score_info {
        uint8_t player_num;
        uint8_t user_type;
        char player_name[NAME_SIZE];
        uint16_t victories;
        int16_t ping;
        bool alive;
    };

    struct score_info_compare {
        bool operator()(const score_info &a, const score_info &b);
    } compare;

    score_info info[MAX_INFO_SIZE];
    uint8_t num_players;

public:
    score(class game_client *client);

public:
    void clear();

    void tick();

    void render(SDL_Renderer *renderer);

    void show(bool shown);

    void handlePacket(byte_array &packet);
};

#endif // __SCORE_H__
