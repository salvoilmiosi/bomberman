#ifndef __PACKET_REPEATER_H__
#define __PACKET_REPEATER_H__

#include "packet_io.h"

#include <deque>

static const int SERV_REPEAT = str2int("REPT");

class packet_repeater {
private:
	struct repeated {
		int packet_id;
		packet_ext *packet;
		IPaddress address;
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