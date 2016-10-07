#include "packet_io.h"
#include "game_world.h"

#include "player.h"
#include "ent_bomb.h"
#include "ent_broken_wall.h"
#include "ent_item.h"

entity *entity::newObjFromPacket(game_world *world, uint16_t id, class packet_ext &packet) {
    uint8_t type = packet.readChar();
    short len = packet.readShort();
    switch(type) {
    case TYPE_PLAYER:
        return new player(world, id, packet);
    case TYPE_BOMB:
        return new bomb(world, id, packet);
    case TYPE_EXPLOSION:
        return new explosion(world, id, packet);
    case TYPE_BROKEN_WALL:
        return new broken_wall(world, id, packet);
    case TYPE_ITEM:
        return new game_item(world, id, packet);
    default:
        packet.skip(len);
        return nullptr;
    }
}
