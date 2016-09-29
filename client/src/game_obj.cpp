#include "game_obj.h"

#include "player.h"
#include "bullet.h"

game_obj *game_obj::newObjFromPacket(Uint16 id, class packet_ext &packet) {
    Uint8 type = packet.readChar();
    short len = packet.readShort();
    switch(type) {
    case TYPE_PLAYER:
        return new player(id, packet);
    case TYPE_BULLET:
        return new bullet(id, packet);
    default:
        packet.skip(len);
        return nullptr;
    }
}
