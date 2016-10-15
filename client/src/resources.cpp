#include "resources.h"

#include <SDL2/SDL_image.h>

#include <map>
#include <vector>
#include <fstream>

#include "chat.h"

using namespace std;

SDL_Texture *font_texture = nullptr;
SDL_Texture *tileset_1_texture = nullptr;
SDL_Texture *tileset_2_texture = nullptr;
SDL_Texture *tileset_3_texture = nullptr;
SDL_Texture *tileset_4_texture = nullptr;
SDL_Texture *explosions_texture = nullptr;
SDL_Texture *items_texture = nullptr;
SDL_Texture *players_texture = nullptr;

SDL_Surface *icon_surface = nullptr;

TTF_Font *resource_font;

static const int ID_MAXSIZE = 32;

struct resource {
	size_t size = 0;
	size_t ptr = 0;
	const char *filename;
};

map<string, resource> resFiles;

static unsigned int readInt(ifstream &ifs) {
	char data[4];
	ifs.read(data, 4);
	unsigned int num =
		(data[0] << 24 & 0xff000000) |
		(data[1] << 16 & 0x00ff0000) |
		(data[2] << 8 & 0x0000ff00) |
		(data[3] & 0x000000ff);
	return num;
}

bool openResourceFile(const char *filename) {
	ifstream ifs(filename, ios::binary);

	if (ifs.fail()) {
		return false;
	}

	if (readInt(ifs) != 0x255435f4) {
		return false;
	}

	int numRes = readInt(ifs);

	resource res;
	char res_id[ID_MAXSIZE];

	while (numRes > 0) {
		memset(&res, 0, sizeof(res));
		memset(res_id, 0, sizeof(res_id));

		ifs.read(res_id, ID_MAXSIZE);

		res.size = readInt(ifs);
		res.ptr = readInt(ifs);
		res.filename = filename;

		resFiles[res_id] = res;

		--numRes;
	}

	return true;
}

SDL_RWops *getResourceRW(const char *RES_ID) {
	auto it = resFiles.find(RES_ID);

	if (it != resFiles.end()) {
		resource &res = it->second;
		char *data = new char[res.size];

		ifstream ifs(res.filename, ios::binary);
		ifs.seekg(res.ptr);
		ifs.read(data, res.size);

		return SDL_RWFromConstMem(data, res.size);
	}
	return nullptr;
}

static SDL_Surface *loadImageFromResource(const char *RES_ID) {
    return IMG_LoadTyped_RW(getResourceRW(RES_ID), 1, "PNG");
}

SDL_Texture *loadTexture(SDL_Renderer *renderer, const char *res_id) {
    SDL_Surface *surf = loadImageFromResource(res_id);
    if (!surf) return nullptr;
    SDL_Texture *text = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_FreeSurface(surf);
    return text;
}

void loadResources(SDL_Renderer *renderer) {
    icon_surface = loadImageFromResource("IDB_ICON2");

    font_texture = loadTexture(renderer, "IDB_TEXT");
    tileset_1_texture = loadTexture(renderer, "IDB_TILESET_1");
    tileset_2_texture = loadTexture(renderer, "IDB_TILESET_2");
    tileset_3_texture = loadTexture(renderer, "IDB_TILESET_3");
    tileset_4_texture = loadTexture(renderer, "IDB_TILESET_4");
    explosions_texture = loadTexture(renderer, "IDB_EXPLOSIONS");
    items_texture = loadTexture(renderer, "IDB_ITEMS");
    players_texture = loadTexture(renderer, "IDB_PLAYERS");

    resource_font = TTF_OpenFontRW(getResourceRW("IDF_FONT1"), 1, CHAR_H);
}

void clearResources() {
    SDL_FreeSurface(icon_surface);

    SDL_DestroyTexture(font_texture);
    SDL_DestroyTexture(tileset_1_texture);
    SDL_DestroyTexture(tileset_2_texture);
    SDL_DestroyTexture(tileset_3_texture);
    SDL_DestroyTexture(tileset_4_texture);
    SDL_DestroyTexture(explosions_texture);
    SDL_DestroyTexture(items_texture);
    SDL_DestroyTexture(players_texture);

    TTF_CloseFont(resource_font);
}
