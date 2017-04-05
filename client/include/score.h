#ifndef __SCORE_H__
#define __SCORE_H__

#include "packet_io.h"
#include "player.h"

#include <map>
#include <deque>

static const int SCOREBOARD_WIDTH = 420;

static const int MAX_INFO_SIZE = 16;

static const int SCORE_LINE_SPACE = 20;
static const int ICON_SIZE = 32;

static const size_t PING_Q_SIZE = 10;
static const int SCORE_RATE = 4;

enum score_user_type {
    SCORE_SPECTATOR,
    SCORE_PLAYER,
    SCORE_BOT
};

class score {
private:
    class game_client *client;

    bool is_shown;

    int ticks_to_send_score;

    uint16_t self_id;

    struct score_info {
        uint16_t user_id;
        uint8_t player_num;
        score_user_type user_type;
        char player_name[NAME_SIZE];
        uint16_t victories;
        int16_t ping;
        bool alive;
    };

    std::map<uint16_t, std::deque<int16_t> > avg_pings;

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

    void setSelfID(uint16_t id) {
        self_id = id;
    }

    int findUserID(std::string name);
};

#endif // __SCORE_H__
