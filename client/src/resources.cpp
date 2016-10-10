#include "resources.h"

#include <SDL2/SDL_image.h>
#include <windows.h>
#include <cstdio>

SDL_Texture *text_texture = nullptr;

SDL_Texture *tileset_1_texture = nullptr;
SDL_Texture *tileset_2_texture = nullptr;
SDL_Texture *explosions_texture = nullptr;
SDL_Texture *items_texture = nullptr;
SDL_Texture *players_texture = nullptr;

SDL_Surface *icon_surface = nullptr;

SDL_Surface *loadSurface(int res_id) {
    HMODULE hModule = GetModuleHandle(nullptr);
    HRSRC hRes = FindResource(hModule, MAKEINTRESOURCE(res_id), "PNG");

    if (!hRes) {
        fprintf(stderr, "Can't load resource %d\n", res_id);
        return nullptr;
    }

    unsigned int res_size = SizeofResource(hModule, hRes);

    HGLOBAL hgRes = LoadResource(hModule, hRes);
    unsigned char* res_data = reinterpret_cast<unsigned char *>(LockResource(hgRes));

    return IMG_Load_RW(SDL_RWFromConstMem(res_data, res_size), 1);
}

SDL_Texture *loadTexture(SDL_Renderer *renderer, int res_id) {
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
    explosions_texture = loadTexture(renderer, IDB_EXPLOSIONS);
    items_texture = loadTexture(renderer, IDB_ITEMS);
    players_texture = loadTexture(renderer, IDB_PLAYERS);
}

void clearResources() {
    SDL_DestroyTexture(text_texture);
    SDL_DestroyTexture(tileset_1_texture);
    SDL_DestroyTexture(tileset_2_texture);
    SDL_DestroyTexture(explosions_texture);
    SDL_DestroyTexture(items_texture);
    SDL_DestroyTexture(players_texture);

    SDL_FreeSurface(icon_surface);
}
