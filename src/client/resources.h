#ifndef __RESOURCES_H__
#define __RESOURCES_H__

#include <SDL2/SDL.h>

#include <string>
#include <stdexcept>

#include "resource_load.h"

extern SDL_Surface *icon_surface;

extern SDL_Texture *font_texture;
extern SDL_Texture *tileset_1_texture;
extern SDL_Texture *tileset_2_texture;
extern SDL_Texture *tileset_3_texture;
extern SDL_Texture *tileset_4_texture;
extern SDL_Texture *tileset_5_texture;
extern SDL_Texture *tileset_6_texture;
extern SDL_Texture *tileset_7_texture;

extern SDL_Texture *explosions_texture;
extern SDL_Texture *items_texture;
extern SDL_Texture *players_texture;
extern SDL_Texture *players_white_texture;
extern SDL_Texture *player_icons_texture;

#define TILE(x, y) {(x)*16, (y)*16, 16, 16}

bool loadResources(SDL_Renderer *renderer);

void clearResources();

#endif // __RESOURCES_H__
