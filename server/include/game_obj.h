#ifndef __GAME_OBJ_H__
#define __GAME_OBJ_H__

#include "packet_io.h"

static const int TYPE_PLAYER = 1;
static const int TYPE_BULLET = 2;

class game_obj {
private:
    const Uint8 obj_type;

    Uint16 id;

    bool destroyed;

protected:
    bool do_send_updates;

public:
    game_obj(const Uint8 type) : obj_type(type) {
        static Uint16 max_id = 1;
        id = max_id++;
        destroyed = false;
        do_send_updates = true;
    }

    virtual ~game_obj() {};

    virtual void tick(class game_server *server) = 0;

    Uint16 getID() {
        return id;
    }

    Uint8 getType() {
        return obj_type;
    }

    bool isNotDestroyed() {
        return !destroyed;
    }

    void destroy() {
        destroyed = true;
    }

    void writeToPacket(packet_ext &packet) {
        packet.writeShort(getID());
        packet.writeChar(obj_type);
        writeObject(packet);
    }

    bool doSendUpdates() {
        return do_send_updates;
    }

private:
    virtual void writeObject(packet_ext &packet) = 0;
};

#endif // __GAME_OBJ_H__
