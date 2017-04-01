#include "game_server.h"

#include "player.h"

#include "strings.h"

#include <iostream>

game_server::game_server(game_world *world) : world(world), voter(this) {
	open = false;
}

game_server::~game_server() {
	closeServer();
}

bool game_server::openServer(uint16_t port) {
	if (open)
		return false;

	sock_set = SDLNet_AllocSocketSet(1);
	socket_serv = SDLNet_UDP_Open(port);

	repeater.setSocket(socket_serv);

	if (socket_serv) {
		SDLNet_UDP_AddSocket(sock_set, socket_serv);
		open = true;
		return true;
	}
	std::cerr << STRING("ERROR_CREATING_SERVER_SOCKET", SDLNet_GetError()) << std::endl;

	return false;
}

void game_server::closeServer() {
	if (!open)
		return;

	while(!users.empty()) {
		auto it = users.begin();
		if (it->second) {
			kickUser(it->second.get(), STRING("SERVER_CLOSED"));
		} else {
			users.erase(it);
		}
	}

	repeater.clear();

	removeBots();

	SDLNet_UDP_Close(socket_serv);
	open = false;

	if (game_thread.joinable()) {
		game_thread.join();
	}
}

user *game_server::findUser(const IPaddress &address) {
	try {
		return users.at(addrLong(address)).get();
	} catch (std::out_of_range) {
		return nullptr;
	}
}

user *game_server::findUserByID(int id) {
	for (auto &it : users) {
		auto &u = it.second;
		if (u && u->getID() == id) {
			return u.get();
		}
	}
	return nullptr;
}

void game_server::sendRepeatPacket(packet_ext &packet, const IPaddress &address, int repeats) {
	if (repeats > 1) {
		repeater.addPacket(packet, address, repeats);
	} else {
		packet.sendTo(address);
	}
}

void game_server::sendToAll(packet_ext &packet, int repeats) {
	for (auto &it : users) {
		auto &u = it.second;
		sendRepeatPacket(packet, u->getAddress(), repeats);
	}
}

packet_ext game_server::snapshotPacket(bool is_add) {
	packet_ext packet(socket_serv);
	packet.writeInt(is_add ? SERV_ADD_ENT : SERV_SNAPSHOT);
	world->writeEntitiesToPacket(packet, is_add);
	return packet;
}

packet_ext game_server::mapPacket() {
	packet_ext packet(socket_serv);
	packet.writeInt(SERV_MAP);
	world->writeMapToPacket(packet);
	return packet;
}

int game_server::game_thread_run() {
	int tick_count = 0;
	while(open) {
		auto it = users.begin();
		while(it != users.end()) {
			auto &u = it->second;
			if (u->tick()) {
				packet_ext packet_self(socket_serv);
				packet_self.writeInt(SERV_SELF);
				uint16_t player_id = 0;
				if (u->getPlayer()) {
					player_id = u->getPlayer()->getID();
				}
				packet_self.writeShort(player_id);
				packet_self.writeShort(u->getID());
				packet_self.sendTo(u->getAddress());

				++it;
			} else {
				messageToAll(COLOR_YELLOW, STRING("CLIENT_TIMED_OUT", u->getName()));
				u->destroyPlayer();
				it = users.erase(it);
			}
		}

		packet_ext snapshot = snapshotPacket();
		sendToAll(snapshot);
		packet_ext m_packet = mapPacket();
		sendToAll(m_packet);

		world->tick();
		voter.tick();

		SDL_Delay(1000 / TICKRATE);

		++tick_count;
	}
	std::cout << "Game server thread out" << std::endl;
	return 0;
}

void game_server::sendPingPacket(const IPaddress &address) {
	packet_ext packet(socket_serv);
	packet.writeInt(SERV_PING);
	packet.sendTo(address);
}

int game_server::receive(packet_ext &packet) {
	int numready = SDLNet_CheckSockets(sock_set, TIMEOUT);
	if (numready > 0) {
		return packet.receive();
	} else if (numready < 0 && errno) {
		std::cerr << STRING("ERROR_CHECKING_SOCKET", numready, SDLNet_GetError()) << std::endl;
	}
	return 0;
}

int game_server::run() {
	game_thread = std::thread([this]{
		game_thread_run();
	});

	while (open) {
		packet_ext packet(socket_serv, true);
		repeater.sendPackets();
		int err = receive(packet);
		if (err > 0) {
			handlePacket(packet);
		} else if (err < 0) {
			std::cerr << STRING("ERROR_RECEIVING_PACKET", err, SDLNet_GetError()) << std::endl;
		}
	}

	return 0;
}

void game_server::sendAddPacket(entity *ent) {
	packet_ext packet(socket_serv);
	packet.writeInt(SERV_ADD_ENT);
	ent->writeToPacket(packet);
	sendToAll(packet, 5);
}

void game_server::sendRemovePacket(entity *ent) {
	packet_ext packet(socket_serv);
	packet.writeInt(SERV_REM_ENT);
	packet.writeShort(ent->getID());
	sendToAll(packet, 5);
}

void game_server::sendResetPacket() {
	packet_ext packet(socket_serv);
	packet.writeInt(SERV_RESET);
	sendToAll(packet, 5);
}

void game_server::kickUser(user *u, const std::string &message) {
	if (!u) return;

	packet_ext packet(socket_serv);
	packet.writeInt(SERV_KICK);
	packet.writeString(message.c_str());
	sendRepeatPacket(packet, u->getAddress());

	messageToAll(COLOR_YELLOW, STRING("CLIENT_WAS_KICKED", u->getName()));

	u->destroyPlayer();
	users.erase(addrLong(u->getAddress()));
}

void game_server::sendSoundPacket(uint8_t sound_id) {
	packet_ext packet(socket_serv);
	packet.writeInt(SERV_SOUND);
	packet.writeChar(sound_id);
	sendToAll(packet);
}

std::string game_server::findNewName(std::string username) {
	for (auto &it : users) {
		auto &u = it.second;
		if (u && username == u->getName()) {
			username += '\'';
		}
	}
	return username;
}

void game_server::addBots(int num_bots) {
	for (;num_bots>0; --num_bots) {
		if (countUsers(true) >= MAX_PLAYERS) {
			break;
		}
		auto b = std::make_unique<user_bot>(this);
		world->addEntity(b->createPlayer(world));
		bots.push_back(std::move(b));
	}
}

void game_server::removeBots() {
	for (auto &it : bots) {
		if (it)
			it->destroyPlayer();
	}
	bots.clear();
}

void game_server::connectCmd(packet_ext &from) {
	if (findUser(from.getAddress())) {
		std::cerr << STRING("CLIENT_ALREADY_CONNECTED", ipString(from.getAddress())) << std::endl;
		return;
	}

	if (users.size() > MAX_USERS) {
		packet_ext packet(socket_serv);
		packet.writeInt(SERV_REJECT);
		packet.writeString(STRING("SERVER_FULL").c_str());
		packet.sendTo(from.getAddress());
		return;
	}
	std::string username = findNewName(from.readString());

	auto u = std::make_unique<user>(this, from.getAddress(), username);

	packet_ext packet(socket_serv);
	packet.writeInt(SERV_ACCEPT);
	packet.writeString(STRING("SERVER_WELCOME").c_str());
	packet.sendTo(from.getAddress());

	snapshotPacket(true).sendTo(from.getAddress());

	messageToAll(COLOR_YELLOW, STRING("CLIENT_CONNECTED", u->getName()).c_str());
	users[addrLong(from.getAddress())] = std::move(u);
}

void game_server::joinCmd(packet_ext &from) {
	user *u = findUser(from.getAddress());
	if (!u) return;
	if (u->getPlayer()) return;

	if (countUsers() < MAX_PLAYERS) {
		world->addEntity(u->createPlayer(world));

		messageToAll(COLOR_YELLOW, STRING("CLIENT_JOINED", u->getName()));
	} else {
		messageToAll(COLOR_YELLOW, STRING("CLIENT_SPECTATING", u->getName()));
	}
}

void game_server::leaveCmd(packet_ext &from) {
	user *u = findUser(from.getAddress());
	if (!u) return;
	if (!u->getPlayer()) return;

	messageToAll(COLOR_YELLOW, STRING("CLIENT_SPECTATING", u->getName()));
	u->destroyPlayer();
}

int game_server::countUsers(bool include_bots) {
	int num_players = 0;
	for (auto &it : users) {
		if (it.second->getPlayer()) {
			++num_players;
		}
	}

	return include_bots ? num_players + bots.size() : num_players;
}

void game_server::setZone(map_zone zone) {
	world->setZone(zone);
}

void game_server::startGame() {
	world->startRound(countUsers());
}

void game_server::resetGame() {
	for (auto &it : users) {
		if (it.second) {
			auto p = it.second->getPlayer();
			if (p) p->resetScore();
		}
	}
	for (auto &b : bots) {
		if (b) {
			auto p = b->getPlayer();
			if (p) p->resetScore();
		}
	}
	world->restartGame();
	startGame();
}

bool game_server::gameStarted() {
	return world->roundStarted();
}

void game_server::chatCmd(packet_ext &packet) {
	user *u = findUser(packet.getAddress());
	if (!u) {
		std::cerr << STRING("INVALID_IP", ipString(packet.getAddress())) << std::endl;
		return;
	}

	char *message = packet.readString();

	messageToAll(COLOR_WHITE, STRING("CLIENT_MESSAGE", u->getName(), message));
}

void game_server::nameCmd(packet_ext &packet) {
	user *u = findUser(packet.getAddress());
	if (!u) {
		std::cerr << STRING("INVALID_IP", ipString(packet.getAddress())) << std::endl;
		return;
	}

	std::string newName = findNewName(packet.readString());

	messageToAll(COLOR_YELLOW, STRING("CLIENT_CHANGED_NAME", u->getName(), newName.c_str()));

	u->setName(newName);
}

void game_server::disconnectCmd(packet_ext &packet) {
	user *u = findUser(packet.getAddress());
	if (!u) {
		std::cerr << STRING("INVALID_IP", ipString(packet.getAddress())) << std::endl;
		return;
	}

	messageToAll(COLOR_YELLOW, STRING("CLIENT_DISCONNECTED", u->getName()));

	u->destroyPlayer();
	users.erase(addrLong(packet.getAddress()));
}

void game_server::pongCmd(packet_ext &from) {
	user *u = findUser(from.getAddress());
	if (!u) {
		std::cerr << STRING("INVALID_IP", ipString(from.getAddress())) << std::endl;
		return;
	}

	u->pongCmd();

	packet_ext packet(socket_serv);
	packet.writeInt(SERV_PING_MSECS);
	packet.writeShort(u->getPing());
	packet.sendTo(u->getAddress());

	//printf("(%s) %s pings %d msecs\n", ipString(address), u->getName(), u->getPing());
}

packet_ext game_server::scorePacket() {
	packet_ext packet(socket_serv);
	packet.writeInt(SERV_SCORE);
	for (auto &it : users) {
		auto &u = it.second;
		packet.writeString(u->getName());
		packet.writeShort(u->getPing());
		if (u->getPlayer()) {
			packet.writeChar(SCORE_PLAYER);
			packet.writeShort(u->getID());
			packet.writeChar(u->getPlayer()->getPlayerNum());
			packet.writeShort(u->getPlayer()->getVictories());
			packet.writeChar(u->getPlayer()->isAlive() ? 1 : 0);
		} else {
			packet.writeChar(SCORE_SPECTATOR);
			packet.writeShort(u->getID());
		}
	}
	for (auto &b : bots) {
		if (b->getPlayer()) {
			packet.writeString(b->getName());
			packet.writeShort(0);
			packet.writeChar(SCORE_BOT);
			packet.writeShort(0);
			packet.writeChar(b->getPlayer()->getPlayerNum());
			packet.writeShort(b->getPlayer()->getVictories());
			packet.writeChar(b->getPlayer()->isAlive() ? 1 : 0);
		}
	}
	return packet;
}

void game_server::scoreCmd(packet_ext &from) {
	scorePacket().sendTo(from.getAddress());
}

void game_server::inputCmd(packet_ext &packet) {
	user *u = findUser(packet.getAddress());
	if (!u) {
		std::cerr << STRING("INVALID_IP", ipString(packet.getAddress())) << std::endl;
		return;
	}

	u->handleInputPacket(packet);
}

void game_server::voteCmd(packet_ext &packet) {
	user *u = findUser(packet.getAddress());
	if (!u) {
		std::cerr << STRING("INVALID_IP", ipString(packet.getAddress())) << std::endl;
		return;
	}

	uint32_t vote_type = packet.readInt();

	uint32_t args = packet.readInt();

	voter.sendVote(u, vote_type, args);
}

void game_server::killCmd(packet_ext &packet) {
	user *u = findUser(packet.getAddress());
	if (!u) {
		std::cerr << STRING("INVALID_IP", ipString(packet.getAddress())) << std::endl;
		return;
	}

	auto p = u->getPlayer();
	if (p) p->kill(nullptr);
}

void game_server::handlePacket(packet_ext &packet) {
	uint32_t magic = packet.readInt();
	if (magic != MAGIC) {
		//fprintf(stderr, "%s: Invalid magic # %0#8x\n", ipString(packet.getAddress()), magic);
		return;
	}

	uint32_t command = packet.readInt();
	switch(command) {
	case CMD_CONNECT:
		connectCmd(packet);
		break;
	case CMD_CHAT:
		chatCmd(packet);
		break;
	case CMD_NAME:
		nameCmd(packet);
		break;
	case CMD_DISCONNECT:
		disconnectCmd(packet);
		break;
	case CMD_PONG:
		pongCmd(packet);
		break;
	case CMD_SCORE:
		scoreCmd(packet);
		break;
	case CMD_INPUT:
		inputCmd(packet);
		break;
	case CMD_JOIN:
		joinCmd(packet);
		break;
	case CMD_LEAVE:
		leaveCmd(packet);
		break;
	case CMD_VOTE:
		voteCmd(packet);
		break;
	case CMD_KILL:
		killCmd(packet);
		break;
	default:
		fprintf(stderr, "%s: Invalid command %0#8x\n", ipString(packet.getAddress()), command);
		break;
	}
}

packet_ext game_server::messagePacket(uint32_t color, const std::string &message) {
	packet_ext packet(socket_serv);
	packet.writeInt(SERV_MESSAGE);
	packet.writeString(message.c_str());
	packet.writeInt(color);

	std::cout << message << std::endl;

	return packet;
}
