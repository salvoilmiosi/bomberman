#ifndef __GAME_MAP_H__
#define __GAME_MAP_H__

#include <vector>
#include <map>

#include <packet_io.h>

enum tile_type {
    TILE_FLOOR,
    TILE_SPAWN,
    TILE_WALL,
    TILE_BREAKABLE,
    TILE_ITEM,
    TILE_SPECIAL
};

enum map_zone {
    ZONE_NORMAL,
    ZONE_WESTERN,
    ZONE_BOMB,
    ZONE_JUMP
};

enum special_type {
    SPECIAL_NONE,
    SPECIAL_TRAMPOLINE
};

static const int MAP_WIDTH = 17;
static const int MAP_HEIGHT = 13;

static const int TILE_SIZE = 100;

struct tile {
    tile_type type;
    uint8_t data;
};

struct point {
    int x, y;
};

class tile_entity {
private:
    const special_type type;
    uint16_t id;
    tile *t_tile;

public:
    tile_entity(const special_type type, tile *t_tile) : type(type), t_tile(t_tile) {
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

    map_zone zone;

    std::vector<point> spawn_pts;

    std::map<tile *, tile_entity *> specials;

public:
    game_map();
    virtual ~game_map();

public:
    void createMap(int w, int h, int num_players, map_zone zone);
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
