#ifndef __INPUT_NET_H__
#define __INPUT_NET_H__

#include "packet_io.h"
#include "input_handler.h"

static const Uint32 INPUT_KEYBOARD = str2int("KBRD");
static const Uint32 INPUT_MOUSE = str2int("MOUS");

class input_net: public input_handler {
private:
    bool cmds[256];

    int mouse_x;
    int mouse_y;

public:
    input_net();

public:
    void handlePacket(packet_ext &packet);

public:
    bool isDown(Uint8 input_cmd) {
        return cmds[input_cmd];
    }

    bool isPressed(Uint8 input_cmd) {
        bool ret = cmds[input_cmd];
        cmds[input_cmd] = false;
        return ret;
    }

    int getMouseX() {
        return mouse_x;
    }

    int getMouseY() {
        return mouse_y;
    }
};

#endif // __INPUT_NET_H__
