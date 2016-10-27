#include "game_map.h"

#include <cstring>
#include <algorithm>
#include <random>
#include <cstdlib>

#include "main.h"
#include "ent_item.h"

#include "tile_trampoline.h"
#include "tile_belt.h"
#include "tile_item_spawner.h"

game_map::game_map(game_world *world) : world(world) {
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
    tiles = nullptr;
    width = 0;
    height = 0;
}

void game_map::tick() {
    for (auto ent : specials) {
        if (ent.second) ent.second->tick();
    }
}

void game_map::createMap(int w, int h, int num_players, map_zone m_zone) {
    clear();

    if (m_zone == ZONE_RANDOM) {
        m_zone = static_cast<map_zone>(random_engine() % 6 + 1);
    }

    width = w;
    height = h;
    zone = m_zone;

    tiles = new tile[w * h];
    memset(tiles, 0, w * h * sizeof(tile));

    std::vector<point> spawns;

    switch (zone) {
    case ZONE_WESTERN:
    case ZONE_DUEL:
        spawns = {{6, 5}, {width-7,5}, {6, height-6}, {width -7, height -6}};
        break;
    default:
        spawns = {{2, 1}, {width-3,1}, {2,height-2}, {width-3,height-2}};
        break;
    }
    std::shuffle(spawns.begin(), spawns.end(), random_engine);

    spawn_pts.clear();

    for (;num_players > 0; --num_players) {
        spawn_pts.push_back(spawns.back());
        spawns.pop_back();
    }

    switch (zone) {
    case ZONE_WESTERN:
    case ZONE_DUEL:
        for (int y = 5; y < height-5; ++y) {
            for (int x = 6; x < width-6; ++x) {
                tile *t = getTile(x, y);
                if (!t) continue;
                t->type = TILE_SPAWN;
            }
        }
        break;
    default:
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
    }

    std::vector<tile *> floor_tiles;
    for (int y=0; y<height; ++y) {
        for (int x=0; x<width; ++x) {
            tile *t = getTile(x, y);
            if (x<=1 || x>=width-2 || y==0 || y==height-1) {
                t->type = TILE_WALL;
                switch (zone) {
                case ZONE_RANDOM:
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
                    break;
                case ZONE_BELT:
                    if (x == 0 || x  == width - 1) {
                        if (y == 0 || y == height - 1) {
                            t->data = 5;
                        } else if ((y % 2) == 1) {
                            t->data = 3;
                        } else {
                            t->data = 6;
                        }
                    } else if (x == 1 || x == width - 2) {
                        if (y == 0 || y == height - 1) {
                            t->data = 1;
                        } else if ((y % 2) == 1) {
                            t->data = 4;
                        } else {
                            t->data = 7;
                        }
                    } else if (y == 0 || y == height - 1) {
                        t->data = 2;
                        if ((x > (width / 2) || x == (width / 2 - 1))) {
                            t->data |= 0x80;
                        }
                    }
                    if (x >= width - 2) {
                        t->data |= 0x80;
                    }
                    break;
                case ZONE_DUEL:
                    if (x == 0 || x == width - 1) {
                        if (y == 0) {
                            t->data = 8;
                        } else if (y == height - 1) {
                            t->data = 3;
                        } else {
                            t->data = (y % 2 == 1) ? 6 : 12;
                        }
                    } else if (x == 1 || x == width - 2) {
                        if (y == 0) {
                            t->data = 1;
                        } else {
                            t->data = (y % 2 == 1) ? 7 : 13;
                        }
                    } else if (y == 0) {
                        if (x <= 4 || x >= 11) {
                            t->data = 2;
                        } else {
                            t->data = x + 13;
                        }
                    }
                    if (y == height - 1 && x > 0) {
                        t->data = (x % 2 == 1) ? 4 : 5;
                    } else if (x >= width - 2) {
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

    static const std::map<map_zone, int> breakables_per_zone = {
        {ZONE_NORMAL, 80}, {ZONE_WESTERN, 65}, {ZONE_BOMB, 60}, {ZONE_JUMP, 55}, {ZONE_BELT, 55},
    };

    std::vector<tile *> breakables;
    auto it = breakables_per_zone.find(zone);
    if (it != breakables_per_zone.end()) {
        for (int i=0; i<it->second; ++i) {
            tile *t = floor_tiles.back();
            t->type = TILE_BREAKABLE;
            breakables.push_back(t);
            floor_tiles.pop_back();
        }
    }

    static const std::map<map_zone, std::map<item_type, int> > item_numbers_per_zone = {
        {ZONE_NORMAL, {{ITEM_BOMB, 7}, {ITEM_FIRE, 5}, {ITEM_ROLLERBLADE, 3}, {ITEM_KICK, 3}, {ITEM_PUNCH, 3}, {ITEM_SKULL, 2}, {ITEM_REDBOMB, 2} }},
        {ZONE_WESTERN, {{ITEM_BOMB, 4}, {ITEM_FIRE, 3}, {ITEM_ROLLERBLADE, 2}, {ITEM_FULL_FIRE, 2}, {ITEM_KICK, 3}, {ITEM_PUNCH, 3}, {ITEM_SKULL, 1} }},
        {ZONE_BOMB, {{ITEM_BOMB, 3}, {ITEM_FIRE, 5}, {ITEM_ROLLERBLADE, 3}, {ITEM_KICK, 3}, {ITEM_PUNCH, 3}, {ITEM_SKULL, 2}, {ITEM_REMOCON, 2} }},
        {ZONE_JUMP, {{ITEM_BOMB, 4}, {ITEM_FIRE, 4}, {ITEM_ROLLERBLADE, 3}, {ITEM_KICK, 3}, {ITEM_PUNCH, 3}, {ITEM_SKULL, 2} }},
        {ZONE_BELT, {{ITEM_BOMB, 5}, {ITEM_FIRE, 5}, {ITEM_ROLLERBLADE, 3}, {ITEM_KICK, 3}, {ITEM_PUNCH, 3}}},
    };

    std::shuffle(breakables.begin(), breakables.end(), random_engine);

    const auto zone_it = item_numbers_per_zone.find(zone);
    if (zone_it == item_numbers_per_zone.end()) goto end_items;

    for (auto item_pair : zone_it->second) {
        for (int i=0; i<item_pair.second; ++i) {
            tile *t = breakables.back();
            if (t) {
                t->data = item_pair.first;
            }
            breakables.pop_back();
            if (breakables.empty()) {
                goto end_items;
            }
        }
    }
    end_items:

    switch (zone) {
    case ZONE_JUMP:
    {
        int num_trampolines = 12 + random_engine() % 4;
        for (int i=0; i<num_trampolines; ++i) {
            tile *t = floor_tiles.back();
            specials[t] = new tile_trampoline(t, this);
            floor_tiles.pop_back();
        }
        break;
    }
    case ZONE_BELT:
        for (int x = 4; x <= width - 6; ++x) {
            tile *t = getTile(x, 3);
            specials[t] = new tile_belt(t, this, BELT_DIR_RIGHT);
        }
        for (int x = 5; x <= width - 5; ++x) {
            tile *t = getTile(x, height - 4);
            specials[t] = new tile_belt(t, this, BELT_DIR_LEFT);
        }
        for (int y = 4; y <= height - 4; ++y) {
            tile *t = getTile(4, y);
            specials[t] = new tile_belt(t, this, BELT_DIR_UP);
        }
        for (int y = 3; y <= height - 5; ++y) {
            tile *t = getTile(width - 5, y);
            specials[t] = new tile_belt(t, this, BELT_DIR_DOWN);
        }
        break;
    case ZONE_DUEL:
        createDuelMap();
        break;
    default:
        break;
    }
}

void game_map::createDuelMap() {
    bool corner_breakables[] = {
        true, false, false,
        true, false, true, false, true,
        false, true, true,
        true, true,
    };
    bool *break_it = corner_breakables;

    for (int y = 2; y <= 5; ++y) {
        for (int x = 4; x <= 8; ++x) {
            tile *t1 = getTile(x, y);
            tile *t2 = getTile(width - 1 - x, y);
            tile *t3 = getTile(x, height - 1 - y);
            tile *t4 = getTile(width - 1 - x, height - 1 - y);
            if (t1->type == TILE_FLOOR) {
                if (*break_it) {
                    t1->type = TILE_BREAKABLE;
                    t2->type = TILE_BREAKABLE;
                    t3->type = TILE_BREAKABLE;
                    t4->type = TILE_BREAKABLE;
                }
                ++break_it;
            }
        }
    }

    static const item_type border_items_h[] = {
        ITEM_BOMB, ITEM_BOMB, ITEM_ROLLERBLADE, ITEM_FULL_FIRE, ITEM_ROLLERBLADE, ITEM_BOMB, ITEM_BOMB,
    };

    static const item_type border_items_v[] = {
        ITEM_BOMB, ITEM_KICK, ITEM_PUNCH, ITEM_BOMB,
    };

    const item_type *item_it = border_items_h;
    for (int x = 2; x <= width - 3; x += 2) {
        tile *t1 = getTile(x, 1);
        specials[t1] = new tile_item_spawner(t1, this, *item_it);

        tile *t2 = getTile(x, height - 2);
        specials[t2] = new tile_item_spawner(t2, this, *item_it);

        ++item_it;
    }

    item_it = border_items_v;
    for (int y = 3; y <= height - 4; y += 2) {
        tile *t1 = getTile(2, y);
        specials[t1] = new tile_item_spawner(t1, this, *item_it);

        tile *t2 = getTile(width - 3, height - y - 1);
        specials[t2] = new tile_item_spawner(t2, this, *item_it);

        ++item_it;
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
        case TILE_BREAKABLE:
            packet.writeChar(TILE_BREAKABLE);
            packet.writeShort(0);
            break;
        default:
            packet.writeChar(t->type);
            packet.writeShort(t->data);
            break;
        }
    }
}
