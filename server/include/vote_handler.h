#ifndef __VOTE_HANDLER_H__
#define __VOTE_HANDLER_H__

#include "user.h"

#include <map>

static const int VOTE_TIMER = 20000;

static const uint32_t VOTE_YES         = str2int("VYES");
static const uint32_t VOTE_NO          = str2int("V_NO");
static const uint32_t VOTE_START       = str2int("STRT");
static const uint32_t VOTE_STOP        = str2int("STOP");
static const uint32_t VOTE_RESET       = str2int("RESE");
static const uint32_t VOTE_ADD_BOT     = str2int("BOT+");
static const uint32_t VOTE_REMOVE_BOTS = str2int("BOT-");
static const uint32_t VOTE_KICK        = str2int("KICK");

class vote_handler {
private:
    class game_server *server;

    uint32_t current_vote = 0;
    uint32_t vote_args;

    std::map<user *, uint32_t> votes;

    int vote_start_time = 0;

public:
    vote_handler(class game_server *server);

public:
    void tick();
    void reset();

    void sendVote(user *u, uint32_t vote_type, uint32_t args);
};

#endif // __VOTE_HANDLER_H__
