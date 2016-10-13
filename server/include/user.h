#ifndef __USER_H__
#define __USER_H__

#include <SDL2/SDL_net.h>
#include <cstring>

#include "input_net.h"

static const size_t NAME_SIZE = 32;

class user {
private:
    const IPaddress address;
    char username[NAME_SIZE];

    uint16_t userid;

    static uint16_t maxuserid;

    int ping_time;
    bool has_pinged;
    int ping_msecs;

    int attempts;

    input_net handler;
    class player *ent = nullptr;

public:
    user(const IPaddress &address, const char *name);

public:
    void createPlayer(class game_world *world, uint8_t player_num);

    const char *getName() {
        return username;
    }

    void setName(const char *name);

    void setPlayer(class player *p);

    const IPaddress &getAddress() {
        return address;
    }

public:
    player *getPlayer() {
        return ent;
    }

    bool pingCmd(bool force = false);

    void pongCmd();

    void handlePacket(packet_ext &packet);

    int getPing();

    int aliveTime();

    uint16_t getID() {
        return userid;
    }

    int pingAttempts() {
        return attempts;
    }

    void destroyPlayer();
};

#endif // __USER_H__
