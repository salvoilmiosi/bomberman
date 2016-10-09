#include "game_client.h"

#include "game_world.h"
#include "player.h"
#include "bindings.h"

#include <windows.h>

game_client::game_client() : g_chat(this) {
}

game_client::~game_client() {

}

bool game_client::connect(const char *address, const char *username, int port) {
    sock_set = SDLNet_AllocSocketSet(1);
    socket = SDLNet_UDP_Open(0);

    if (!socket) {
        return false;
    }

    SDLNet_ResolveHost(&server_ip, address, port);

    packet_ext packet(socket);
    packet.writeInt(CMD_CONNECT);
    packet.writeString(username);
    packet.sendTo(server_ip);

    SDLNet_UDP_AddSocket(sock_set, socket);

    if (SDLNet_CheckSockets(sock_set, TIMEOUT) <= 0) {
        return false;
    }

    packet_ext accepter(socket, true);

    if (accepter.receive() <= 0) {
        return false;
    }

    uint32_t magic = accepter.readInt();
    if (magic != MAGIC) {
        return false;
    }

    uint32_t type = accepter.readInt();
    char *message;
    switch (type) {
    case SERV_ACCEPT:
        message = accepter.readString();
        printf("%s\n", message);
        return true;
    case SERV_REJECT:
        message = accepter.readString();
        printf("%s\n", message);
    default:
        return false;
    }

    // should never happen
    return false;
}

void game_client::disconnect() {
    packet_ext packet(socket);
    packet.writeInt(CMD_DISCONNECT);
    packet.sendTo(server_ip);
}

bool game_client::sendInput(uint8_t cmd, bool down) {
    if (cmd == 0) return false;
    packet_ext packet(socket);
    packet.writeInt(CMD_INPUT);
    packet.writeInt(str2int("KBRD"));
    packet.writeChar(cmd);
    packet.writeChar(down ? 1 : 0);
    return packet.sendTo(server_ip);
}

bool game_client::sendMouse(int x, int y) {
    packet_ext packet(socket);
    packet.writeInt(CMD_INPUT);
    packet.writeInt(str2int("MOUS"));
    packet.writeInt(x);
    packet.writeInt(y);
    return packet.sendTo(server_ip);
}

bool game_client::tick() {
    static int last_recv_time;

    world.tick();
    g_chat.tick();

    while (true) {
        packet_ext packet(socket, true);
        if (packet.receive() <= 0) break;

        last_recv_time = SDL_GetTicks();

        int magic = packet.readInt();
        if (magic != MAGIC) break;

        int message = packet.readInt();
        switch (message) {
        case SERV_MESSAGE:
            messageCmd(packet);
            break;
        case SERV_PING:
            pingCmd(packet);
            break;
        case SERV_PING_MSECS:
            msecCmd(packet);
            break;
        case SERV_STATUS:
            statCmd(packet);
            break;
        case SERV_SNAPSHOT:
            snapshotCmd(packet);
            break;
        case SERV_ADD_ENT:
            addCmd(packet);
            break;
        case SERV_REM_ENT:
            remCmd(packet);
            break;
        case SERV_SELF:
            selfCmd(packet);
            break;
        case SERV_MAP:
            mapCmd(packet);
            break;
        }
    }

    if (SDL_GetTicks() - last_recv_time > TIMEOUT) {
        return false;
    }

    return true;
}

void game_client::render(SDL_Renderer *renderer) {
    world.render(renderer);
    g_chat.render(renderer);
}

void game_client::handleEvent(const SDL_Event &event) {
    if (g_chat.isTyping()) {
        g_chat.handleEvent(event);
        return;
    }
    bool down = false;
    switch(event.type) {
    case SDL_KEYDOWN:
    case SDL_KEYUP:
        down = event.type == SDL_KEYDOWN;
        if (!down && event.key.keysym.scancode == SDL_SCANCODE_T) {
            g_chat.startTyping();
            break;
        }
        if (!event.key.repeat) {
            sendInput(getKeyBinding(event.key.keysym.scancode), down);
        }
        break;
    case SDL_MOUSEMOTION:
        //sendMouse(event.motion.x, event.motion.y);
        break;
    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP:
        down = event.type == SDL_MOUSEBUTTONDOWN;
        sendInput(getMouseBinding(event.button.button), down);
        break;
    }
}

void game_client::sendChatMessage(const char *message) {
    packet_ext packet(socket);
    packet.writeInt(CMD_CHAT);
    packet.writeString(message);
    packet.sendTo(server_ip);
}

void game_client::messageCmd(packet_ext &packet) {
    char *message = packet.readString();
    uint32_t color = packet.readInt();

    printf("%s\n", message);
    g_chat.addLine(message, color);
}

void game_client::pingCmd(packet_ext &from) {
    packet_ext packet(socket);
    packet.writeInt(CMD_PONG);
    packet.sendTo(server_ip);
}

void game_client::msecCmd(packet_ext &from) {
    ping_msecs = from.readShort();
}

void game_client::statCmd(packet_ext &packet) {
    // TODO scoreboard
}

void game_client::snapshotCmd(packet_ext &packet) {
    short num_ents = packet.readShort();
    for(;num_ents > 0; --num_ents) {
        uint16_t id = packet.readShort();
        uint8_t type = packet.readChar();
        short len = packet.readShort();

        entity *ent = world.findID(id);
        if (ent && ent->getType() == type) {
            ent->readFromPacket(packet);
        } else {
            packet.skip(len);
        }
    }
}

void game_client::addCmd(packet_ext &packet) {
    short num_ents = packet.readShort();
    for(;num_ents > 0; --num_ents) {
        uint16_t id = packet.readShort();
        if (! world.findID(id)) {
            entity *ent = entity::newObjFromPacket(&world, id, packet);
            if (ent) {
                world.addEntity(ent);
            }
        }
    }
}

void game_client::remCmd(packet_ext &packet) {
    uint16_t id = packet.readShort();
    world.removeEntity(world.findID(id));
}

void game_client::selfCmd(packet_ext &packet) {
    uint16_t self_id = packet.readShort();

    entity *ent = world.findID(self_id);
    if (ent && ent->getType() == TYPE_PLAYER) {
        dynamic_cast<player *>(ent)->setSelf(true);
    }
}

void game_client::mapCmd(packet_ext &packet) {
    world.handleMapPacket(packet);
}
