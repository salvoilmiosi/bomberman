#ifndef __GAME_CLIENT_H__
#define __GAME_CLIENT_H__

#include <SDL2/SDL_net.h>

#include "packet_io.h"
#include "game_world.h"
#include "chat.h"

static const int TIMEOUT = 2000;

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
static const uint32_t SERV_ADD_MANY    = str2int("ADDM");
static const uint32_t SERV_ADD_ENT     = str2int("ADDO");
static const uint32_t SERV_REM_ENT     = str2int("REMO");
static const uint32_t SERV_SELF        = str2int("SELF");
static const uint32_t SERV_MAP         = str2int("MAPP");

class game_client {
private:
    UDPsocket socket;
    SDLNet_SocketSet sock_set;
    IPaddress server_ip;

    short ping_msecs;

private:
    game_world world;

    chat g_chat;

public:
    game_client();
    virtual ~game_client();

public:
    bool connect(const char *address, const char *username, int port);
    void disconnect();

    bool tick();

    void render(SDL_Renderer *renderer);

    void handleEvent(const SDL_Event &event);

    bool sendInput(uint8_t cmd, bool down);
    bool sendMouse(int x, int y);

    short getPingMsecs() {
        return ping_msecs;
    }

    game_world &getWorld() {
        return world;
    }

    void sendChatMessage(const char *message);

private:
    void messageCmd(packet_ext &);
    void pingCmd(packet_ext &);
    void msecCmd(packet_ext &);
    void statCmd(packet_ext &);
    void snapshotCmd(packet_ext &);
    void addCmd(packet_ext &);
    void remCmd(packet_ext &);
    void selfCmd(packet_ext &);
    void mapCmd(packet_ext &);
};

#endif // __GAME_CLIENT_H__
