#include "bindings.h"

static const int NUM_KEYS = 512;
static const int NUM_MOUSE_BUTTONS = 16;

static usr_input key_bindings[NUM_KEYS];
static usr_input mouse_bindings[NUM_MOUSE_BUTTONS];

void setupBindings() {
    memset(key_bindings, 0, NUM_KEYS);
    key_bindings[SDL_SCANCODE_UP]       = USR_UP;
    key_bindings[SDL_SCANCODE_W]        = USR_UP;
    key_bindings[SDL_SCANCODE_DOWN]     = USR_DOWN;
    key_bindings[SDL_SCANCODE_S]        = USR_DOWN;
    key_bindings[SDL_SCANCODE_LEFT]     = USR_LEFT;
    key_bindings[SDL_SCANCODE_A]        = USR_LEFT;
    key_bindings[SDL_SCANCODE_RIGHT]    = USR_RIGHT;
    key_bindings[SDL_SCANCODE_D]        = USR_RIGHT;
    key_bindings[SDL_SCANCODE_SPACE]    = USR_PLANT;
    key_bindings[SDL_SCANCODE_Z]        = USR_PLANT;
    key_bindings[SDL_SCANCODE_X]        = USR_PUNCH;
    key_bindings[SDL_SCANCODE_C]        = USR_DETONATE;
    key_bindings[SDL_SCANCODE_V]        = USR_STOP_KICK;

    memset(mouse_bindings, 0, NUM_MOUSE_BUTTONS);
    //mouse_bindings[SDL_BUTTON_LEFT] = USR_PLANT;
    //mouse_bindings[SDL_BUTTON_RIGHT] = USR_PUNCH;
}

usr_input getKeyBinding(SDL_Scancode code) {
    return key_bindings[code];
}

usr_input getMouseBinding(uint8_t button) {
    return mouse_bindings[button];
}
