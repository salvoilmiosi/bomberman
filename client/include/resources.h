#ifndef __RESOURCES_H__
#define __RESOURCES_H__

#if defined(_WIN32) || (! defined(__cplusplus))

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

#define IDI_ICON1 "resource/icon.ico"
#define IDB_ICON2 "resource/icon.png"
#define IDB_TEXT "resource/text.png"
#define IDB_TILESET_1 "resource/tileset_1.png"
#define IDB_TILESET_2 "resource/tileset_2.png"
#define IDB_TILESET_3 "resource/tileset_3.png"
#define IDB_TILESET_4 "resource/tileset_4.png"
#define IDB_EXPLOSIONS "resource/explosions.png"
#define IDB_ITEMS "resource/items.png"
#define IDB_PLAYERS "resource/players.png"
#define IDM_BATTLE "resource/sound/ogg/sbm-12.ogg"
#define IDW_PLANT "resource/sound/plant.wav"
#define IDW_EXPLODE "resource/sound/explode.wav"
#define IDW_PICKUP "resource/sound/pickup.wav"
#define IDW_SKULL "resource/sound/skull.wav"
#define IDW_DEATH "resource/sound/death.wav"
#define IDW_PUNCH "resource/sound/punch.wav"
#define IDW_SLIDE "resource/sound/slide.wav"
#define IDW_HARDHIT "resource/sound/hardhit.wav"
#define IDW_BOUNCE "resource/sound/bounce.wav"
#define IDW_JUMP "resource/sound/jump.wav"

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

#ifdef _WIN32
SDL_RWops *getResourceRW(int res_id, const char *type);
#else
#define getResourceRW(res_id, type) SDL_RWFromFile(res_id, "rb")
#endif

void loadResources(SDL_Renderer *renderer);

void clearResources();

#endif // __cplusplus

#endif // __RESOURCES_H__
