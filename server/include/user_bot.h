#ifndef __USER_BOT_H__
#define __USER_BOT_H__

#include "user.h"

class user_bot {
private:
	class bot_handler : public input_handler {
	public:
		user_bot *bot;

	public:
		bot_handler(user_bot *bot) : bot(bot) {}

	public:
		bool isDown(usr_input input_cmd);

		bool isPressed(usr_input input_cmd);

		int getMouseX();

		int getMouseY();
	};

	class game_server *server;
	std::string username;

	class player *ent = nullptr;

	bot_handler handler;

public:
	user_bot(class game_server *server);
	virtual ~user_bot();

public:
	void createPlayer(class game_world *world);

	const char *getName() {
		return username.c_str();
	}

	void setName(const std::string &name);

	class player *getPlayer() {
		return ent;
	}

	void tick();

	void destroyPlayer();
};

#endif
