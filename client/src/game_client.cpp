#include "game_client.h"

#include "game_world.h"
#include "player.h"
#include "bindings.h"
#include "game_sound.h"
#include "listenserver.h"
#include "strings.h"

#include <algorithm>
#include <iostream>

game_client::game_client() : g_chat(this), g_score(this) {
	sock_set = SDLNet_AllocSocketSet(1);
}

game_client::~game_client() {
	if (sock_set) {
		SDLNet_FreeSocketSet(sock_set);
		sock_set = nullptr;
	}

	if (socket) {
		SDLNet_UDP_Close(socket);
		socket = nullptr;
	}

	if (receiver_thread.joinable()) {
		receiver_thread.join();
	}
}

bool game_client::connect(const char *address, uint16_t port) {
	if (socket) return false;

	socket = SDLNet_UDP_Open(0);

	if (!socket) {
		g_chat.addLine(COLOR_RED, STRING("ERROR_CREATING_SOCKET", SDLNet_GetError()));
		clear();
		return false;
	}

	SDLNet_ResolveHost(&server_ip, address, port);

	packet_ext packet(socket);
	packet.writeInt(CMD_CONNECT);
	packet.writeString(user_name.c_str());
	packet.sendTo(server_ip);

	SDLNet_UDP_AddSocket(sock_set, socket);

	if (SDLNet_CheckSockets(sock_set, TIMEOUT) <= 0) {
		g_chat.addLine(COLOR_RED, STRING("CONNECTION_TIMED_OUT", address, port));
		clear();
		return false;
	}

	packet_ext accepter(socket, true);
	if (accepter.receive() <= 0) {
		g_chat.addLine(COLOR_RED, STRING("SERVER_DOES_NOT_RESPOND", address, port, SDLNet_GetError()));
		clear();
		return false;
	}

	uint32_t magic = accepter.readInt();
	if (magic != MAGIC) {
		g_chat.addLine(COLOR_RED, STRING("ERROR_COULD_NOT_CONNECT"));
		clear();
		return false;
	}

	uint32_t type = accepter.readInt();
	char *message;
	switch (type) {
	case SERV_ACCEPT:
		message = accepter.readString();
		g_chat.addLine(COLOR_CYAN, STRING("SERVER_ACCEPTED", message));
		g_chat.addLine(COLOR_CYAN, STRING("SERVER_CONNECTED", address, port));

		sendJoinCmd();

		playMusic(music_battle);

		receiver_thread = std::thread([this]{
			receiver_run();
		});

		return true;
	case SERV_REJECT:
		message = accepter.readString();
		g_chat.addLine(COLOR_RED, STRING("SERVER_REJECTED", message));
		clear();
		return false;
	}

	return false;
}

void game_client::disconnect() {
	if (socket == nullptr) return;

	packet_ext packet(socket);
	packet.writeInt(CMD_DISCONNECT);
	packet.sendTo(server_ip);

	clear();

	g_chat.addLine(COLOR_CYAN, STRING("SERVER_DISCONNECTED"));
}

void game_client::quit() {
	disconnect();
	is_open = false;
}

void game_client::clear() {
	if (socket == nullptr) return;

	SDLNet_UDP_DelSocket(sock_set, socket);
	SDLNet_UDP_Close(socket);
	socket = nullptr;

	if (receiver_thread.joinable()) {
		receiver_thread.join();
	}

	world.clear();
	g_score.clear();

	if (isListenServerOpen()) {
		stopListenServer();
	}

	stopMusic();
}

bool game_client::sendInput(uint8_t cmd, bool down) {
	if (cmd == 0) return false;
	if (socket == nullptr) return false;

	packet_ext packet(socket);
	packet.writeInt(CMD_INPUT);
	packet.writeInt(str2int("KBRD"));
	packet.writeChar(cmd);
	packet.writeChar(down ? 1 : 0);
	return packet.sendTo(server_ip);
}

bool game_client::sendMouse(int x, int y) {
	if (socket == nullptr) return false;

	packet_ext packet(socket);
	packet.writeInt(CMD_INPUT);
	packet.writeInt(str2int("MOUS"));
	packet.writeInt(x);
	packet.writeInt(y);
	return packet.sendTo(server_ip);
}

void game_client::tick() {
	world.tick();
	g_chat.tick();
	g_score.tick();

	auto it = packet_repeat_ids.begin();
	while (it != packet_repeat_ids.end()) {
		if (SDL_GetTicks() - it->second > TIMEOUT) {
			it = packet_repeat_ids.erase(it);
		} else {
			++it;
		}
	}
}

int game_client::receiver_run() {
	int err_ret = 0;

	while (true) {
		int numready = SDLNet_CheckSockets(sock_set, TIMEOUT);
		if (!socket) break;

		if (numready > 0) {
			packet_ext packet(socket, true);
			int err = packet.receive();
			if (err < 0) {
				std::cerr << STRING("ERROR_RECEIVING_PACKET", err, SDLNet_GetError()) << std::endl;
			} else {
				int magic = packet.readInt();
				if (magic == MAGIC) {
					handlePacket(packet);
				}
			}
		} else if (numready == 0) {
			g_chat.addLine(COLOR_RED, STRING("CLIENT_TIMED_OUT"));
			clear();
			break;
		} else if (errno) {
			std::cerr << STRING("ERROR_CHECKING_SOCKET", numready, SDLNet_GetError()) << std::endl;
			clear();
			err_ret = errno;
			break;
		}
	}

	//g_chat.addLine(COLOR_YELLOW, "Receiver thread stopped.");

	return err_ret;
}

void game_client::render(SDL_Renderer *renderer) {
	world.render(renderer);
	g_chat.render(renderer);
	g_score.render(renderer);
}

void game_client::handlePacket(byte_array &packet) {
	int message = packet.readInt();
	switch (message) {
	case SERV_MESSAGE:
		messageCmd(packet);
		break;
	case SERV_KICK:
		kickCmd(packet);
		break;
	case SERV_PING:
		pingCmd(packet);
		break;
	case SERV_PING_MSECS:
		msecCmd(packet);
		break;
	case SERV_SCORE:
		scoreCmd(packet);
		break;
	case SERV_SNAPSHOT:
		snapshotCmd(packet);
		break;
	case SERV_ADD_ENT:
		addCmd(packet);
		break;
	case SERV_REM_ENT:
		remCmd(packet);
		break;
	case SERV_SELF:
		selfCmd(packet);
		break;
	case SERV_MAP:
		mapCmd(packet);
		break;
	case SERV_SOUND:
		soundCmd(packet);
		break;
	case SERV_RESET:
		resetCmd(packet);
		break;
	case SERV_REPEAT:
		repeatCmd(packet);
		break;
	}
}

void game_client::execCmd(const std::string &message) {
	static const char *SPACE = " \t";

	size_t wbegin = 0;
	size_t wend = message.find_first_of(SPACE, wbegin);

	std::string cmd = message.substr(wbegin, wend - wbegin);
	std::transform(cmd.begin(), cmd.end(), cmd.begin(), tolower);
	if (cmd == "listenserver") {
		uint16_t port = DEFAULT_PORT;
		wbegin = message.find_first_not_of(SPACE, wend);
		if (wbegin != std::string::npos) {
			wend = message.find_first_of(SPACE, wbegin);

			std::string port_str = message.substr(wbegin, wend - wbegin);
			try {
				port = std::stoi(port_str);
			} catch (std::invalid_argument) {
				g_chat.addLine(COLOR_RED, STRING("NOT_A_NUMBER", port_str.c_str()));
			}
		}

		int err = startListenServer(port);
		if (err) {
			g_chat.addLine(COLOR_RED, STRING("ERROR_CREATING_LISTENSERVER", err));
		} else {
			connect("localhost", port);
		}
	} else if (cmd == "connect") {
		wbegin = message.find_first_not_of(SPACE, wend);
		if (wbegin == std::string::npos) {
			g_chat.addLine(COLOR_ORANGE, STRING("USAGE_connect"));
			return;
		}
		wend = message.find_first_of(SPACE, wbegin);

		std::string address = message.substr(wbegin, wend - wbegin);

		wbegin = message.find_first_not_of(SPACE, wend);
		if (wbegin == std::string::npos) {
			connect(address.c_str());
			return;
		}

		std::string port = message.substr(wbegin);
		try {
			connect(address.c_str(), std::stoi(port));
		} catch (std::invalid_argument) {
			g_chat.addLine(COLOR_RED, STRING("NOT_A_NUMBER", port.c_str()));
		}
	} else if (cmd == "join") {
		sendJoinCmd();
	} else if (cmd == "spectate") {
		sendLeaveCmd();
	} else if (cmd == "disconnect") {
		disconnect();
	} else if (cmd == "name") {
		wbegin = message.find_first_not_of(SPACE, wend);
		if (wbegin == std::string::npos) {
			g_chat.addLine(COLOR_ORANGE, STRING("USAGE_name"));
			return;
		}
		wend = message.find_last_not_of(SPACE);

		std::string name = message.substr(wbegin, wend - wbegin + 1);
		setName(name);
	} else if (cmd == "volume") {
		wbegin = message.find_first_not_of(' ', wend);
		if (wbegin == std::string::npos) {
			g_chat.addLine(COLOR_ORANGE, STRING("USAGE_volume"));
			return;
		}
		wend = message.find_first_of(SPACE, wbegin);

		std::string arg1 = message.substr(wbegin, wend - wbegin);

		std::string arg2;

		wbegin = message.find_first_not_of(SPACE, wend);
		if (wbegin != std::string::npos) {
			wend = message.find_last_not_of(SPACE);
			arg2 = message.substr(wbegin, wend - wbegin + 1);
		}

		int volume_int = 0;
		try {
			volume_int = std::stoi(arg1);
			if (volume_int > 100) {
				volume_int = 100;
			}
		} catch (std::invalid_argument) {
			g_chat.addLine(COLOR_RED, STRING("NOT_A_NUMBER", arg1.c_str()));
			return;
		}

		if (arg2 == "music") {
			setMusicVolume(volume_int * MIX_MAX_VOLUME / 100);
			g_chat.addLine(COLOR_ORANGE, STRING("VOLUME_SET_MUSIC", volume_int));
		} else {
			setVolume(volume_int * MIX_MAX_VOLUME / 100);
			g_chat.addLine(COLOR_ORANGE, STRING("VOLUME_SET_MAIN", volume_int));
		}
	} else if (cmd == "vote") {
		wbegin = message.find_first_not_of(SPACE, wend);
		if (wbegin == std::string::npos) {
			g_chat.addLine(COLOR_ORANGE, STRING("USAGE_vote"));
			return;
		}
		wend = message.find_first_of(SPACE, wbegin);

		std::string vote_type = message.substr(wbegin, wend - wbegin);
		std::transform(vote_type.begin(), vote_type.end(), vote_type.begin(), tolower);

		std::string args;

		wbegin = message.find_first_not_of(SPACE, wend);
		if (wbegin != std::string::npos) {
			wend = message.find_last_not_of(SPACE);
			args = message.substr(wbegin, wend - wbegin + 1);
		}

		if (vote_type == "start") {
			sendVoteCmd(VOTE_START, 0);
		} else if (vote_type == "stop") {
			sendVoteCmd(VOTE_STOP, 0);
		} else if (vote_type == "reset") {
			sendVoteCmd(VOTE_RESET, 0);
		} else if (vote_type == "bot_add") {
			if (args.empty()) {
				sendVoteCmd(VOTE_ADD_BOT, 1);
			} else try {
				sendVoteCmd(VOTE_ADD_BOT, std::stoi(args));
			} catch (std::invalid_argument) {
				g_chat.addLine(COLOR_RED, STRING("NOT_A_NUMBER", args.c_str()));
			}
		} else if (vote_type == "bot_remove") {
			sendVoteCmd(VOTE_REMOVE_BOTS, 0);
		} else if (vote_type == "kick") {
			if (args.empty()) {
				g_chat.addLine(COLOR_ORANGE, STRING("USAGE_vote_kick"));
				return;
			}
			int user_id = g_score.findUserID(args.c_str());
			if (user_id > 0) {
				sendVoteCmd(VOTE_KICK, user_id);
			} else {
				g_chat.addLine(COLOR_ORANGE, STRING("INVALID_USER", args.c_str()));
			}
		} else if (vote_type == "zone") {
			if (args.empty()) {
				g_chat.addLine(COLOR_ORANGE, STRING("USAGE_vote_zone"));
				return;
			}
			std::transform(args.begin(), args.end(), args.begin(), tolower);
			int zone_id = game_map::getZoneByName(args);
			if (zone_id >= 0) {
				sendVoteCmd(VOTE_ZONE, zone_id);
			} else {
				g_chat.addLine(COLOR_ORANGE, STRING("INVALID_ZONE_NAME", args.c_str()));
			}
		} else if (vote_type == "yes") {
			sendVoteCmd(VOTE_YES, 0);
		} else if (vote_type == "no") {
			sendVoteCmd(VOTE_NO, 0);
		} else {
			g_chat.addLine(COLOR_ORANGE, STRING("INVALID_VOTE", vote_type.c_str()));
		}
	} else if (cmd == "kill") {
		sendKillCmd();
	} else if (cmd == "quit") {
		quit();
	} else {
		g_chat.addLine(COLOR_ORANGE, STRING("INVALID_COMMAND", cmd.c_str()));
	}
}

void game_client::handleEvent(const SDL_Event &event) {
	if (g_chat.isTyping()) {
		g_chat.handleEvent(event);
		return;
	}
	bool down = false;
	switch(event.type) {
	case SDL_KEYDOWN:
		switch (event.key.keysym.scancode) {
		case SDL_SCANCODE_TAB:
			g_score.show(true);
			break;
		default:
			if (!event.key.repeat) {
				sendInput(getKeyBinding(event.key.keysym.scancode), true);
			}
			break;
		}
		break;
	case SDL_KEYUP:
		switch (event.key.keysym.scancode) {
		case SDL_SCANCODE_TAB:
			g_score.show(false);
			break;
		case SDL_SCANCODE_T:
			g_chat.startTyping();
			break;
		default:
			if (!event.key.repeat) {
				sendInput(getKeyBinding(event.key.keysym.scancode), false);
			}
			break;
		}
		break;
	case SDL_MOUSEMOTION:
		//sendMouse(event.motion.x, event.motion.y);
		break;
	case SDL_MOUSEBUTTONDOWN:
	case SDL_MOUSEBUTTONUP:
		down = event.type == SDL_MOUSEBUTTONDOWN;
		sendInput(getMouseBinding(event.button.button), down);
		break;
	}
}

bool game_client::sendJoinCmd() {
	if (socket == nullptr) {
		g_chat.addLine(COLOR_RED, STRING("NOT_CONNECTED"));
		return false;
	}

	packet_ext packet(socket);
	packet.writeInt(CMD_JOIN);
	return packet.sendTo(server_ip);
}

bool game_client::sendLeaveCmd() {
	if (socket == nullptr) {
		g_chat.addLine(COLOR_RED, STRING("NOT_CONNECTED"));
		return false;
	}

	packet_ext packet(socket);
	packet.writeInt(CMD_LEAVE);
	return packet.sendTo(server_ip);
}

bool game_client::sendVoteCmd(uint32_t vote_type, uint32_t args) {
	if (socket == nullptr) {
		g_chat.addLine(COLOR_RED, STRING("NOT_CONNECTED"));
		return false;
	}

	packet_ext packet(socket);
	packet.writeInt(CMD_VOTE);
	packet.writeInt(vote_type);
	packet.writeInt(args);
	return packet.sendTo(server_ip);
}

bool game_client::sendKillCmd() {
	if (socket == nullptr) {
		g_chat.addLine(COLOR_RED, STRING("NOT_CONNECTED"));
		return false;
	}

	packet_ext packet(socket);
	packet.writeInt(CMD_KILL);
	return packet.sendTo(server_ip);
}

bool game_client::sendChatMessage(const char *message) {
	if (socket == nullptr) {
		g_chat.addLine(COLOR_RED, STRING("NOT_CONNECTED"));
		return false;
	}

	packet_ext packet(socket);
	packet.writeInt(CMD_CHAT);
	packet.writeString(message);
	return packet.sendTo(server_ip);
}

bool game_client::sendScorePacket() {
	if (socket == nullptr) {
		//g_chat.addLine(COLOR_RED, STRING("NOT_CONNECTED"));
		return false;
	}

	packet_ext packet(socket);
	packet.writeInt(CMD_SCORE);
	return packet.sendTo(server_ip);
}

void game_client::setName(const std::string &name) {
	user_name = name;
	g_chat.addLine(COLOR_CYAN, STRING("NAME_SET", name.c_str()));

	if (socket == nullptr) return;

	packet_ext packet(socket);
	packet.writeInt(CMD_NAME);
	packet.writeString(name.c_str());
	packet.sendTo(server_ip);
}

void game_client::messageCmd(byte_array &packet) {
	char *message = packet.readString();
	uint32_t color = packet.readInt();

	std::cout << message << std::endl;
	g_chat.addLine(color, message);
}

void game_client::kickCmd(byte_array &packet) {
	char *message = packet.readString();

	g_chat.addLine(COLOR_RED, STRING("CLIENT_KICKED", message));

	clear();
}

void game_client::pingCmd(byte_array &from) {
	if (socket == nullptr) return;

	packet_ext packet(socket);
	packet.writeInt(CMD_PONG);
	packet.sendTo(server_ip);
}

void game_client::msecCmd(byte_array &from) {
	ping_msecs = from.readShort();
}

void game_client::scoreCmd(byte_array &packet) {
	g_score.handlePacket(packet);
}

void game_client::snapshotCmd(byte_array &packet) {
	while(!packet.atEnd()) {
		uint16_t id = packet.readShort();
		uint8_t type = packet.readChar();

		byte_array ba = packet.readByteArray();

		entity *ent = world.findID(id);
		if (ent && ent->getType() == type) {
			ent->readFromByteArray(ba);
		} else {
			ent = entity::newObjFromByteArray(&world, id, static_cast<entity_type>(type), ba);
			if (ent) {
				world.addEntity(ent);
			}
		}
	}
}

void game_client::addCmd(byte_array &packet) {
	while(!packet.atEnd()) {
		uint16_t id = packet.readShort();
		if (! world.findID(id)) {
			uint8_t type = packet.readChar();
			byte_array ba = packet.readByteArray();
			entity *ent = entity::newObjFromByteArray(&world, id, static_cast<entity_type>(type), ba);
			if (ent) {
				world.addEntity(ent);
			}
		}
	}
}

void game_client::remCmd(byte_array &packet) {
	while (!packet.atEnd()) {
		uint16_t id = packet.readShort();
		world.removeEntity(world.findID(id));
	}
}

void game_client::selfCmd(byte_array &packet) {
	uint16_t self_id = packet.readShort();

	entity *ent = world.findID(self_id);
	if (ent && ent->getType() == TYPE_PLAYER) {
		dynamic_cast<player *>(ent)->setSelf(true);
	}

	g_score.setSelfID(packet.readShort());
}

void game_client::mapCmd(byte_array &packet) {
	world.handleMapPacket(packet);
}

void game_client::soundCmd(byte_array &packet) {
	uint8_t id = packet.readChar();
	playWave(static_cast<wave_id>(id));
}

void game_client::resetCmd(byte_array &packet) {
	world.clear();
}

void game_client::repeatCmd(byte_array &packet) {
	int packet_id = packet.readInt();

	if (packet_repeat_ids.find(packet_id) == packet_repeat_ids.end()) {
		byte_array ba = packet.readByteArray();
		int magic = ba.readInt();
		if (magic == MAGIC) {
			handlePacket(ba);
		}
		packet_repeat_ids[packet_id] = SDL_GetTicks();
	}
}
