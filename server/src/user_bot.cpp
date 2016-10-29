#include "user_bot.h"

#include "game_server.h"
#include "game_world.h"
#include "player.h"
#include "input_handler.h"
#include "main.h"

#include <cstring>
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
	static const std::vector<const char *> NAMES = {"Bud", "Milan", "Rolland", "Don", "Milford", "Gayle", "Alberto", "Alec", "Bennett", "Eloy", "Marcel", "Mathew", "Wilbert", "Charlie", "Mikel", "Olen", "Lincoln", "Hai", "Sam", "Angelo", "Elmer", "Lance", "Leandro", "Alexander", "Carlo", "Heath", "Boyd", "Cameron", "Sang", "Julius", "Robt", "Damien", "Eduardo", "Wilford", "Graig", "Jonas", "Rusty", "Norbert", "Billy", "Abe", "David", "Orval", "Joesph", "Anibal", "Josue", "Leonel", "Jefferson", "Robby", "Sydney", "Darren"};

	username = std::string("BOT ") + NAMES[random_engine() % NAMES.size()];
}

user_bot::~user_bot() {
	//destroyPlayer();
}

void user_bot::createPlayer(game_world *world) {
	ent = new player(world, &handler);
	ent->setName(username);
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
