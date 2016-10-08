#ifndef __SERVER_H__
#define __SERVER_H__

#include <SDL2/SDL.h>
#include <SDL2/SDL_net.h>

#include <cstdio>
#include <map>

#include "packet_io.h"
#include "user.h"

static const uint16_t PORT = 27015;

static const int TICKRATE = 60;
static const int MAX_ATTEMPTS = 3;
static const int TIMEOUT = 1000;

static const int SNAPSHOT_RATE = 1;

static const int NUM_PLAYERS = 1;
static const int NUM_SPECTATORS = 16;

static const uint32_t CMD_CONNECT      = str2int("CONN");
static const uint32_t CMD_CHAT         = str2int("CHAT");
static const uint32_t CMD_DISCONNECT   = str2int("DCON");
static const uint32_t CMD_NAME         = str2int("NAME");
static const uint32_t CMD_PONG         = str2int("PONG");
static const uint32_t CMD_STATUS       = str2int("STAT");
static const uint32_t CMD_INPUT        = str2int("INPU");

static const uint32_t SERV_ACCEPT      = str2int("ACPT");
static const uint32_t SERV_REJECT      = str2int("REJT");
static const uint32_t SERV_KICK        = str2int("KICK");
static const uint32_t SERV_MESSAGE     = str2int("MESG");
static const uint32_t SERV_PING        = str2int("PING");
static const uint32_t SERV_PING_MSECS  = str2int("MSEC");
static const uint32_t SERV_STATUS      = str2int("STAT");
static const uint32_t SERV_SNAPSHOT    = str2int("SHOT");
static const uint32_t SERV_ADD_ENT     = str2int("ADDO");
static const uint32_t SERV_REM_ENT     = str2int("REMO");
static const uint32_t SERV_SELF        = str2int("SELF");
static const uint32_t SERV_MAP         = str2int("MAPP");

class game_server {
private:
    bool open;
    UDPsocket socket_serv;

    std::map<Uint64, user *> users;
    SDLNet_SocketSet sock_set;

    SDL_Thread *game_thread;

    class game_world *world;

public:
    game_server(class game_world *world);
    virtual ~game_server();

public:
    bool openServer();
    void closeServer();

    int run();

    bool isOpen() {
        return open;
    }

    int game_thread_run();

    void sendAddPacket(class entity *ent);
    void sendRemovePacket(class entity *ent);

private:
    int receive(packet_ext &packet);
    void handlePacket(packet_ext &packet);

    bool sendPing(user *u, bool force = false);

    char *findNewName(const char *username);

    user *findUser(const IPaddress &address);
    void sendToAll(const packet_ext &packet);

    packet_ext snapshotPacket(bool is_add = false);
    packet_ext statusPacket();
    packet_ext mapPacket();

    void connectCmd(packet_ext&);
    void chatCmd(packet_ext&);
    void nameCmd(packet_ext&);
    void disconnectCmd(packet_ext&);
    void pongCmd(packet_ext&);
    void statusCmd(packet_ext&);
    void quitCmd(packet_ext&);
    void inputCmd(packet_ext&);

public:
    void messageToAll(const char *message);

    template<typename ... T> void messageToAll(const char *format, T ... args) {
        char message[128];
        snprintf(message, 128, format, args ...);

        messageToAll(message);
    }
};

#endif // __SERVER_H__
