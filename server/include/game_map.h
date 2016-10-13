#ifndef __GAME_MAP_H__
#define __GAME_MAP_H__

#include <vector>
#include <map>

#include <packet_io.h>

static const uint8_t TILE_FLOOR = 0;
static const uint8_t TILE_SPAWN = 1;
static const uint8_t TILE_WALL = 2;
static const uint8_t TILE_BREAKABLE = 3;
static const uint8_t TILE_ITEM = 4;
static const uint8_t TILE_SPECIAL = 5;

static const uint8_t ZONE_NORMAL = 0;
static const uint8_t ZONE_WESTERN = 1;
static const uint8_t ZONE_BOMB = 2;
static const uint8_t ZONE_JUMP = 3;

static const int MAP_WIDTH = 17;
static const int MAP_HEIGHT = 13;

static const int TILE_SIZE = 100;

static const uint8_t SPECIAL_TRAMPOLINE = 1;

struct tile {
    uint8_t type;
    uint8_t data;
};

struct point {
    int x, y;
};

class tile_entity {
private:
    const uint8_t type;
    uint16_t id;
    tile *t_tile;

public:
    tile_entity(const uint8_t type, tile *t_tile) : type(type), t_tile(t_tile) {
        // TILE ENTITY DATA:
        // |type | id    | data     |
        // | --- | ----- | -------- |
        // | byte 0      | byte 1   |

        static uint16_t max_id = 1;
        id = max_id++;
        t_tile->data = (type & 0x7) << 5;
    }

    virtual ~tile_entity() {}

public:
    virtual void tick() = 0;

    uint8_t getType() const {
        return type;
    }

protected:
    void setData(uint8_t data) {
        t_tile->data = (t_tile->data & 0xe0) | (data & 0x1f);
    }

private:
    friend class game_map;
};

class game_map {
private:
    tile *tiles = nullptr;

    int width = 0;
    int height = 0;

    int zone;

    std::vector<point> spawn_pts;

    std::map<tile *, tile_entity *> specials;

public:
    game_map();
    virtual ~game_map();

public:
    void createMap(int w, int h, int num_players, int zone = 0);
    void clear();

    void tick();

    uint8_t getZone() const {
        return zone;
    }

    tile *getTile(int x, int y) const;

    int getTileX(tile *t) const;
    int getTileY(tile *t) const;

    tile_entity *getSpecial(tile *t) const;

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
