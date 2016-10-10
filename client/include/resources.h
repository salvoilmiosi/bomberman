#ifndef __RESOURCES_H__
#define __RESOURCES_H__

#define IDI_ICON1 1
#define IDB_ICON2 2
#define IDB_TEXT 3
#define IDB_TILESET_1 4
#define IDB_TILESET_2 5
#define IDB_EXPLOSIONS 10
#define IDB_ITEMS 11
#define IDB_PLAYERS 12

#ifdef __cplusplus

#include <SDL2/SDL.h>

#define TILE(x, y) {x * 16, y * 16, 16, 16}

extern SDL_Surface *icon_surface;

extern SDL_Texture *text_texture;
extern SDL_Texture *tileset_1_texture;
extern SDL_Texture *tileset_2_texture;
extern SDL_Texture *explosions_texture;
extern SDL_Texture *items_texture;
extern SDL_Texture *players_texture;

void loadResources(SDL_Renderer *renderer);

void clearResources();

#endif // __cplusplus

#endif // __RESOURCES_H__
