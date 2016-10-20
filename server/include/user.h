#ifndef __USER_H__
#define __USER_H__

#include <SDL2/SDL_net.h>
#include <cstring>

#include "input_net.h"

static const size_t USER_NAME_SIZE = 32;
static const int MAX_PING_ATTEMPTS = 5;

class user {
private:
    class game_server *server;

    const IPaddress address;
    char username[USER_NAME_SIZE];

    uint16_t userid;

    static uint16_t maxuserid;

    int ping_time;
    bool has_pinged;
    int ping_msecs;

    int attempts;

    input_net handler;
    class player *ent = nullptr;

public:
    user(class game_server *server, const IPaddress &address, const char *name);

    virtual ~user();

public:
    void createPlayer(class game_world *world, uint8_t player_num);

    const char *getName() {
        return username;
    }

    void setName(const char *name);

    const IPaddress &getAddress() {
        return address;
    }

public:
    class player *getPlayer() {
        return ent;
    }

    bool tick();

    void pongCmd();

    void handleInputPacket(packet_ext &packet);

    int getPing();

    int aliveTime();

    uint16_t getID() {
        return userid;
    }

    void destroyPlayer();

private:
    void sendPing(bool force = false);
};

#endif // __USER_H__
