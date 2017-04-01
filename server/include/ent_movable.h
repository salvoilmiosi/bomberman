#ifndef __ENT_MOVABLE_H__
#define __ENT_MOVABLE_H__

#include "game_world.h"

class ent_movable : public entity {
protected:
	float fx = 0.f;
	float fy = 0.f;
	float fz = 0.f;

public:
	ent_movable(game_world &world, entity_type type);

	virtual bool move(float dx, float dy) = 0;

	const float getX() {
        return fx;
	}

	const float getY() {
        return fy;
	}

    const uint8_t getTileX() {
        return fx / TILE_SIZE + 0.5f;
    }

    const uint8_t getTileY() {
        return fy / TILE_SIZE + 0.5f;
    }

protected:
	bool std_move(float dx, float dy, uint8_t flags = 0);
};

#endif
