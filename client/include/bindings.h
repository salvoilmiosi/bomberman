#ifndef __BINDINGS_H__
#define __BINDINGS_H__

#include <SDL2/SDL.h>

static const uint8_t USR_UP = 1;
static const uint8_t USR_DOWN = 2;
static const uint8_t USR_LEFT = 3;
static const uint8_t USR_RIGHT = 4;
static const uint8_t USR_PLANT = 5;
static const uint8_t USR_PUNCH = 6;
static const uint8_t USR_DETONATE = 7;
static const uint8_t USR_STOP_KICK = 8;

void setupBindings();

uint8_t getKeyBinding(SDL_Scancode code);

uint8_t getMouseBinding(uint8_t button);

#endif // __BINDINGS_H__
