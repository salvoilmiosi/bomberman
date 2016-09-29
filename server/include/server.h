#ifndef __SERVER_H__
#define __SERVER_H__

#include <SDL2/SDL.h>
#include <SDL2/SDL_net.h>

#include <cstdio>
#include <map>

#include "packet_io.h"
#include "user.h"
#include "game_world.h"

static const Uint16 PORT = 27015;

static const int TICKRATE = 60;
static const int MAX_ATTEMPTS = 3;
static const int TIMEOUT = 1000;

static const int SNAPSHOT_RATE = 1;

static const Uint32 CMD_CONNECT      = str2int("CONN");
static const Uint32 CMD_CHAT         = str2int("CHAT");
static const Uint32 CMD_DISCONNECT   = str2int("DCON");
static const Uint32 CMD_NAME         = str2int("NAME");
static const Uint32 CMD_PONG         = str2int("PONG");
static const Uint32 CMD_STATUS       = str2int("STAT");
static const Uint32 CMD_INPUT        = str2int("INPU");

static const Uint32 SERV_ACCEPT      = str2int("ACPT");
static const Uint32 SERV_MESSAGE     = str2int("MESG");
static const Uint32 SERV_PING        = str2int("PING");
static const Uint32 SERV_PING_MSECS  = str2int("MSEC");
static const Uint32 SERV_STATUS      = str2int("STAT");
static const Uint32 SERV_SNAPSHOT    = str2int("SHOT");
static const Uint32 SERV_ADD_OBJ     = str2int("ADDO");
static const Uint32 SERV_REM_OBJ     = str2int("REMO");
static const Uint32 SERV_SELF        = str2int("SELF");

class game_server {
private:
    bool open;
    UDPsocket socket_serv;

    std::map<Uint64, user *> users;
    SDLNet_SocketSet sock_set;

    SDL_Thread *game_thread;

    game_world world;

public:
    game_server();
    virtual ~game_server();

public:
    bool openServer();
    void closeServer();

    int run();

    bool isOpen() {
        return open;
    }

    int game_thread_run();

    void addObject(game_obj *obj);
    void removeObject(game_obj *obj);

    const std::set<game_obj *>&getObjects() {
        return world.getObjects();
    }

private:
    int receive(packet_ext &packet);
    void handlePacket(packet_ext &packet);

    bool sendPing(user *u, bool force = false);

    char *findNewName(const char *username);

    user *findUser(const IPaddress &address);
    void sendToAll(packet_ext &packet);

    packet_ext snapshotPacket();
    packet_ext statusPacket();

    void connectCmd(packet_ext&);
    void chatCmd(packet_ext&);
    void nameCmd(packet_ext&);
    void disconnectCmd(packet_ext&);
    void pongCmd(packet_ext&);
    void statusCmd(packet_ext&);
    void quitCmd(packet_ext&);
    void inputCmd(packet_ext&);

    template<typename ... T> void messageToAll(const char *format, T ... args) {
        char message[128];
        snprintf(message, 128, format, args ...);

        packet_ext packet(socket_serv);
        packet.writeInt(SERV_MESSAGE);
        packet.writeString(message);

        sendToAll(packet);

        printf("%s\n", message);
    }
};

#endif // __SERVER_H__
