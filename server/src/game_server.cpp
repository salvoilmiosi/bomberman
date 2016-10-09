#include "game_server.h"

#include "player.h"

game_server::game_server(game_world *world, uint8_t num_players) : world(world) {
    NUM_PLAYERS = num_players;
    open = false;
}

game_server::~game_server() {
    closeServer();
}

bool game_server::openServer(uint16_t port) {
    if (open)
        return false;

    sock_set = SDLNet_AllocSocketSet(1);
    socket_serv = SDLNet_UDP_Open(port);
    if (socket_serv) {
        SDLNet_UDP_AddSocket(sock_set, socket_serv);
        open = true;
        return true;
    }
    fprintf(stderr, "Error creating server socket: %s\n", SDLNet_GetError());

    return false;
}

void game_server::closeServer() {
    if (!open)
        return;

    for (auto it : users) {
        if (it.second)
            delete it.second;
    }
    users.clear();
    SDLNet_UDP_Close(socket_serv);
    open = false;
}

bool game_server::sendPing(user *u, bool force) {
    if (u->pingCmd(force)) {
        packet_ext packet(socket_serv);
        packet.writeInt(SERV_PING);
        packet.sendTo(u->getAddress());
        return true;
    }
    return false;
}

user *game_server::findUser(const IPaddress &address) {
    auto it = users.find(addrLong(address));
    if (it == users.end()) {
        return nullptr;
    }
    return it->second;
}

void game_server::sendToAll(const packet_ext &packet) {
    for (auto it : users) {
        user *u = it.second;
        packet.sendTo(u->getAddress());
    }
}

packet_ext game_server::snapshotPacket(bool is_add) {
    packet_ext packet(socket_serv);
    packet.writeInt(is_add ? SERV_ADD_ENT : SERV_SNAPSHOT);
    world->writeEntitiesToPacket(packet, is_add);
    return packet;
}

packet_ext game_server::mapPacket() {
    packet_ext packet(socket_serv);
    packet.writeInt(SERV_MAP);
    world->writeMapToPacket(packet);
    return packet;
}

int game_thread_func(void *data) {
    game_server *server = (game_server *)data;

    if (server) {
        return server->game_thread_run();
    }
    return 1;
}

int game_server::game_thread_run() {
    int tick_count = 0;
    while(open) {
        auto it = users.begin();
        while(it != users.end()) {
            user *u = it->second;
            if (u->getPing() < 0) {
                sendPing(u);
            }
            if (u->pingAttempts() > MAX_ATTEMPTS) {
                messageToAll(COLOR_YELLOW, "(%s) %s timed out", ipString(u->getAddress()), u->getName());
                u->destroyPlayer();
                delete u;
                it = users.erase(it);
            } else if (u->aliveTime() > TIMEOUT) {
                sendPing(u, true);
            } else {
                ++it;
            }
        }

        if (tick_count % SNAPSHOT_RATE == 0) {
            packet_ext snapshot = snapshotPacket();
            sendToAll(snapshot);
            sendToAll(mapPacket());
        }

        world->tick();

        SDL_Delay(1000 / TICKRATE);

        ++tick_count;
    }
    return 0;
}

int game_server::receive(packet_ext &packet) {
    int numready = SDLNet_CheckSockets(sock_set, TIMEOUT);
    if (numready > 0) {
        return packet.receive();
    } else if (numready < 0) {
        fprintf(stderr, "Error checking socket: %s\n", SDLNet_GetError());
    }
    return 0;
}

int game_server::run() {
    game_thread = SDL_CreateThread(game_thread_func, "GameLoop", this);

    if (!game_thread) {
        fprintf(stderr, "Error creating game thread: %s\n", SDL_GetError());
        return 1;
    }

    while (open) {
        packet_ext packet(socket_serv, true);
        int err = receive(packet);
        if (err > 0) {
            //printf("%s sent %d bytes\n", ipString(packet->address), packet->len);
            handlePacket(packet);
        } else if (err < 0) {
            fprintf(stderr, "Error receiving packet: %s\n", SDLNet_GetError());
        }
    }

    return 0;
}

void game_server::sendAddPacket(entity *ent) {
    packet_ext packet(socket_serv);
    packet.writeInt(SERV_ADD_ENT);
    packet.writeShort(1);
    ent->writeToPacket(packet);
    sendToAll(packet);
}

void game_server::sendRemovePacket(entity *ent) {
    packet_ext packet(socket_serv);
    packet.writeInt(SERV_REM_ENT);
    packet.writeShort(ent->getID());
    sendToAll(packet);
}

char *game_server::findNewName(const char *username) {
    static char newName[NAME_SIZE];
    strncpy(newName, username, NAME_SIZE);
    for (auto it : users) {
        user *u = it.second;
        if (strncmp(newName, u->getName(), NAME_SIZE) == 0) {
            strcat(newName, "'");
        }
    }
    return newName;
}

void game_server::connectCmd(packet_ext &from) {
    if (findUser(from.getAddress())) {
        fprintf(stderr, "%s is already connected\n", ipString(from.getAddress()));
        return;
    }

    if (users.size() > MAX_USERS) {
        packet_ext packet(socket_serv);
        packet.writeInt(SERV_REJECT);
        packet.writeString("Server is full");
        packet.sendTo(from.getAddress());
        return;
    }
    char *username = findNewName(from.readString());

    user *u = new user(from.getAddress(), username);
    users[addrLong(from.getAddress())] = u;

    packet_ext packet(socket_serv);
    packet.writeInt(SERV_ACCEPT);
    packet.writeString("Welcome to the server");
    packet.sendTo(from.getAddress());

    snapshotPacket(true).sendTo(from.getAddress());

    if (users.size() <= NUM_PLAYERS) {
        u->createPlayer(world, world->getNextPlayerNum());
        world->addEntity(u->getPlayer());

        packet_ext packet_self(socket_serv);
        packet_self.writeInt(SERV_SELF);
        packet_self.writeShort(u->getPlayer()->getID());
        packet_self.sendTo(from.getAddress());

        messageToAll(COLOR_YELLOW, "%s connected", u->getName());

        if (users.size() == NUM_PLAYERS) {
            world->startRound();
        }
    } else {
        messageToAll(COLOR_YELLOW, "%s joined the spectators", u->getName());
    }
}

void game_server::chatCmd(packet_ext &packet) {
    user *u = findUser(packet.getAddress());
    if (!u) {
        fprintf(stderr, "Invalid user ip %s\n", ipString(packet.getAddress()));
        return;
    }

    char *message = packet.readString();

    messageToAll(COLOR_WHITE, "%s : %s", u->getName(), message);
}

void game_server::nameCmd(packet_ext &packet) {
    user *u = findUser(packet.getAddress());
    if (!u) {
        fprintf(stderr, "Invalid user ip %s\n", ipString(packet.getAddress()));
        return;
    }

    char *newName = findNewName(packet.readString());

    messageToAll(COLOR_YELLOW, "%s changed name to %s\n", u->getName(), newName);
}

void game_server::disconnectCmd(packet_ext &packet) {
    user *u = findUser(packet.getAddress());
    if (!u) {
        fprintf(stderr, "Invalid user ip %s\n", ipString(packet.getAddress()));
        return;
    }

    messageToAll(COLOR_YELLOW, "%s disconnected", u->getName());

    u->destroyPlayer();
    users.erase(addrLong(packet.getAddress()));
    delete u;
}

void game_server::pongCmd(packet_ext &from) {
    user *u = findUser(from.getAddress());
    if (!u) {
        fprintf(stderr, "Invalid user ip %s\n", ipString(from.getAddress()));
        return;
    }

    u->pongCmd();

    packet_ext packet(socket_serv);
    packet.writeInt(SERV_PING_MSECS);
    packet.writeShort(u->getPing());
    packet.sendTo(u->getAddress());

    //printf("(%s) %s pings %d msecs\n", ipString(address), u->getName(), u->getPing());
    sendPing(u);
}

packet_ext game_server::scorePacket() {
    packet_ext packet(socket_serv);
    packet.writeInt(SERV_SCORE);
    packet.writeShort(users.size());
    for (auto it : users) {
        user *u = it.second;
        packet.writeString(u->getName());
        packet.writeShort(u->getPing());
        if (u->getPlayer()) {
            packet.writeChar(1);
            packet.writeChar(u->getPlayer()->getPlayerNum());
            packet.writeShort(u->getPlayer()->getVictories());
        } else {
            packet.writeChar(0);
        }
    }
    return packet;
}

void game_server::scoreCmd(packet_ext &from) {
    scorePacket().sendTo(from.getAddress());
}

void game_server::inputCmd(packet_ext &packet) {
    user *u = findUser(packet.getAddress());
    if (!u) {
        fprintf(stderr, "Invalid user ip %s\n", ipString(packet.getAddress()));
        return;
    }

    u->handlePacket(packet);
}

void game_server::handlePacket(packet_ext &packet) {
    uint32_t magic = packet.readInt();
    if (magic != MAGIC) {
        //fprintf(stderr, "%s: Invalid magic # %0#8x\n", ipString(packet.getAddress()), magic);
        return;
    }

    uint32_t command = packet.readInt();
    switch(command) {
    case CMD_CONNECT:
        connectCmd(packet);
        break;
    case CMD_CHAT:
        chatCmd(packet);
        break;
    case CMD_NAME:
        nameCmd(packet);
        break;
    case CMD_DISCONNECT:
        disconnectCmd(packet);
        break;
    case CMD_PONG:
        pongCmd(packet);
        break;
    case CMD_SCORE:
        scoreCmd(packet);
        break;
    case CMD_INPUT:
        inputCmd(packet);
        break;
    default:
        fprintf(stderr, "%s: Invalid command %0#8x\n", ipString(packet.getAddress()), command);
    }
}

void game_server::messageToAll(uint32_t color, const char *message) {
    packet_ext packet(socket_serv);
    packet.writeInt(SERV_MESSAGE);
    packet.writeString(message);
    packet.writeInt(color);

    sendToAll(packet);

    printf("%s\n", message);
}
