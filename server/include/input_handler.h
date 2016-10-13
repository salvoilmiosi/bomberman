#ifndef __INPUT_HANDLER_H__
#define __INPUT_HANDLER_H__

#include <SDL2/SDL.h>

static const uint8_t USR_UP = 1;
static const uint8_t USR_DOWN = 2;
static const uint8_t USR_LEFT = 3;
static const uint8_t USR_RIGHT = 4;
static const uint8_t USR_PLANT = 5;
static const uint8_t USR_PUNCH = 6;
static const uint8_t USR_DETONATE = 7;

class input_handler {
public:
    virtual bool isDown(uint8_t input_cmd) = 0;
    virtual bool isPressed(uint8_t input_cmd) = 0;

    virtual int getMouseX() = 0;
    virtual int getMouseY() = 0;
};

#endif // __INPUT_HANDLER_H__
