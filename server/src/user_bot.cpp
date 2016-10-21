#include "user_bot.h"

#include "game_server.h"
#include "game_world.h"
#include "player.h"
#include "input_handler.h"

#include <cstring>

bool user_bot::bot_handler::isDown(uint8_t input_cmd) {
	switch (input_cmd) {
	case USR_LEFT:
		return true;
	case USR_RIGHT:
		return SDL_GetTicks() % 1000 > 500;
	default:
		return false;
	}
}

bool user_bot::bot_handler::isPressed(uint8_t input_cmd) {
	return false;
}

int user_bot::bot_handler::getMouseX() {
	return 0;
}

int user_bot::bot_handler::getMouseY() {
	return 0;
}

user_bot::user_bot(game_server *server) : server(server), handler(this) {
	// TODO generate bot names
	setName("BOT Dummy");
}

user_bot::~user_bot() {
	//destroyPlayer();
}

void user_bot::createPlayer(game_world *world, uint8_t player_num) {
	ent = new player(world, &handler, player_num);
	ent->setName(username);
}

void user_bot::setName(const char *name) {
	strncpy(username, name, USER_NAME_SIZE);
}

void user_bot::tick() {

}

void user_bot::destroyPlayer() {
	if (ent)
		ent->destroy();
}