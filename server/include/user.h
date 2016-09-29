#ifndef __USER_H__
#define __USER_H__

#include <SDL2/SDL_net.h>
#include <cstring>

#include "input_net.h"
#include "player.h"

class user {
private:
    const IPaddress address;
    char username[NAME_SIZE];

    Uint16 userid;

    static Uint16 maxuserid;

    int ping_time;
    bool has_pinged;
    int ping_msecs;

    int attempts;

    input_net handler;
    player *obj;

public:
    user(const IPaddress &address, const char *name);

public:
    const char *getName() {
        return username;
    }

    void setName(const char *name) {
        strncpy(username, name, NAME_SIZE);
        if (obj) {
            obj->setName(name);
        }
    }

    const IPaddress &getAddress() {
        return address;
    }

public:
    void connected(class game_server *server);
    void disconnected(class game_server *server);

    void setPlayer(player *p) {
        obj = p;
        if (obj) {
            obj->setName(username);
        }
    }

    player *getPlayer() {
        return obj;
    }

    bool pingCmd(bool force = false);

    void pongCmd();

    void handlePacket(packet_ext &packet);

    int getPing();

    int aliveTime();

    Uint16 getID() {
        return userid;
    }

    int pingAttempts() {
        return attempts;
    }
};

#endif // __USER_H__
