#ifndef __RESOURCES_H__
#define __RESOURCES_H__

#include <SDL2/SDL.h>

extern SDL_Surface *icon_surface;

extern SDL_Texture *text_texture;
extern SDL_Texture *tileset_1_texture;
extern SDL_Texture *tileset_2_texture;
extern SDL_Texture *tileset_3_texture;
extern SDL_Texture *tileset_4_texture;
extern SDL_Texture *explosions_texture;
extern SDL_Texture *items_texture;
extern SDL_Texture *players_texture;

#define TILE(x, y) {(x)*16, (y)*16, 16, 16}

bool openResourceFile(const char *filename);

SDL_RWops *getResourceRW(const char *RES_ID);

void loadResources(SDL_Renderer *renderer);

void clearResources();

#endif // __RESOURCES_H__
