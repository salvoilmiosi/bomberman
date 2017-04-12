#ifndef __ENTITY_H__
#define __ENTITY_H__

#include "packet_io.h"

enum entity_type {
	TYPE_NONE,
	TYPE_PLAYER,
	TYPE_BOMB,
	TYPE_EXPLOSION,
	TYPE_BROKEN_WALL,
	TYPE_ITEM
};

class entity {
private:
	const entity_type type;

	uint16_t id;

	bool destroyed;

protected:
	bool do_send_updates;

	class game_world &world;

public:
	entity(class game_world &world, const entity_type type) : type(type), world(world) {
		static uint16_t max_id = 1;
		id = max_id++;
		destroyed = false;
		do_send_updates = true;
	}

	virtual ~entity() {};

	virtual void tick() = 0;

	uint16_t getID() {
		return id;
	}

	const entity_type getType() {
		return type;
	}

	virtual const uint8_t getTileX() = 0;
	virtual const uint8_t getTileY() = 0;

	bool isNotDestroyed() {
		return !destroyed;
	}

	void destroy() {
		destroyed = true;
	}

	void writeToPacket(packet_ext &packet) {
		packet.writeShort(getID());
		packet.writeChar(getType());
		packet.writeByteArray(toByteArray());
	}

	bool doSendUpdates() {
		return do_send_updates;
	}

	virtual byte_array toByteArray() = 0;
};

#endif // __ENTITY_H__