#include "game_map.h"

#include <cstring>
#include <algorithm>
#include <random>
#include <cstdlib>

#include "main.h"
#include "ent_item.h"

#include "tile_trampoline.h"

game_map::game_map() {
    tiles = nullptr;
    width = 0;
    height = 0;
}

game_map::~game_map() {
    clear();
}

void game_map::clear() {
    if (!tiles) {
        delete[] tiles;
    }
    for (auto it : specials) {
        delete it.second;
    }
    specials.clear();
}

void game_map::tick() {
    for (auto ent : specials) {
        if (ent.second) ent.second->tick();
    }
}

void game_map::createMap(int w, int h, int num_players, int m_zone) {
    width = w;
    height = h;
    zone = m_zone;

    clear();

    tiles = new tile[w * h];
    memset(tiles, 0, w * h * sizeof(tile));

    std::vector<point> spawns;

    switch (zone) {
    case ZONE_NORMAL:
    case ZONE_BOMB:
    case ZONE_JUMP:
        spawns = {{2, 1}, {width-3,1}, {2,height-2}, {width-3,height-2}};
        break;
    case ZONE_WESTERN:
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
    case ZONE_NORMAL:
    case ZONE_BOMB:
    case ZONE_JUMP:
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
    case ZONE_WESTERN:
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
                case ZONE_NORMAL:
                    if (x == 0 || x == width-1) {
                        t->data = 1;
                    } else {
                        t->data = 0;
                    }
                    break;
                case ZONE_WESTERN:
                    if (x == 0 || x == width - 1) {
                        if (y == 0) {
                            t->data = 1;
                        } else if (y == height - 2) {
                            t->data = 2;
                        } else if (y == height - 1) {
                            t->data = 3;
                        } else {
                            t->data = 4;
                        }
                    } else if (x == 1 || x == width - 2) {
                        if (y == 0) {
                            t->data = 5;
                        } else if (y == height - 2) {
                            t->data = 6;
                        } else if (y == height - 1) {
                            t->data = 7;
                        } else if (y == 1) {
                            t->data = 8;
                        } else {
                            t->data = 9;
                        }
                    } else if (y == 0) {
                        t->data = 10;
                    } else if (y == height - 1) {
                        t->data = 11;
                    }
                    if (x >= width - 2) {
                        t->data |= 0x80;
                    }
                    break;
                case ZONE_BOMB:
                    if (x == 0 || x == width - 1) {
                        if (y == 0) {
                            t->data = 1;
                        } else if (y == height - 1) {
                            t->data = 3;
                        } else {
                            t->data = 2;
                        }
                    } else if (x == 1 || x == width - 2) {
                        if (y == 0) {
                            t->data = 4;
                        } else if (y == height - 1) {
                            t->data = 6;
                        } else {
                            t->data = 5;
                        }
                    } else if (y == 0) {
                        t->data = 9;
                    } else if (y == height - 1) {
                        t->data = 8;
                    }
                    if (x >= width - 2) {
                        t->data |= 0x80;
                    }
                    break;
                case ZONE_JUMP:
                    if (x == 0) {
                        static const uint8_t sky_left[] = {2, 3, 4, 5, 6, 9, 10, 11, 3, 4, 13, 11, 3};
                        t->data = sky_left[y % 13];
                    } else if (x == width - 1) {
                        static const uint8_t sky_right[] = {12, 9, 10, 11, 3, 6, 9, 4, 12, 12, 13, 11, 2};
                        t->data = sky_right[y % 13];
                    } else if (x == 1 || x == width - 2) {
                        if (y == 0) {
                            t->data = 8;
                        } else if (y == 1) {
                            t->data = 7;
                        } else if (y == height - 1) {
                            t->data = 21;
                        } else {
                            t->data = 14;
                        }
                    } else if (y == 0) {
                        t->data = 1;
                    } else if (y == height - 1) {
                        t->data = 22;
                    }
                    if (x >= width - 2) {
                        t->data |= 0x80;
                    }
                }
            } else if (x % 2 == 1 && y % 2 == 0) {
                t->type = TILE_WALL;
            } else if (t->type == TILE_FLOOR) {
                floor_tiles.push_back(t);
            }
        }
    }

    std::shuffle(floor_tiles.begin(), floor_tiles.end(), random_engine);

    int breakables_per_zone[] = {80, 65, 60, 55};
    int num_breakables = breakables_per_zone[zone];
    std::vector<tile *> breakables;

    if (zone == ZONE_JUMP) {
        int num_trampolines = 12 + random_engine() % 4;
        while (num_trampolines>0) {
            tile *t = floor_tiles.back();
            t->type = TILE_SPECIAL;
            tile_trampoline *ent = new tile_trampoline(t);
            specials[t] = ent;
            floor_tiles.pop_back();

            --num_trampolines;
        }
    }

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
        case ZONE_NORMAL:
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
        case ZONE_WESTERN:
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
        case ZONE_BOMB:
            if (i < 3) {
                t->data = ITEM_BOMB;
            } else if (i < 8) {
                t->data = ITEM_FIRE;
            } else if (i < 11) {
                t->data = ITEM_ROLLERBLADE;
            } else if (i < 14) {
                t->data = ITEM_KICK;
            } else if (i < 17) {
                t->data = ITEM_PUNCH;
            } else if (i < 19) {
                t->data = ITEM_SKULL;
            } else if (i < 21) {
                t->data = ITEM_REMOCON;
            } else {
                return;
            }
            break;
        case ZONE_JUMP:
            if (i < 4) {
                t->data = ITEM_BOMB;
            } else if (i < 8) {
                t->data = ITEM_FIRE;
            } else if (i < 11) {
                t->data = ITEM_ROLLERBLADE;
            } else if (i < 14) {
                t->data = ITEM_KICK;
            } else if (i < 17) {
                t->data = ITEM_PUNCH;
            } else if (i < 19) {
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

tile_entity *game_map::getSpecial(tile *t) const {
    if (t->type != TILE_SPECIAL) return nullptr;

    auto it = specials.find(t);
    if (it != specials.end()) {
        return it->second;
    }
    return nullptr;
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
        switch (t->type) {
        case TILE_SPAWN:
            packet.writeChar(TILE_FLOOR);
            packet.writeChar(0);
            break;
        case TILE_ITEM:
            packet.writeChar(TILE_BREAKABLE);
            packet.writeChar(0);
            break;
        default:
            packet.writeChar(t->type);
            packet.writeChar(t->data);
            break;
        }
    }
}
