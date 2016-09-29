#include "game_client.h"
#include "player.h"

game_client::game_client() {
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
    packet.writeInt(str2int("CONN"));
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

    Uint32 magic = accepter.readInt();
    if (magic != MAGIC) {
        return false;
    }

    Uint32 message = accepter.readInt();
    if (message != str2int("ACPT")) {
        return false;
    }

    return true;
}

void game_client::disconnect() {
    packet_ext packet(socket);
    packet.writeInt(str2int("DCON"));
    packet.sendTo(server_ip);
}

bool game_client::sendInput(Uint8 cmd, bool down) {
    packet_ext packet(socket);
    packet.writeInt(str2int("INPU"));
    packet.writeInt(str2int("KBRD"));
    packet.writeChar(cmd);
    packet.writeChar(down ? 1 : 0);
    return packet.sendTo(server_ip);
}

bool game_client::sendMouse(int x, int y) {
    packet_ext packet(socket);
    packet.writeInt(str2int("INPU"));
    packet.writeInt(str2int("MOUS"));
    packet.writeInt(x);
    packet.writeInt(y);
    return packet.sendTo(server_ip);
}

bool game_client::receive() {
    static int last_recv_time;

    while (true) {
        packet_ext packet(socket, true);
        if (packet.receive() <= 0) break;

        last_recv_time = SDL_GetTicks();

        int magic = packet.readInt();
        if (magic != MAGIC) break;

        int message = packet.readInt();
        switch (message) {
        case str2int("MESG"):
            messageCmd(packet);
            break;
        case str2int("PING"):
            pingCmd(packet);
            break;
        case str2int("MSEC"):
            msecCmd(packet);
            break;
        case str2int("STAT"):
            statCmd(packet);
            break;
        case str2int("SHOT"):
            snapshotCmd(packet);
            break;
        case str2int("ADDO"):
            addCmd(packet);
            break;
        case str2int("REMO"):
            remCmd(packet);
            break;
        case str2int("SELF"):
            selfCmd(packet);
            break;
        }
    }

    if (SDL_GetTicks() - last_recv_time > TIMEOUT) {
        return false;
    }

    return true;
}

void game_client::messageCmd(packet_ext &packet) {
    printf("%s\n", packet.readString());
}

void game_client::pingCmd(packet_ext &from) {
    packet_ext packet(socket);
    packet.writeInt(str2int("PONG"));
    packet.sendTo(server_ip);
}

void game_client::msecCmd(packet_ext &from) {
    ping_msecs = from.readShort();
}

void game_client::statCmd(packet_ext &packet) {
    // TODO scoreboard
}

void game_client::snapshotCmd(packet_ext &packet) {
    short num_objs = packet.readShort();
    for(;num_objs > 0; --num_objs) {
        Uint16 id = packet.readShort();
        Uint8 type = packet.readChar();
        short len = packet.readShort();

        game_obj *obj = world.findID(id);
        if (obj && obj->getType() == type) {
            obj->readFromPacket(packet);
        } else {
            packet.skip(len);
        }
    }
}

void game_client::addCmd(packet_ext &packet) {
    Uint16 id = packet.readShort();
    if (! world.findID(id)) {
        game_obj *obj = game_obj::newObjFromPacket(id, packet);
        if (obj) {
            world.addObject(obj);
        }
    }
}

void game_client::remCmd(packet_ext &packet) {
    Uint16 id = packet.readShort();
    world.removeObject(world.findID(id));
}

void game_client::selfCmd(packet_ext &packet) {
    short id = packet.readShort();
    game_obj *obj = world.findID(id);
    if (obj && obj->getType() == TYPE_PLAYER) {
        ((player *)obj)->setSelf(true);
    }
}
