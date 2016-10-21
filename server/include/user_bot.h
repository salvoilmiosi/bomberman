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
		bool isDown(uint8_t input_cmd);

		bool isPressed(uint8_t input_cmd);

		int getMouseX();

		int getMouseY();
	};

	class game_server *server;
	char username[USER_NAME_SIZE];

	class player *ent = nullptr;

	bot_handler handler;

public:
	user_bot(class game_server *server);
	virtual ~user_bot();

public:
	void createPlayer(class game_world *world, uint8_t player_num);

	const char *getName() {
		return username;
	}

	void setName(const char *name);

	class player *getPlayer() {
		return ent;
	}

	void tick();

	void destroyPlayer();
};

#endif