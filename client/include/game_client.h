#ifndef __GAME_CLIENT_H__
#define __GAME_CLIENT_H__

#include <SDL2/SDL_net.h>

#include "packet_io.h"
#include "game_world.h"

static const int TIMEOUT = 2000;

class game_client {
private:
    UDPsocket socket;
    SDLNet_SocketSet sock_set;
    IPaddress server_ip;

    game_world world;

    short ping_msecs;

public:
    game_client();
    virtual ~game_client();

public:
    bool connect(const char *address, const char *username, int port);
    void disconnect();

    bool receive();

    bool sendInput(Uint8 cmd, bool down);
    bool sendMouse(int x, int y);

    std::map<Uint16, game_obj *> getObjects() {
        return world.getObjects();
    }

    void tick() {
        world.tick();
    }

    short getPingMsecs() {
        return ping_msecs;
    }

private:
    void messageCmd(packet_ext &);
    void pingCmd(packet_ext &);
    void msecCmd(packet_ext &);
    void statCmd(packet_ext &);
    void snapshotCmd(packet_ext &);
    void addCmd(packet_ext &);
    void remCmd(packet_ext &);
    void selfCmd(packet_ext &);
};

#endif // __GAME_CLIENT_H__
