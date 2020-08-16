#include "input_net.h"

input_net::input_net() {
    memset(cmds, 0, sizeof cmds);
}

void input_net::handlePacket(packet_ext &packet) {
    uint32_t type = packet.readInt();

    switch(type) {
    case CMD_INPUT_KEYBOARD:
        {
            uint8_t cmd = packet.readChar();
            bool down = packet.readChar() != 0;
            cmds[cmd] = down;
        }
        break;
    case CMD_INPUT_MOUSE:
        {
            mouse_x = packet.readInt();
            mouse_y = packet.readInt();
        }
        break;
    default:
        break;
    }
}
