#ifndef __VOTE_HANDLER_H__
#define __VOTE_HANDLER_H__

#include "user.h"

#include <map>

static const int VOTE_TIMER = 20000;

static const uint8_t VOTE_START = 1;
static const uint8_t VOTE_STOP = 2;
static const uint8_t VOTE_RESET = 3;

class vote_handler {
private:
    class game_server *server;

    uint8_t current_vote = 0;

    std::map<user *, uint8_t> votes;

    int vote_start_time = 0;

public:
    vote_handler(class game_server *server);

public:
    void tick();
    void reset();

    void sendVote(user *u, uint8_t vote_type);
};

#endif // __VOTE_HANDLER_H__
