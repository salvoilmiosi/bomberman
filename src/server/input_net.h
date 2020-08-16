#ifndef __INPUT_NET_H__
#define __INPUT_NET_H__

#include "packet_io.h"
#include "input_handler.h"

static const uint32_t CMD_INPUT_KEYBOARD = str2int("KBRD");
static const uint32_t CMD_INPUT_MOUSE = str2int("MOUS");

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
    bool isDown(usr_input input_cmd) {
        return cmds[input_cmd];
    }

    bool isPressed(usr_input input_cmd) {
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
