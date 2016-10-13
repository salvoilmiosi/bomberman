#include "user.h"

#include "player.h"

uint16_t user::maxuserid = 0;

user::user(const IPaddress &address, const char *name) : address(address) {
    userid = maxuserid++;
    has_pinged = false;
    ping_msecs = -1;
    attempts = 0;

    setName(name);
}

void user::setName(const char *name) {
    strncpy(username, name, NAME_SIZE);
    if (ent) {
        ent->setName(name);
    }
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

void user::createPlayer(game_world *world, uint8_t player_num) {
    ent = new player(world, &handler, player_num);
    ent->setName(username);
}

void user::setPlayer(player *p) {
    ent = p;
    if (ent) {
        ent->setName(username);
    }
}

void user::destroyPlayer() {
    if (ent)
        ent->destroy();
}
