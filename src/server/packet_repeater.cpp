#include "packet_repeater.h"

#include <cstdio>

#include "game_server.h"

int packet_repeater::packet_id = 0;

packet_repeater::~packet_repeater() {
	clear();
}

void packet_repeater::clear() {
	packets.clear();
}

void packet_repeater::addPacket(packet_ext &packet, const IPaddress &address, int repeats) {
	auto packet_rep = std::make_unique<packet_ext>(socket);
	packet_rep->writeInt(SERV_REPEAT);
	packet_rep->writeInt(packet_id);
	packet_rep->writeByteArray(packet);

	packet_rep->sendTo(address); // try first time instantly

	packets.push_back({std::move(packet_rep), address, packet_id, repeats});

	++packet_id;
}

void packet_repeater::sendPackets() {
	auto it = packets.begin();
	while (it != packets.end()) {
		it->packet->sendTo(it->address);
		if (-- it->repeats <= 0) {
			it = packets.erase(it);
		} else {
			++it;
		}
	}
}
