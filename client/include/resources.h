#ifndef __RESOURCES_H__
#define __RESOURCES_H__

#define IDI_ICON1 1
#define IDB_ICON2 2
#define IDB_TILESET 3
#define IDB_FONT 4

#ifdef __cplusplus

#include <SDL2/SDL.h>

extern SDL_Texture *tileset_texture;
extern SDL_Texture *font_texture;

extern SDL_Surface *icon_surface;

void loadResources(SDL_Renderer *renderer);

void clearResources();

#endif // __cplusplus

#endif // __RESOURCES_H__
