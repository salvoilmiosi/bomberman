#ifndef __INPUT_HANDLER_H__
#define __INPUT_HANDLER_H__

#include <SDL2/SDL.h>

static const Uint8 USR_UP = 1;
static const Uint8 USR_DOWN = 2;
static const Uint8 USR_LEFT = 3;
static const Uint8 USR_RIGHT = 4;
static const Uint8 USR_JUMP = 5;
static const Uint8 USR_ACTION = 6;
static const Uint8 USR_FIRE = 7;

class input_handler {
public:
    virtual bool isDown(Uint8 input_cmd) = 0;
    virtual bool isPressed(Uint8 input_cmd) = 0;

    virtual int getMouseX() = 0;
    virtual int getMouseY() = 0;
};

#endif // __INPUT_HANDLER_H__
