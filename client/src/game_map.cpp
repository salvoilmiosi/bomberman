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
            case TILE_FLOOR:
            case TILE_SPAWN:
                if (zone == 0) {
                    if (t_up && t_up->type != TILE_FLOOR && t_up->type != TILE_SPAWN) {
                        src_rect = TILE(4, 0);
                    } else {
                        src_rect = TILE(3, 0);
                    }
                } else if (zone == 1) {
                    if (t_up && t_up->type == TILE_WALL && t_up->data == 11) {
                        src_rect = TILE(1, 3);
                    } else if (t_up && t_up->type == TILE_BREAKABLE) {
                        src_rect = TILE(2, 3);
                    } else {
                        src_rect = TILE(3, 2);
                    }
                }
                break;
            case TILE_WALL:
                if (zone == 0) {
                    if (t->data == 1) {
                        src_rect = TILE(0, 0);
                    } else {
                        src_rect = TILE(1, 0);
                    }
                } else if (zone == 1) {
                    uint8_t tx = (t->data & 0x0f) % 5;
                    uint8_t ty = (t->data & 0x0f) / 5;
                    src_rect = TILE(tx, ty);
                }
                break;
            case TILE_BREAKABLE:
                if (zone == 0) {
                    src_rect = TILE(2, 0);
                } else if (zone == 1) {
                    if (t_up && (t_up->type == TILE_BREAKABLE || (t_up->type == TILE_WALL && t_up->data == 11))) {
                        src_rect = TILE(4, 2);
                    } else {
                        src_rect = TILE(2, 2);
                    }
                }
                break;
            default:
                break;
                // should never happen
            }
            dst_rect.x = TILE_SIZE * x + left();
            dst_rect.y = TILE_SIZE * y + top();

            SDL_Texture *tileset = tileset_1_texture;
            switch (zone) {
            case 0:
                tileset = tileset_1_texture;
                break;
            case 1:
                tileset = tileset_2_texture;
                break;
            default:
                return;
            }

            SDL_RendererFlip flip = SDL_FLIP_NONE;
            if (t->data & 0x80) {
                flip = (SDL_RendererFlip) (flip | SDL_FLIP_HORIZONTAL);
            }
            if (t->data & 0x40) {
                flip = (SDL_RendererFlip) (flip | SDL_FLIP_VERTICAL);
            }

            SDL_RenderCopyEx(renderer, tileset, &src_rect, &dst_rect, 0, 0, flip);
        }
    }
}

void game_map::readFromPacket(packet_ext &packet) {
    short w = packet.readShort();
    short h = packet.readShort();
    zone = packet.readChar();

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
            uint8_t type = packet.readChar();
            uint8_t data = packet.readChar();

            tile *t = getTile(x, y);
            if (t) {
                t->type = type;
                t->data = data;
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
