#include "server.h"

game_server::game_server() {
    open = false;

    openServer();
}

game_server::~game_server() {
    closeServer();
}

bool game_server::openServer() {
    if (open)
        return false;

    sock_set = SDLNet_AllocSocketSet(1);
    socket_serv = SDLNet_UDP_Open(PORT);
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

void game_server::sendToAll(packet_ext &packet) {
    for (auto it : users) {
        user *u = it.second;
        packet.sendTo(u->getAddress());
    }
}

packet_ext game_server::snapshotPacket() {
    packet_ext packet(socket_serv);
    packet.writeInt(SERV_SNAPSHOT);
    packet.writeShort(world.objectCount());
    for(auto obj : world.getObjects()) {
        if (obj && obj->isNotDestroyed() && obj->doSendUpdates()) {
            obj->writeToPacket(packet);
        }
    }
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
                messageToAll("(%s) %s timed out", ipString(u->getAddress()), u->getName());
                u->disconnected(this);
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
        }

        world.tick(this);

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

void game_server::addObject(game_obj *obj) {
    world.addObject(obj);

    packet_ext packet(socket_serv);
    packet.writeInt(SERV_ADD_OBJ);
    obj->writeToPacket(packet);
    sendToAll(packet);
}

void game_server::removeObject(game_obj *obj) {
    packet_ext packet(socket_serv);
    packet.writeInt(SERV_REM_OBJ);
    packet.writeShort(obj->getID());
    sendToAll(packet);

    obj->destroy();
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
    char *username = findNewName(from.readString());

    user *u = new user(from.getAddress(), username);
    users[addrLong(from.getAddress())] = u;

    packet_ext packet(socket_serv);
    packet.writeInt(SERV_ACCEPT);
    packet.sendTo(from.getAddress());

    u->connected(this);

    for(auto obj : world.getObjects()) {
        if (obj && obj->isNotDestroyed() && obj != u->getPlayer()) {
            packet_ext packet(socket_serv);
            packet.writeInt(SERV_ADD_OBJ);
            obj->writeToPacket(packet);
            packet.sendTo(from.getAddress());
        }
    }

    packet_ext packet_self(socket_serv);
    packet_self.writeInt(SERV_SELF);
    packet_self.writeShort(u->getPlayer()->getID());
    packet_self.sendTo(from.getAddress());

    messageToAll("(%s) %s connected", ipString(from.getAddress()), u->getName());
}

void game_server::chatCmd(packet_ext &packet) {
    user *u = findUser(packet.getAddress());
    if (!u) {
        fprintf(stderr, "Invalid user ip %s\n", ipString(packet.getAddress()));
        return;
    }

    char *message = packet.readString();

    messageToAll("(%s) %s : %s", ipString(packet.getAddress()), u->getName(), message);
}

void game_server::nameCmd(packet_ext &packet) {
    user *u = findUser(packet.getAddress());
    if (!u) {
        fprintf(stderr, "Invalid user ip %s\n", ipString(packet.getAddress()));
        return;
    }

    char *newName = findNewName(packet.readString());

    messageToAll("(%s) %s changed name to %s\n", ipString(u->getAddress()), u->getName(), newName);
}

void game_server::disconnectCmd(packet_ext &packet) {
    user *u = findUser(packet.getAddress());
    if (!u) {
        fprintf(stderr, "Invalid user ip %s\n", ipString(packet.getAddress()));
        return;
    }

    messageToAll("(%s) %s disconnected\n", ipString(u->getAddress()), u->getName());

    u->disconnected(this);
    delete u;
    users.erase(addrLong(packet.getAddress()));
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

packet_ext game_server::statusPacket() {
    packet_ext packet(socket_serv);
    packet.writeInt(SERV_STATUS);
    packet.writeShort(users.size());
    for (auto it : users) {
        user *u = it.second;
        packet.writeShort(u->getID());
        packet.writeLong(addrLong(u->getAddress()));
        packet.writeString(u->getName());
    }
    return packet;
}

void game_server::statusCmd(packet_ext &from) {
    statusPacket().sendTo(from.getAddress());
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
    Uint32 magic = packet.readInt();
    if (magic != MAGIC) {
        //fprintf(stderr, "%s: Invalid magic # %0#8x\n", ipString(packet.getAddress()), magic);
        return;
    }

    Uint32 command = packet.readInt();
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
    case CMD_STATUS:
        statusCmd(packet);
        break;
    case CMD_INPUT:
        inputCmd(packet);
        break;
    default:
        fprintf(stderr, "%s: Invalid command %0#8x\n", ipString(packet.getAddress()), command);
    }
}