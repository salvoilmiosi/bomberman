#include "user_bot.h"

#include "game_server.h"
#include "game_world.h"
#include "player.h"
#include "input_handler.h"
#include "strings.h"

#include <vector>

bool user_bot::bot_handler::isDown(usr_input input_cmd) {
	switch (input_cmd) {
	case USR_LEFT:
		return true;
	case USR_RIGHT:
		return SDL_GetTicks() % 1000 > 500;
	default:
		return false;
	}
}

bool user_bot::bot_handler::isPressed(usr_input input_cmd) {
	return false;
}

int user_bot::bot_handler::getMouseX() {
	return 0;
}

int user_bot::bot_handler::getMouseY() {
	return 0;
}

user_bot::user_bot(game_server *server) : server(server), handler(this) {
	username = std::string("BOT ") + STRING("BOT_NAME");
}

user_bot::~user_bot() {
	//destroyPlayer();
}

std::shared_ptr<player> user_bot::createPlayer(game_world &world) {
	ent = std::make_shared<player>(world, &handler);
	ent->setName(username);
	return ent;
}

void user_bot::setName(const std::string &name) {
	username = name;
	if (ent) {
		ent->setName(name);
	}
}

void user_bot::tick() {

}

void user_bot::destroyPlayer() {
	if (ent) {
		ent->destroy();
		ent = nullptr;
	}
}
