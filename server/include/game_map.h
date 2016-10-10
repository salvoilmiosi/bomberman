#ifndef __GAME_MAP_H__
#define __GAME_MAP_H__

#include <vector>

#include <packet_io.h>

static const uint8_t TILE_FLOOR = 0;
static const uint8_t TILE_SPAWN = 1;
static const uint8_t TILE_WALL = 2;
static const uint8_t TILE_BREAKABLE = 3;
static const uint8_t TILE_ITEM = 4;

struct tile {
    uint8_t type;
    uint8_t data;
};

struct point {
    int x, y;
};

static const int MAP_WIDTH = 17;
static const int MAP_HEIGHT = 13;

static const int TILE_SIZE = 100;

class game_map {
private:
    tile *tiles;

    int width;
    int height;

    int zone;

    std::vector<point> spawn_pts;

public:
    game_map();
    virtual ~game_map();

public:
    void createMap(int w, int h, int num_players, int zone = 0);

    uint8_t getZone() const {
        return zone;
    }

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
