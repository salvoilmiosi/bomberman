#include "user.h"

#include "game_world.h"
#include "server.h"

Uint16 user::maxuserid = 0;

user::user(const IPaddress &address, const char *name) : address(address) {
    userid = maxuserid++;
    has_pinged = false;
    ping_msecs = -1;
    attempts = 0;
    obj = nullptr;
    setName(name);
}

bool user::pingCmd(bool force) {
    if (!has_pinged || force) {
        ping_time = SDL_GetTicks();
        has_pinged = true;
        ++attempts;
        return true;
    }
    return false;
}

void user::pongCmd() {
    attempts = 0;
    ping_msecs = aliveTime();
    has_pinged = false;
}

void user::handlePacket(packet_ext &packet) {
    handler.handlePacket(packet);
}

int user::aliveTime() {
    if (has_pinged)
        return SDL_GetTicks() - ping_time;
    else
        return -1;
}

int user::getPing() {
    return ping_msecs;
}

void user::connected(game_server *server) {
    player *p = new player(&handler);

    setPlayer(p);

    server->addObject(p);
}

void user::disconnected(game_server *server) {
    if (obj) {
        server->removeObject(obj);
    }
}
