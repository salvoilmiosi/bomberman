#include "resources.h"

#include <SDL2/SDL_image.h>
#include <windows.h>
#include <cstdio>

SDL_Texture *tileset_texture = nullptr;
SDL_Texture *items_texture = nullptr;
SDL_Texture *players_texture = nullptr;
SDL_Texture *text_texture = nullptr;
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

void loadResources(SDL_Renderer *renderer) {
    SDL_Surface *tileset_surface = loadSurface(IDB_TILESET);
    tileset_texture = SDL_CreateTextureFromSurface(renderer, tileset_surface);

    SDL_Surface *items_surface = loadSurface(IDB_ITEMS);
    items_texture = SDL_CreateTextureFromSurface(renderer, items_surface);

    SDL_Surface *players_surface = loadSurface(IDB_PLAYERS);
    players_texture = SDL_CreateTextureFromSurface(renderer, players_surface);

    SDL_Surface *text_surface = loadSurface(IDB_TEXT);
    text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);

    SDL_FreeSurface(tileset_surface);
    SDL_FreeSurface(items_surface);
    SDL_FreeSurface(players_surface);
    SDL_FreeSurface(text_surface);

    icon_surface = loadSurface(IDB_ICON2);
}

void clearResources() {
    SDL_DestroyTexture(tileset_texture);
    SDL_DestroyTexture(items_texture);
    SDL_DestroyTexture(players_texture);
    SDL_DestroyTexture(text_texture);

    SDL_FreeSurface(icon_surface);
}
