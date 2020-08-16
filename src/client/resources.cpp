#include "resources.h"

#include <SDL2/SDL_image.h>

#include "chat.h"

using namespace std;

SDL_Texture *font_texture = nullptr;
SDL_Texture *tileset_1_texture = nullptr;
SDL_Texture *tileset_2_texture = nullptr;
SDL_Texture *tileset_3_texture = nullptr;
SDL_Texture *tileset_4_texture = nullptr;
SDL_Texture *tileset_5_texture = nullptr;
SDL_Texture *tileset_6_texture = nullptr;
SDL_Texture *tileset_7_texture = nullptr;

SDL_Texture *explosions_texture = nullptr;
SDL_Texture *items_texture = nullptr;
SDL_Texture *players_texture = nullptr;
SDL_Texture *players_white_texture = nullptr;
SDL_Texture *player_icons_texture = nullptr;

SDL_Surface *icon_surface = nullptr;

TTF_Font *resource_font;

static inline SDL_Surface *loadImageFromResource(const char *RES_ID) {
    return IMG_LoadTyped_RW(getResourceRW(RES_ID), 1, "PNG");
}

SDL_Texture *loadTexture(SDL_Renderer *renderer, const char *res_id) {
    SDL_Surface *surf = loadImageFromResource(res_id);
    if (!surf) return nullptr;
    SDL_Texture *text = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_FreeSurface(surf);
    return text;
}

bool loadResources(SDL_Renderer *renderer) {
    icon_surface = loadImageFromResource("IDB_ICON2");

    font_texture = loadTexture(renderer, "IDB_TEXT");
    tileset_1_texture = loadTexture(renderer, "IDB_TILESET_1");
    tileset_2_texture = loadTexture(renderer, "IDB_TILESET_2");
    tileset_3_texture = loadTexture(renderer, "IDB_TILESET_3");
    tileset_4_texture = loadTexture(renderer, "IDB_TILESET_4");
    tileset_5_texture = loadTexture(renderer, "IDB_TILESET_5");
    tileset_6_texture = loadTexture(renderer, "IDB_TILESET_6");
    tileset_7_texture = loadTexture(renderer, "IDB_TILESET_7");

    explosions_texture = loadTexture(renderer, "IDB_EXPLOSIONS");
    items_texture = loadTexture(renderer, "IDB_ITEMS");
    players_texture = loadTexture(renderer, "IDB_PLAYERS");
    players_white_texture = loadTexture(renderer, "IDB_PLAYERS_WHITE");
    player_icons_texture = loadTexture(renderer, "IDB_PLAYER_ICONS");

    resource_font = TTF_OpenFontRW(getResourceRW("IDF_FONT"), 1, CHAR_H);

    return true;
}

void clearResources() {
    SDL_FreeSurface(icon_surface);

    SDL_DestroyTexture(font_texture);
    SDL_DestroyTexture(tileset_1_texture);
    SDL_DestroyTexture(tileset_2_texture);
    SDL_DestroyTexture(tileset_3_texture);
    SDL_DestroyTexture(tileset_4_texture);
    SDL_DestroyTexture(tileset_5_texture);
    SDL_DestroyTexture(tileset_6_texture);
    SDL_DestroyTexture(tileset_7_texture);

    SDL_DestroyTexture(explosions_texture);
    SDL_DestroyTexture(items_texture);
    SDL_DestroyTexture(players_texture);
    SDL_DestroyTexture(players_white_texture);
    SDL_DestroyTexture(player_icons_texture);

    TTF_CloseFont(resource_font);
}
