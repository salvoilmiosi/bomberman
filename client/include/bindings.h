#ifndef __BINDINGS_H__
#define __BINDINGS_H__

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

void setupBindings();

usr_input getKeyBinding(SDL_Scancode code);

usr_input getMouseBinding(uint8_t button);

#endif // __BINDINGS_H__
