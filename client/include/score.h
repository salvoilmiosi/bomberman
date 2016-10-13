#ifndef __SCORE_H__
#define __SCORE_H__

#include "packet_io.h"
#include "player.h"

static const int SCOREBOARD_WIDTH = 400;

static const int MAX_INFO_SIZE = 16;

class score {
private:
    class game_client *client;

    bool is_shown;

    int ticks_to_send_score;

    struct score_info {
        uint8_t player_num;
        char player_name[NAME_SIZE];
        uint16_t victories;
        int16_t ping;
        bool is_player;
    };

    score_info info[MAX_INFO_SIZE];
    uint8_t num_players;

public:
    score(class game_client *client);

public:
    void clear();

    void tick();

    void render(SDL_Renderer *renderer);

    void show(bool shown);

    void handlePacket(packet_ext &packet);
};

#endif // __SCORE_H__
