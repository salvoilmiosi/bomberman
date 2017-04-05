#include "game_server.h"

#include "player.h"

#include "strings.h"

#include <iostream>

game_server::game_server(game_world &world) : world(world), voter(this) {
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

	while (!users.empty()) {
		kickUser(users.begin()->second, STRING("SERVER_CLOSED"));
	}

	repeater.clear();

	removeBots();

	SDLNet_UDP_Close(socket_serv);
	open = false;

	if (game_thread.joinable()) {
		game_thread.join();
	}
}

user &game_server::findUser(const IPaddress &address) {
	return users.at(addrLong(address));
}

user &game_server::findUserByID(int id) {
	for (auto &it : users) {
		auto &u = it.second;
		if (u.getID() == id) {
			return u;
		}
	}
	throw std::out_of_range("");
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
		sendRepeatPacket(packet, u.getAddress(), repeats);
	}
}

packet_ext game_server::snapshotPacket(bool is_add) {
	packet_ext packet(socket_serv);
	packet.writeInt(is_add ? SERV_ADD_ENT : SERV_SNAPSHOT);
	world.writeEntitiesToPacket(packet, is_add);
	return packet;
}

packet_ext game_server::mapPacket() {
	packet_ext packet(socket_serv);
	packet.writeInt(SERV_MAP);
	world.writeMapToPacket(packet);
	return packet;
}

int game_server::game_thread_run() {
	int tick_count = 0;
	while(open) {
		auto it = users.begin();
		while(it != users.end()) {
			auto &u = it->second;
			if (u.tick()) {
				packet_ext packet_self(socket_serv);
				packet_self.writeInt(SERV_SELF);
				uint16_t player_id = 0;
				if (u.getPlayer()) {
					player_id = u.getPlayer()->getID();
				}
				packet_self.writeShort(player_id);
				packet_self.writeShort(u.getID());
				packet_self.sendTo(u.getAddress());

				++it;
			} else {
				messageToAll(COLOR_YELLOW, STRING("CLIENT_TIMED_OUT", u.getName()));
				u.destroyPlayer();
				it = users.erase(it);
			}
		}

		packet_ext snapshot = snapshotPacket();
		sendToAll(snapshot);
		packet_ext m_packet = mapPacket();
		sendToAll(m_packet);

		world.tick();
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

void game_server::kickUser(user &u, const std::string &message) {
	packet_ext packet(socket_serv);
	packet.writeInt(SERV_KICK);
	packet.writeString(message);
	sendRepeatPacket(packet, u.getAddress());

	messageToAll(COLOR_YELLOW, STRING("CLIENT_WAS_KICKED", u.getName()));

	u.destroyPlayer();
	users.erase(addrLong(u.getAddress()));
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
		if (username == u.getName()) {
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
		bots.push_back(user_bot(this));
		world.addEntity(bots.back().createPlayer(world));
	}
}

void game_server::removeBots() {
	for (auto &it : bots) {
		it.destroyPlayer();
	}
	bots.clear();
}

int game_server::countUsers(bool include_bots) {
	int num_players = 0;
	for (auto &it : users) {
		if (it.second.getPlayer()) {
			++num_players;
		}
	}

	return include_bots ? num_players + bots.size() : num_players;
}

void game_server::setZone(map_zone zone) {
	world.setZone(zone);
}

void game_server::startGame() {
	world.startRound(countUsers());
}

void game_server::resetGame() {
	for (auto &it : users) {
		auto p = it.second.getPlayer();
		if (p) p->resetScore();
	}
	for (auto &b : bots) {
		auto p = b.getPlayer();
		if (p) p->resetScore();
	}
	world.restartGame();
	startGame();
}

bool game_server::gameStarted() {
	return world.roundStarted();
}

packet_ext game_server::scorePacket() {
	packet_ext packet(socket_serv);
	packet.writeInt(SERV_SCORE);
	for (auto &it : users) {
		auto &u = it.second;
		packet.writeString(u.getName());
		packet.writeShort(u.getPing());
		if (u.getPlayer()) {
			packet.writeChar(SCORE_PLAYER);
			packet.writeShort(u.getID());
			packet.writeChar(u.getPlayer()->getPlayerNum());
			packet.writeShort(u.getPlayer()->getVictories());
			packet.writeChar(u.getPlayer()->isAlive() ? 1 : 0);
		} else {
			packet.writeChar(SCORE_SPECTATOR);
			packet.writeShort(u.getID());
		}
	}
	for (auto &b : bots) {
		if (b.getPlayer()) {
			packet.writeString(b.getName());
			packet.writeShort(0);
			packet.writeChar(SCORE_BOT);
			packet.writeShort(0);
			packet.writeChar(b.getPlayer()->getPlayerNum());
			packet.writeShort(b.getPlayer()->getVictories());
			packet.writeChar(b.getPlayer()->isAlive() ? 1 : 0);
		}
	}
	return packet;
}

void game_server::handlePacket(packet_ext &packet) {
	uint32_t magic = packet.readInt();
	if (magic != MAGIC) {
		return;
	}

	using usr_fun = std::function<void(packet_ext &, user &)>;
	static std::map<uint32_t, usr_fun> userFunctions;

	if (userFunctions.empty()) {
		userFunctions[CMD_CHAT] = [this](packet_ext &packet, user &u) {
			std::string message = packet.readString();

			messageToAll(COLOR_WHITE, STRING("CLIENT_MESSAGE", u.getName(), message));
		};

		userFunctions[CMD_NAME] = [this](packet_ext &packet, user &u) {
			std::string newName = findNewName(packet.readString());

			messageToAll(COLOR_YELLOW, STRING("CLIENT_CHANGED_NAME", u.getName(), newName));

			u.setName(newName);
		};

		userFunctions[CMD_DISCONNECT] = [this](packet_ext &, user &u){
			messageToAll(COLOR_YELLOW, STRING("CLIENT_DISCONNECTED", u.getName()));

			u.destroyPlayer();
			users.erase(addrLong(u.getAddress()));
		};

		userFunctions[CMD_PONG] = [this](packet_ext &, user &u) {
			u.pongCmd();

			packet_ext packet(socket_serv);
			packet.writeInt(SERV_PING_MSECS);
			packet.writeShort(u.getPing());
			packet.sendTo(u.getAddress());
		};

		userFunctions[CMD_SCORE] = [this](packet_ext &from, user &u){
			scorePacket().sendTo(from.getAddress());
		};

		userFunctions[CMD_INPUT] = [this](packet_ext &packet, user &u){
			u.handleInputPacket(packet);
		};

		userFunctions[CMD_JOIN] = [this](packet_ext &packet, user &u){
			if (u.getPlayer()) return;

			if (countUsers() < MAX_PLAYERS) {
				world.addEntity(u.createPlayer(world));

				messageToAll(COLOR_YELLOW, STRING("CLIENT_JOINED", u.getName()));
			} else {
				messageToAll(COLOR_YELLOW, STRING("CLIENT_SPECTATING", u.getName()));
			}
		};

		userFunctions[CMD_LEAVE] = [this](packet_ext &packet, user &u){
			if (!u.getPlayer()) return;
			messageToAll(COLOR_YELLOW, STRING("CLIENT_SPECTATING", u.getName()));
			u.destroyPlayer();
		};

		userFunctions[CMD_VOTE] = [this](packet_ext &packet, user &u) {
			uint32_t vote_type = packet.readInt();

			uint32_t args = packet.readInt();

			voter.sendVote(u, vote_type, args);
		};

		userFunctions[CMD_KILL] = [this](packet_ext &packet, user &u){
			auto p = u.getPlayer();
			if (p) p->kill(nullptr);
		};
	}

	uint32_t command = packet.readInt();
	if (command == CMD_CONNECT) {
		handleConnect(packet);
	} else try {
		auto func = userFunctions.at(command);

		try {
			user &u = findUser(packet.getAddress());
			func(packet, u);
		} catch (std::out_of_range) {
			std::cerr << STRING("INVALID_IP", ipString(packet.getAddress())) << std::endl;
		}
	} catch (std::out_of_range) {
		std::cerr << STRING("INVALID_COMMAND", ipString(packet.getAddress()), command) << std::endl;
	}
}

void game_server::handleConnect(packet_ext &from) {
	try {
		findUser(from.getAddress());
		std::cerr << STRING("CLIENT_ALREADY_CONNECTED", ipString(from.getAddress())) << std::endl;
	} catch (std::out_of_range) {
		if (users.size() > MAX_USERS) {
			packet_ext packet(socket_serv);
			packet.writeInt(SERV_REJECT);
			packet.writeString(STRING("SERVER_FULL"));
			packet.sendTo(from.getAddress());
			return;
		}
		std::string username = findNewName(from.readString());

		auto &u = users.insert(std::make_pair(addrLong(from.getAddress()), user(this, from.getAddress(), username))).first->second;

		packet_ext packet(socket_serv);
		packet.writeInt(SERV_ACCEPT);
		packet.writeString(STRING("SERVER_WELCOME"));
		packet.sendTo(from.getAddress());

		snapshotPacket(true).sendTo(from.getAddress());

		messageToAll(COLOR_YELLOW, STRING("CLIENT_CONNECTED", u.getName()));
	}
}

packet_ext game_server::messagePacket(uint32_t color, const std::string &message) {
	packet_ext packet(socket_serv);
	packet.writeInt(SERV_MESSAGE);
	packet.writeString(message);
	packet.writeInt(color);

	std::cout << message << std::endl;

	return packet;
}
