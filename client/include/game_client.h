#ifndef __GAME_CLIENT_H__
#define __GAME_CLIENT_H__

#include <SDL2/SDL_net.h>

#include "packet_io.h"
#include "game_world.h"
#include "chat.h"
#include "score.h"
#include "main.h"

static const int TIMEOUT = 2000;

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
static const uint32_t SERV_ADD_MANY    = str2int("ADDM");
static const uint32_t SERV_ADD_ENT     = str2int("ADDO");
static const uint32_t SERV_REM_ENT     = str2int("REMO");
static const uint32_t SERV_SELF        = str2int("SELF");
static const uint32_t SERV_MAP         = str2int("MAPP");
static const uint32_t SERV_SOUND       = str2int("WAVE");

class game_client {
private:
    UDPsocket socket = nullptr;
    SDLNet_SocketSet sock_set = nullptr;
    IPaddress server_ip;

    short ping_msecs;

    char user_name[NAME_SIZE];

private:
    game_world world;

    chat g_chat;
    score g_score;

    bool is_open = true;

public:
    game_client();

    virtual ~game_client();

public:
    bool connect(const char *address, uint16_t port = DEFAULT_PORT);
    void disconnect();

    void clear();

    void tick();

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
    void sendScorePacket();
    void setName(const char *name);
    void quit();

    bool isOpen() {
        return is_open;
    }

private:
    void messageCmd(packet_ext &);
    void pingCmd(packet_ext &);
    void msecCmd(packet_ext &);
    void scoreCmd(packet_ext &);
    void snapshotCmd(packet_ext &);
    void addCmd(packet_ext &);
    void remCmd(packet_ext &);
    void selfCmd(packet_ext &);
    void mapCmd(packet_ext &);
    void soundCmd(packet_ext &);
};

#endif // __GAME_CLIENT_H__
