#ifndef __RESOURCES_H__
#define __RESOURCES_H__

#ifdef _WINDOWS_

#define IDI_ICON1 1
#define IDB_ICON2 2
#define IDB_TEXT 3
#define IDB_TILESET_1 4
#define IDB_TILESET_2 5
#define IDB_TILESET_3 6
#define IDB_TILESET_4 7
#define IDB_EXPLOSIONS 10
#define IDB_ITEMS 11
#define IDB_PLAYERS 12
#define IDM_BATTLE 20
#define IDM_LEVEL1 200
#define IDW_PLANT 21
#define IDW_EXPLODE 22
#define IDW_PICKUP 23
#define IDW_SKULL 24
#define IDW_DEATH 25
#define IDW_PUNCH 26
#define IDW_SLIDE 27
#define IDW_HARDHIT 28
#define IDW_BOUNCE 29
#define IDW_JUMP 30

#else

#define IDI_ICON1 "resources/icon.ico"
#define IDB_ICON2 "resources/icon.png"
#define IDB_TEXT "resources/text.png"
#define IDB_TILESET_1 "resources/tileset_1.png"
#define IDB_TILESET_2 "resources/tileset_2.png"
#define IDB_TILESET_3 "resources/tileset_3.png"
#define IDB_TILESET_4 "resources_tileset_4.png"
#define IDB_EXPLOSIONS "resources/explosions.png"
#define IDB_ITEMS "resources/items.png"
#define IDB_PLAYERS "resources/players.png"
#define IDM_BATTLE ""
#define IDM_LEVEL1 ""
#define IDW_PLANT ""
#define IDW_EXPLODE ""
#define IDW_PICKUP ""
#define IDW_SKULL ""
#define IDW_DEATH ""
#define IDW_PUNCH ""
#define IDW_SLIDE ""
#define IDW_HARDHIT ""
#define IDW_BOUNCE ""
#define IDW_JUMP ""

#endif

#ifdef __cplusplus

#include <SDL2/SDL.h>

#define TILE(x, y) {(x) * 16, (y) * 16, 16, 16}

extern SDL_Surface *icon_surface;

extern SDL_Texture *text_texture;
extern SDL_Texture *tileset_1_texture;
extern SDL_Texture *tileset_2_texture;
extern SDL_Texture *tileset_3_texture;
extern SDL_Texture *tileset_4_texture;
extern SDL_Texture *explosions_texture;
extern SDL_Texture *items_texture;
extern SDL_Texture *players_texture;

#ifdef _WINDOWS_
SDL_RWops *getResourceRW(int res_id, const char *type);
#else
#define getResourceRW(res_id, type) SDL_RWFromFile(res_id, "rb")
#endif

void loadResources(SDL_Renderer *renderer);

void clearResources();

#endif // __cplusplus

#endif // __RESOURCES_H__
