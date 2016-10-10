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

void game_map::createMap(int w, int h, int num_players, int m_zone) {
    width = w;
    height = h;
    zone = m_zone;

    if (tiles)
        delete[] tiles;

    tiles = new tile[w * h];
    memset(tiles, 0, w * h * sizeof(tile));

    std::vector<point> spawns;

    switch (zone) {
    case 0:
        spawns = {{2, 1}, {width-3,1}, {2,height-2}, {width-3,height-2}};
        break;
    case 1:
        spawns = {{6, 5}, {width-7,5}, {6, height-6}, {width -7, height -6}};
        break;
    default:
        return;
    }
    std::shuffle(spawns.begin(), spawns.end(), random_engine);

    spawn_pts.clear();

    for (;num_players > 0; --num_players) {
        spawn_pts.push_back(spawns.back());
        spawns.pop_back();
    }

    switch (zone) {
    case 0:
        for (const point &pt : spawn_pts) {
            for (int y = pt.y - 1; y <= pt.y + 1; ++y) {
                for (int x = pt.x - 1; x <= pt.x + 1; ++x) {
                    tile *t = getTile(x, y);
                    if (!t) continue;
                    t->type = TILE_SPAWN;
                }
            }
        }
        break;
    case 1:
        for (int y = 5; y < height-5; ++y) {
            for (int x = 6; x < width-6; ++x) {
                tile *t = getTile(x, y);
                if (!t) continue;
                t->type = TILE_SPAWN;
            }
        }
        break;
    default:
        return;
    }

    std::vector<tile *> floor_tiles;
    for (int y=0; y<height; ++y) {
        for (int x=0; x<width; ++x) {
            tile *t = getTile(x, y);
            if (x<=1 || x>=width-2 || y==0 || y==height-1) {
                t->type = TILE_WALL;
                switch (zone) {
                case 0:
                    if (x == 0 || x == width-1) {
                        t->data = 1;
                    } else {
                        t->data = 0;
                    }
                    break;
                case 1:
                    if (x == 0 || x == width - 1) {
                        if (y == 0) {
                            t->data = 0;
                        } else if (y == height - 2) {
                            t->data = 1;
                        } else if (y == height - 1) {
                            t->data = 2;
                        } else {
                            t->data = 3;
                        }
                    } else if (x == 1 || x == width - 2) {
                        if (y == 0) {
                            t->data = 4;
                        } else if (y == height - 2) {
                            t->data = 5;
                        } else if (y == height - 1) {
                            t->data = 6;
                        } else if (y == 1) {
                            t->data = 7;
                        } else {
                            t->data = 8;
                        }
                    } else if (y == 0) {
                        t->data = 9;
                    } else if (y == height - 1) {
                        t->data = 10;
                    } else {
                        t->data = 11;
                    }
                    if (x >= width - 2) {
                        t->data |= 0x80;
                    }
                    break;
                }
            } else if (x % 2 == 1 && y % 2 == 0) {
                t->type = TILE_WALL;
                t->data = 11;
            } else if (t->type == TILE_FLOOR) {
                floor_tiles.push_back(t);
            }
        }
    }

    std::shuffle(floor_tiles.begin(), floor_tiles.end(), random_engine);

    int num_breakables = 80;
    switch (zone) {
    case 0:
        num_breakables = 80;
        break;
    case 1:
        num_breakables = 65;
        break;
    }

    std::vector<tile *> breakables;
    for (int i=0; i<num_breakables; ++i) {
        tile *t = floor_tiles[i];
        t->type = TILE_BREAKABLE;
        breakables.push_back(t);
    }

    std::shuffle(breakables.begin(), breakables.end(), random_engine);
    for (int i=0; true; ++i) {
        tile *t = breakables[i];
        if (!t) break;
        t->type = TILE_ITEM;
        switch (zone) {
        case 0:
            if (i < 7) {
                t->data = ITEM_BOMB;
            } else if (i < 12) {
                t->data = ITEM_FIRE;
            } else if (i < 15) {
                t->data = ITEM_ROLLERBLADE;
            } else if (i < 18) {
                t->data = ITEM_KICK;
            } else if (i < 21) {
                t->data = ITEM_PUNCH;
            } else if (i < 23) {
                t->data = ITEM_SKULL;
            } else {
                return;
            }
            break;
        case 1:
            if (i < 4) {
                t->data = ITEM_BOMB;
            } else if (i < 7) {
                t->data = ITEM_FIRE;
            } else if (i < 9) {
                t->data = ITEM_ROLLERBLADE;
            } else if (i < 11) {
                t->data = ITEM_FULL_FIRE;
            } else if (i < 14) {
                t->data = ITEM_KICK;
            } else if (i < 17) {
                t->data = ITEM_PUNCH;
            } else if (i < 18) {
                t->data = ITEM_SKULL;
            } else {
                return;
            }
            break;
        default:
            return;
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
    packet.writeChar(zone);
    for (tile *t = tiles; t - tiles < width * height; ++t) {
        if (t->type == TILE_ITEM) {
            packet.writeChar(TILE_BREAKABLE);
            packet.writeChar(0);
        } else {
            packet.writeChar(t->type);
            packet.writeChar(t->data);
        }
    }
}
