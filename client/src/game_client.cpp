#include "game_client.h"

#include "game_world.h"
#include "player.h"
#include "bindings.h"
#include "game_sound.h"

game_client::game_client() : g_chat(this), g_score(this) {
    sock_set = SDLNet_AllocSocketSet(1);

    strcpy(user_name, "Player");
}

game_client::~game_client() {
    if (sock_set) {
        SDLNet_FreeSocketSet(sock_set);
        sock_set = nullptr;
    }

    if (socket) {
        SDLNet_UDP_Close(socket);
        socket = nullptr;
    }
}

bool game_client::connect(const char *address, uint16_t port) {
    if (socket) return false;

    socket = SDLNet_UDP_Open(0);

    if (!socket) {
        g_chat.addLine(COLOR_RED, "Error creating socket: %s", SDLNet_GetError());
        clear();
        return false;
    }

    SDLNet_ResolveHost(&server_ip, address, port);

    packet_ext packet(socket);
    packet.writeInt(CMD_CONNECT);
    packet.writeString(user_name);
    packet.sendTo(server_ip);

    SDLNet_UDP_AddSocket(sock_set, socket);

    if (SDLNet_CheckSockets(sock_set, TIMEOUT) <= 0) {
        g_chat.addLine(COLOR_RED, "Server at host %s:%d timed out.", address, port);
        clear();
        return false;
    }

    packet_ext accepter(socket, true);

    if (accepter.receive() <= 0) {
        g_chat.addLine(COLOR_RED, "Server at host %s:%d does not respond: %s", address, port, SDLNet_GetError());
        clear();
        return false;
    }

    uint32_t magic = accepter.readInt();
    if (magic != MAGIC) {
        g_chat.addLine(COLOR_RED, "Error connecting to the host at %s:%d", address, port);
        clear();
        return false;
    }

    uint32_t type = accepter.readInt();
    char *message;
    switch (type) {
    case SERV_ACCEPT:
        message = accepter.readString();
        g_chat.addLine(COLOR_CYAN, "Server accepted: %s", message);
        g_chat.addLine(COLOR_CYAN, "Connected to %s:%d", address, port);

        playMusic(music_battle);
        return true;
    case SERV_REJECT:
        message = accepter.readString();
        g_chat.addLine(COLOR_RED, "Server rejected: %s", message);
        clear();
        return false;
    default:
        g_chat.addLine(COLOR_RED, "Error: Server returned the wrong command.");
        clear();
        return false;
    }

    // should never happen
    return false;
}

void game_client::disconnect() {
    if (socket == nullptr) return;

    packet_ext packet(socket);
    packet.writeInt(CMD_DISCONNECT);
    packet.sendTo(server_ip);

    clear();

    g_chat.addLine(COLOR_CYAN, "Disconnected.");
}

void game_client::quit() {
    disconnect();
    is_open = false;
}

void game_client::clear() {
    if (socket == nullptr) return;

    SDLNet_UDP_DelSocket(sock_set, socket);
    SDLNet_UDP_Close(socket);
    socket = nullptr;

    world.clear();
    g_score.clear();

    stopMusic();
}

bool game_client::sendInput(uint8_t cmd, bool down) {
    if (cmd == 0) return false;
    if (socket == nullptr) return false;

    packet_ext packet(socket);
    packet.writeInt(CMD_INPUT);
    packet.writeInt(str2int("KBRD"));
    packet.writeChar(cmd);
    packet.writeChar(down ? 1 : 0);
    return packet.sendTo(server_ip);
}

bool game_client::sendMouse(int x, int y) {
    if (socket == nullptr) return false;

    packet_ext packet(socket);
    packet.writeInt(CMD_INPUT);
    packet.writeInt(str2int("MOUS"));
    packet.writeInt(x);
    packet.writeInt(y);
    return packet.sendTo(server_ip);
}

void game_client::tick() {
    static int last_recv_time;

    world.tick();
    g_chat.tick();
    g_score.tick();

    if (!socket) return;

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
        case SERV_SCORE:
            scoreCmd(packet);
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
        case SERV_SOUND:
            soundCmd(packet);
            break;
        }
    }

    if (SDL_GetTicks() - last_recv_time > TIMEOUT) {
        g_chat.addLine(COLOR_RED, "Timed out from server.");
        world.clear();
        clear();
    }

    return;
}

void game_client::render(SDL_Renderer *renderer) {
    world.render(renderer);
    g_chat.render(renderer);
    g_score.render(renderer);
}

void game_client::handleEvent(const SDL_Event &event) {
    if (g_chat.isTyping()) {
        g_chat.handleEvent(event);
        return;
    }
    bool down = false;
    switch(event.type) {
    case SDL_KEYDOWN:
        switch (event.key.keysym.scancode) {
        case SDL_SCANCODE_TAB:
            g_score.show(true);
            break;
        default:
            if (!event.key.repeat) {
                sendInput(getKeyBinding(event.key.keysym.scancode), true);
            }
            break;
        }
        break;
    case SDL_KEYUP:
        switch (event.key.keysym.scancode) {
        case SDL_SCANCODE_TAB:
            g_score.show(false);
            break;
        case SDL_SCANCODE_T:
            g_chat.startTyping();
            break;
        default:
            if (!event.key.repeat) {
                sendInput(getKeyBinding(event.key.keysym.scancode), false);
            }
            break;
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
    if (socket == nullptr) return;

    packet_ext packet(socket);
    packet.writeInt(CMD_CHAT);
    packet.writeString(message);
    packet.sendTo(server_ip);
}

void game_client::sendScorePacket() {
    if (socket == nullptr) return;

    packet_ext packet(socket);
    packet.writeInt(CMD_SCORE);
    packet.sendTo(server_ip);
}

void game_client::setName(const char *name) {
    strncpy(user_name, name, NAME_SIZE);
    g_chat.addLine(COLOR_CYAN, "Set name: %s", name);

    if (socket == nullptr) return;

    packet_ext packet(socket);
    packet.writeInt(CMD_NAME);
    packet.writeString(name);
    packet.sendTo(server_ip);
}

void game_client::messageCmd(packet_ext &packet) {
    char *message = packet.readString();
    uint32_t color = packet.readInt();

    printf("%s\n", message);
    g_chat.addLine(color, message);
}

void game_client::pingCmd(packet_ext &from) {
    if (socket == nullptr) return;

    packet_ext packet(socket);
    packet.writeInt(CMD_PONG);
    packet.sendTo(server_ip);
}

void game_client::msecCmd(packet_ext &from) {
    ping_msecs = from.readShort();
}

void game_client::scoreCmd(packet_ext &packet) {
    g_score.handlePacket(packet);
}

void game_client::snapshotCmd(packet_ext &packet) {
    while(!packet.atEnd()) {
        uint16_t id = packet.readShort();
        uint8_t type = packet.readChar();

        byte_array ba = packet.readByteArray();

        entity *ent = world.findID(id);
        if (ent && ent->getType() == type) {
            ent->readFromByteArray(ba);
        }
    }
}

void game_client::addCmd(packet_ext &packet) {
    while(!packet.atEnd()) {
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

void game_client::soundCmd(packet_ext &packet) {
    uint8_t sound_id = packet.readChar();
    playWaveById(sound_id);
}
