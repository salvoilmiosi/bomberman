#include "game_map.h"

#include <cstring>
#include <algorithm>
#include <random>
#include <cstdlib>

#include "main.h"
#include "ent_item.h"

game_map::game_map() {
    tiles = nullptr;
    width = 0;
    height = 0;
}

game_map::~game_map() {
    delete[] tiles;
}

void game_map::createMap(int w, int h, int num_players) {
    width = w;
    height = h;

    if (tiles)
        delete[] tiles;

    tiles = new tile[w * h];
    memset(tiles, 0, w * h * sizeof(tile));

    std::vector<point> spawns = {{1,1},{width-2,1},{1,height-2},{width-2,height-2}};
    std::shuffle(spawns.begin(), spawns.end(), random_engine);

    spawn_pts.clear();

    for (;num_players > 0; --num_players) {
        spawn_pts.push_back(spawns.back());
        spawns.pop_back();
    }

    for (const point &pt : spawn_pts) {
        for (int y = pt.y - 1; y <= pt.y + 1; ++y) {
            for (int x = pt.x - 1; x <= pt.x + 1; ++x) {
                tile *t = getTile(x, y);
                if (!t) continue;
                t->type = tile::TILE_SPAWN;
            }
        }
    }

    std::vector<tile *> floor_tiles;
    for (int y=0; y<height; ++y) {
        for (int x=0; x<width; ++x) {
            tile *t = getTile(x, y);
            if (x==0 || x==width-1 || y==0 || y==height-1) {
                t->type = tile::TILE_WALL;
            } else if (x % 2 == 0 && y % 2 == 0) {
                t->type = tile::TILE_WALL;
            } else if (t->type == tile::TILE_FLOOR) {
                floor_tiles.push_back(t);
            }
        }
    }

    std::shuffle(floor_tiles.begin(), floor_tiles.end(), random_engine);

    std::vector<tile *> breakables;
    for (int i=0; i<NUM_BREAKABLES; ++i) {
        tile *t = floor_tiles[i];
        t->type = tile::TILE_BREAKABLE;
        breakables.push_back(t);
    }

    std::shuffle(breakables.begin(), breakables.end(), random_engine);
    for (int i=0; i<NUM_ITEMS; ++i) {
        tile *t = breakables[i];
        t->type = tile::TILE_ITEM;
        if (i < 7) {
            t->item_type = ITEM_BOMB;
        } else if (i < 12) {
            t->item_type = ITEM_FIRE;
        } else if (i < 15) {
            t->item_type = ITEM_ROLLERBLADE;
        } else if (i < 18) {
            t->item_type = ITEM_KICK;
        } else if (i < 21) {
            t->item_type = ITEM_PUNCH;
        } else if (i < 23) {
            t->item_type = ITEM_SKULL;
        }
    }
}

tile *game_map::getTile(int x, int y) const {
    if (x < 0 || x >= width) {
        return nullptr;
    }
    if (y < 0 || y >= height) {
        return nullptr;
    }
    return tiles + width * y + x;
}

int game_map::getTileX(tile *t) const {
    if (!t) return -1;
    return (t-tiles) % width;
}

int game_map::getTileY(tile *t) const {
    if (!t) return -1;
    return (t-tiles) / width;
}

point game_map::getSpawnPt(unsigned int num) {
    if (num >= spawn_pts.size()) {
        throw 0;
    }
    return spawn_pts[num];
}

void game_map::writeToPacket(packet_ext &packet) {
    packet.writeShort(width);
    packet.writeShort(height);
    for (tile *t = tiles; t - tiles < width * height; ++t) {
        if (t->type == tile::TILE_ITEM) {
            packet.writeChar((uint8_t) tile::TILE_BREAKABLE);
        } else {
            packet.writeChar((uint8_t) t->type);
        }
    }
}
