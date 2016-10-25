#ifndef __PACKET_REPEATER_H__
#define __PACKET_REPEATER_H__

#include "packet_io.h"

#include <deque>

static const int DEFAULT_REPEATS = 5;

class packet_repeater {
private:
	struct repeated {
		packet_ext *packet;
		IPaddress address;

		int packet_id;
		int repeats;
	};

	std::deque<repeated> packets;

	UDPsocket socket;

	static int packet_id;

public:
	virtual ~packet_repeater();

	void clear();

	void setSocket(UDPsocket sock) {
		socket = sock;
	}

	void addPacket(packet_ext &packet, const IPaddress &address, int repeats);

	void sendPackets();
};

#endif
