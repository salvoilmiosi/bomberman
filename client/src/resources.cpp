#include "resources.h"

#include <SDL2/SDL_image.h>
#include <cstdio>

#ifdef _WINDOWS_
#define RES_ID_TYPE int
#include <windows.h>
#else
#define RES_ID_TYPE const char *
#endif

SDL_Texture *text_texture = nullptr;

SDL_Texture *tileset_1_texture = nullptr;
SDL_Texture *tileset_2_texture = nullptr;
SDL_Texture *tileset_3_texture = nullptr;
SDL_Texture *tileset_4_texture = nullptr;
SDL_Texture *explosions_texture = nullptr;
SDL_Texture *items_texture = nullptr;
SDL_Texture *players_texture = nullptr;

SDL_Surface *icon_surface = nullptr;

#ifdef _WINDOWS_
SDL_RWops *getResourceRW(int res_id, const char *type) {
    HMODULE hModule = GetModuleHandle(nullptr);
    HRSRC hRes = FindResource(hModule, MAKEINTRESOURCE(res_id), type);

    if (!hRes) {
        fprintf(stderr, "Can't load resource %d\n", res_id);
        return nullptr;
    }

    unsigned int res_size = SizeofResource(hModule, hRes);

    HGLOBAL hgRes = LoadResource(hModule, hRes);
    unsigned char* res_data = reinterpret_cast<unsigned char *>(LockResource(hgRes));

    return SDL_RWFromConstMem(res_data, res_size);
}
#endif

SDL_Surface *loadSurface(RES_ID_TYPE res_id) {
    return IMG_Load_RW(getResourceRW(res_id, "PNG"), 1);
}

SDL_Texture *loadTexture(SDL_Renderer *renderer, RES_ID_TYPE res_id) {
    SDL_Surface *surf = loadSurface(res_id);
    if (!surf) return nullptr;
    SDL_Texture *text = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_FreeSurface(surf);
    return text;
}

void loadResources(SDL_Renderer *renderer) {
    icon_surface = loadSurface(IDB_ICON2);

    text_texture = loadTexture(renderer, IDB_TEXT);
    tileset_1_texture = loadTexture(renderer, IDB_TILESET_1);
    tileset_2_texture = loadTexture(renderer, IDB_TILESET_2);
    tileset_3_texture = loadTexture(renderer, IDB_TILESET_3);
    tileset_4_texture = loadTexture(renderer, IDB_TILESET_4);
    explosions_texture = loadTexture(renderer, IDB_EXPLOSIONS);
    items_texture = loadTexture(renderer, IDB_ITEMS);
    players_texture = loadTexture(renderer, IDB_PLAYERS);
}

void clearResources() {
    SDL_DestroyTexture(text_texture);
    SDL_DestroyTexture(tileset_1_texture);
    SDL_DestroyTexture(tileset_2_texture);
    SDL_DestroyTexture(tileset_3_texture);
    SDL_DestroyTexture(tileset_4_texture);
    SDL_DestroyTexture(explosions_texture);
    SDL_DestroyTexture(items_texture);
    SDL_DestroyTexture(players_texture);

    SDL_FreeSurface(icon_surface);
}
