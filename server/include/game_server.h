#ifndef __SERVER_H__
#define __SERVER_H__

#include <SDL2/SDL.h>
#include <SDL2/SDL_net.h>

#include <cstdio>
#include <map>

#include "packet_io.h"
#include "user.h"

static const int TICKRATE = 60;
static const int MAX_ATTEMPTS = 3;
static const int TIMEOUT = 1000;

static const int SNAPSHOT_RATE = 1;

static const uint16_t MAX_USERS = 128;

static const uint32_t CMD_CONNECT      = str2int("CONN");
static const uint32_t CMD_CHAT         = str2int("CHAT");
static const uint32_t CMD_DISCONNECT   = str2int("DCON");
static const uint32_t CMD_NAME         = str2int("NAME");
static const uint32_t CMD_PONG         = str2int("PONG");
static const uint32_t CMD_SCORE        = str2int("SCOR");
static const uint32_t CMD_INPUT        = str2int("INPU");

static const uint32_t SERV_ACCEPT      = str2int("ACPT");
static const uint32_t SERV_REJECT      = str2int("REJT");
static const uint32_t SERV_KICK        = str2int("KICK");
static const uint32_t SERV_MESSAGE     = str2int("MESG");
static const uint32_t SERV_PING        = str2int("PING");
static const uint32_t SERV_PING_MSECS  = str2int("MSEC");
static const uint32_t SERV_SCORE       = str2int("SCOR");
static const uint32_t SERV_SNAPSHOT    = str2int("SHOT");
static const uint32_t SERV_ADD_ENT     = str2int("ADDO");
static const uint32_t SERV_REM_ENT     = str2int("REMO");
static const uint32_t SERV_SELF        = str2int("SELF");
static const uint32_t SERV_MAP         = str2int("MAPP");

static const uint32_t COLOR_RED        = 0xff0000ff;
static const uint32_t COLOR_GREEN      = 0x00ff00ff;
static const uint32_t COLOR_BLUE       = 0x0000ffff;
static const uint32_t COLOR_MAGENTA    = 0xff00ffff;
static const uint32_t COLOR_YELLOW     = 0xffff00ff;
static const uint32_t COLOR_CYAN       = 0x00ffffff;
static const uint32_t COLOR_BLACK      = 0x000000ff;
static const uint32_t COLOR_WHITE      = 0xffffffff;

class game_server {
private:
    bool open;
    UDPsocket socket_serv;

    std::map<Uint64, user *> users;
    SDLNet_SocketSet sock_set;

    SDL_Thread *game_thread;

    class game_world *world;

    uint8_t NUM_PLAYERS;

public:
    game_server(class game_world *world, uint8_t num_players);
    virtual ~game_server();

public:
    bool openServer(uint16_t port);
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
    void sendToAll(packet_ext &packet);

    packet_ext snapshotPacket(bool is_add = false);
    packet_ext scorePacket();
    packet_ext mapPacket();

    void connectCmd(packet_ext&);
    void chatCmd(packet_ext&);
    void nameCmd(packet_ext&);
    void disconnectCmd(packet_ext&);
    void pongCmd(packet_ext&);
    void scoreCmd(packet_ext&);
    void quitCmd(packet_ext&);
    void inputCmd(packet_ext&);

public:
    void messageToAll(uint32_t color, const char *message);

    template<typename ... T> void messageToAll(uint32_t color, const char *format, T ... args) {
        char message[128];
        snprintf(message, 128, format, args ...);

        messageToAll(color, message);
    }
};

#endif // __SERVER_H__
