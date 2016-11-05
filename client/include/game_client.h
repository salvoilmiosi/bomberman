#ifndef __GAME_CLIENT_H__
#define __GAME_CLIENT_H__

#include <SDL2/SDL_net.h>
#include <string>
#include <map>
#include <thread>

#include "packet_io.h"
#include "game_world.h"
#include "chat.h"
#include "score.h"
#include "main.h"

static const int TIMEOUT = 2000;
static const int CONNECT_ATTEMPTS = 10;

static const uint32_t CMD_CONNECT      = str2int("CONN");
static const uint32_t CMD_CHAT         = str2int("CHAT");
static const uint32_t CMD_DISCONNECT   = str2int("DCON");
static const uint32_t CMD_NAME         = str2int("NAME");
static const uint32_t CMD_PONG         = str2int("PONG");
static const uint32_t CMD_SCORE        = str2int("SCOR");
static const uint32_t CMD_INPUT        = str2int("INPU");
static const uint32_t CMD_JOIN         = str2int("JOIN");
static const uint32_t CMD_LEAVE        = str2int("LEAV");
static const uint32_t CMD_VOTE         = str2int("VOTE");
static const uint32_t CMD_KILL         = str2int("KILL");

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
static const uint32_t SERV_RESET       = str2int("RESE");
static const uint32_t SERV_SOUND       = str2int("WAVE");
static const uint32_t SERV_REPEAT      = str2int("REPT");

static const uint32_t VOTE_YES         = str2int("VYES");
static const uint32_t VOTE_NO          = str2int("V_NO");
static const uint32_t VOTE_START       = str2int("STRT");
static const uint32_t VOTE_STOP        = str2int("STOP");
static const uint32_t VOTE_RESET       = str2int("RESE");
static const uint32_t VOTE_ADD_BOT     = str2int("BOT+");
static const uint32_t VOTE_REMOVE_BOTS = str2int("BOT-");
static const uint32_t VOTE_KICK        = str2int("KICK");
static const uint32_t VOTE_ZONE        = str2int("ZONE");

class game_client {
private:
    UDPsocket socket = nullptr;
    SDLNet_SocketSet sock_set = nullptr;
    IPaddress server_ip;

    short ping_msecs;

    std::string user_name = "Player";

    std::map<int, int> packet_repeat_ids;

private:
    game_world world;

    chat g_chat;
    score g_score;

    bool is_open = true;

    std::thread receiver_thread;

    int receiver_run();

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

    void execCmd(const std::string &message);

    bool sendInput(uint8_t cmd, bool down);
    bool sendMouse(int x, int y);
    bool sendJoinCmd();
    bool sendLeaveCmd();
    bool sendVoteCmd(uint32_t vote_type, uint32_t args);
    bool sendKillCmd();
    bool sendChatMessage(const char *message);
    bool sendScorePacket();

    short getPingMsecs() {
        return ping_msecs;
    }

    game_world &getWorld() {
        return world;
    }

    void setName(const std::string &name);
    void quit();

    bool isOpen() {
        return is_open;
    }

private:
    void handlePacket(byte_array &);

    void messageCmd(byte_array &);
    void kickCmd(byte_array &);
    void pingCmd(byte_array &);
    void msecCmd(byte_array &);
    void scoreCmd(byte_array &);
    void snapshotCmd(byte_array &);
    void addCmd(byte_array &);
    void remCmd(byte_array &);
    void selfCmd(byte_array &);
    void mapCmd(byte_array &);
    void soundCmd(byte_array &);
    void resetCmd(byte_array &);
    void repeatCmd(byte_array &);
};

#endif // __GAME_CLIENT_H__
