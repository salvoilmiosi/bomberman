#ifndef __GAME_MAP_H__
#define __GAME_MAP_H__

#include <vector>

#include <packet_io.h>

struct tile {
    enum tile_type {
        TILE_FLOOR = 0, TILE_SPAWN, TILE_WALL, TILE_BREAKABLE, TILE_ITEM,
    };
    tile_type type;
    uint8_t item_type;
};

struct point {
    int x, y;
};

static const int MAP_WIDTH = 15;
static const int MAP_HEIGHT = 13;

static const int NUM_BREAKABLES = 80;
static const int NUM_ITEMS = 23;

static const int TILE_SIZE = 100;

class game_map {
private:
    tile *tiles;

    int width;
    int height;

    std::vector<point> spawn_pts;

public:
    game_map();
    virtual ~game_map();

public:
    void createMap(int w, int h, int num_players);

    tile *getTile(int x, int y) const;

    int getTileX(tile *t) const;
    int getTileY(tile *t) const;

    int getWidth() const {
        return width;
    }

    int getHeight() const {
        return height;
    }

    point getSpawnPt(unsigned int numt);

    void writeToPacket(packet_ext &packet);
};

#endif // __GAME_MAP_H__
