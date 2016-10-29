#ifndef __USER_H__
#define __USER_H__

#include <SDL2/SDL_net.h>
#include <string>

#include "input_net.h"

static const int MAX_PING_ATTEMPTS = 5;

class user {
private:
    class game_server *server;

    const IPaddress address;
    std::string username;

    uint16_t userid;

    static uint16_t maxuserid;

    int ping_time;
    bool has_pinged;
    int ping_msecs;

    int attempts;

    input_net handler;
    class player *ent = nullptr;

public:
    user(class game_server *server, const IPaddress &address, const std::string &name);

    virtual ~user();

public:
    void createPlayer(class game_world *world);

    const char *getName() {
        return username.c_str();
    }

    void setName(const std::string &name);

    const IPaddress &getAddress() {
        return address;
    }

public:
    void setPlayer(class player *p);

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
