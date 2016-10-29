#ifndef __SERVER_H__
#define __SERVER_H__

#include <SDL2/SDL.h>
#include <SDL2/SDL_net.h>

#include <cstdio>
#include <map>
#include <vector>

#include "packet_io.h"
#include "user.h"
#include "user_bot.h"
#include "packet_repeater.h"
#include "vote_handler.h"

static const int TIMEOUT = 1000;

static const int TICKRATE = 60;

static const uint16_t MAX_USERS = 128;

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
static const uint32_t SERV_ADD_ENT     = str2int("ADDO");
static const uint32_t SERV_REM_ENT     = str2int("REMO");
static const uint32_t SERV_SELF        = str2int("SELF");
static const uint32_t SERV_MAP         = str2int("MAPP");
static const uint32_t SERV_SOUND       = str2int("WAVE");
static const uint32_t SERV_RESET       = str2int("RESE");
static const uint32_t SERV_REPEAT      = str2int("REPT");

static const uint32_t COLOR_RED        = 0xff0000ff;
static const uint32_t COLOR_GREEN      = 0x00ff00ff;
static const uint32_t COLOR_BLUE       = 0x0000ffff;
static const uint32_t COLOR_MAGENTA    = 0xff00ffff;
static const uint32_t COLOR_YELLOW     = 0xffff00ff;
static const uint32_t COLOR_CYAN       = 0x00ffffff;
static const uint32_t COLOR_BLACK      = 0x000000ff;
static const uint32_t COLOR_WHITE      = 0xffffffff;

enum score_user_type {
    SCORE_SPECTATOR,
    SCORE_PLAYER,
    SCORE_BOT
};

static const int MAX_PLAYERS = 4;

class game_server {
private:
    bool open;

    class game_world *world;

    std::map<uint64_t, user *> users;
    std::vector<user_bot *> bots;

    SDLNet_SocketSet sock_set;
    UDPsocket socket_serv;

    SDL_Thread *game_thread;

    packet_repeater repeater;

    vote_handler voter;

public:
    game_server(class game_world *world);
    virtual ~game_server();

public:
    bool openServer(uint16_t port);
    void closeServer();

    int run();

    bool isOpen() {
        return open;
    }

    void startGame();
    void resetGame();

    bool gameStarted();

    void addBots(int num_bots);
    void removeBots();

    void sendAddPacket(class entity *ent);
    void sendRemovePacket(class entity *ent);
    void sendResetPacket();
    void sendSoundPacket(uint8_t sound_id);
    void sendPingPacket(const IPaddress &address);

    void kickUser(user *u, const char *message);

    int countUsers(bool include_bots = true);

public:
    void sendRepeatPacket(packet_ext &packet, const IPaddress &addresss, int repeats = DEFAULT_REPEATS);
    void sendToAll(packet_ext &packet, int repeats = 1);

    packet_ext messagePacket(uint32_t color, const char *message);

    template<typename ... T> packet_ext messagePacket(uint32_t color, const char *format, T ... args) {
        char message[128];
        snprintf(message, 128, format, args ...);

        return messagePacket(color, message);
    }

    template<typename ... T> void messageToAll(uint32_t color, const char *format, T ... args) {
        packet_ext packet = messagePacket(color, format, args ...);
        sendToAll(packet, DEFAULT_REPEATS);
    }

    template<typename ... T> void sendMessageTo(const IPaddress &address, uint32_t color, const char *format, T ... args) {
        packet_ext packet = messagePacket(color, format, args ...);
        sendRepeatPacket(packet, address);
    }

    user *findUserByID(int id);

private:
    friend int game_thread_func(void *data);
    int game_thread_run();

    int receive(packet_ext &packet);
    void handlePacket(packet_ext &packet);

    char *findNewName(const char *username);

    user *findUser(const IPaddress &address);

    packet_ext snapshotPacket(bool is_add = false);
    packet_ext scorePacket();
    packet_ext mapPacket();

private:
    void connectCmd(packet_ext&);
    void joinCmd(packet_ext&);
    void leaveCmd(packet_ext&);
    void chatCmd(packet_ext&);
    void nameCmd(packet_ext&);
    void disconnectCmd(packet_ext&);
    void pongCmd(packet_ext&);
    void scoreCmd(packet_ext&);
    void quitCmd(packet_ext&);
    void inputCmd(packet_ext&);
    void voteCmd(packet_ext&);
    void killCmd(packet_ext&);
};

#endif // __SERVER_H__
