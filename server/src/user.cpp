#include "user.h"

#include "player.h"

#include "game_server.h"

uint16_t user::maxuserid = 0;

user::user(game_server *server, const IPaddress &address, const char *name) : server(server), address(address) {
    userid = maxuserid++;
    has_pinged = false;
    ping_msecs = -1;
    attempts = 0;

    setName(name);
}

user::~user() {
    //destroyPlayer();
}

bool user::tick() {
    if (getPing() < 0) {
        sendPing();
    }
    if (attempts > MAX_PING_ATTEMPTS) {
        return false;
    } else if (aliveTime() > TIMEOUT) {
        sendPing(true);
    }
    return true;
}

void user::sendPing(bool force) {
    if (!has_pinged || force) {
        ping_time = SDL_GetTicks();
        has_pinged = true;
        ++attempts;

        server->sendPingPacket(getAddress());
    }
}

void user::setName(const char *name) {
    strncpy(username, name, USER_NAME_SIZE);
    if (ent) {
        ent->setName(name);
    }
}

void user::pongCmd() {
    attempts = 0;
    ping_msecs = aliveTime();
    has_pinged = false;

    sendPing();
}

void user::handleInputPacket(packet_ext &packet) {
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

void user::createPlayer(game_world *world) {
    ent = new player(world, &handler);
    ent->setName(username);
}

void user::setPlayer(player *p) {
    ent = p;
    strncpy(username, p->getName(), USER_NAME_SIZE);
}

void user::destroyPlayer() {
    if (ent) {
        ent->destroy();
        ent = nullptr;
    }
}
