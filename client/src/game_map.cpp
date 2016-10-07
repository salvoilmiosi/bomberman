#include "game_map.h"

#include <cstring>
#include <cstdio>

#include "main.h"
#include "resources.h"

game_map::game_map() {
}

game_map::~game_map() {
    delete[] tiles;
}

tile *game_map::getTile(int x, int y) {
    if (x < 0 || x >= width) {
        return nullptr;
    }
    if (y < 0 || y >= height) {
        return nullptr;
    }
    return tiles + width * y + x;
}

void game_map::render(SDL_Renderer *renderer) {
    SDL_Rect src_rect = {0, 0, 16, 16};
    SDL_Rect dst_rect = {0, 0, TILE_SIZE, TILE_SIZE};

    for (int y=0; y<height; ++y) {
        for (int x=0; x<width; ++x) {
            tile *t = getTile(x, y);
            tile *t_up = getTile(x, y-1);
            if (!t) continue;
            switch (t->type) {
            case tile::TILE_FLOOR:
            case tile::TILE_SPAWN:
                if (t_up && t_up->type != tile::TILE_FLOOR && t_up->type != tile::TILE_SPAWN) {
                    src_rect = {64, 0, 16, 16};
                } else {
                    src_rect = {48, 0, 16, 16};
                }
                break;
            case tile::TILE_WALL:
                src_rect = {16, 0, 16, 16};
                break;
            case tile::TILE_BREAKABLE:
            case tile::TILE_ITEM:
                src_rect = {32, 0, 16, 16};
                break;
            default:
                break;
                // should never happen
            }
            dst_rect.x = TILE_SIZE * x + left();
            dst_rect.y = TILE_SIZE * y + top();
            SDL_RenderCopy(renderer, tileset_texture, &src_rect, &dst_rect);
        }
    }
}

void game_map::readFromPacket(packet_ext &packet) {
    short w = packet.readShort();
    short h = packet.readShort();

    if (w != width || h != height) {
        if (tiles)
            delete tiles;
        tiles = new tile[w * h];
        memset(tiles, 0, w * h * sizeof(tile));
    }

    width = w;
    height = h;

    for (int y=0; y<h; ++y) {
        for (int x=0; x<w; ++x) {
            char type = packet.readChar();

            tile *t = getTile(x, y);
            if (t) {
                t->type = (tile::tile_type) type;
            }
        }
    }
}

int game_map::left() {
    return (WINDOW_WIDTH - width * TILE_SIZE) / 2;
}

int game_map::top() {
    return (WINDOW_HEIGHT - height * TILE_SIZE) / 2;
}
