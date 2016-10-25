#ifndef __INPUT_HANDLER_H__
#define __INPUT_HANDLER_H__

#include <SDL2/SDL.h>

enum usr_input {
    USR_NONE,
    USR_UP,
    USR_DOWN,
    USR_LEFT,
    USR_RIGHT,
    USR_PLANT,
    USR_PUNCH,
    USR_DETONATE,
    USR_STOP_KICK
};

class input_handler {
public:
    virtual bool isDown(usr_input input_cmd) = 0;
    virtual bool isPressed(usr_input input_cmd) = 0;

    virtual int getMouseX() = 0;
    virtual int getMouseY() = 0;
};

#endif // __INPUT_HANDLER_H__
