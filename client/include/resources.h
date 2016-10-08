#ifndef __RESOURCES_H__
#define __RESOURCES_H__

#define IDI_ICON1 1
#define IDB_ICON2 2
#define IDB_TILESET 3
#define IDB_ITEMS 4
#define IDB_PLAYERS 5
#define IDB_FONT 6

#ifdef __cplusplus

#include <SDL2/SDL.h>

#define TILE(x, y) {x * 16, y * 16, 16, 16}

extern SDL_Texture *tileset_texture;
extern SDL_Texture *items_texture;
extern SDL_Texture *players_texture;
extern SDL_Texture *font_texture;

extern SDL_Surface *icon_surface;

void loadResources(SDL_Renderer *renderer);

void clearResources();

#endif // __cplusplus

#endif // __RESOURCES_H__
