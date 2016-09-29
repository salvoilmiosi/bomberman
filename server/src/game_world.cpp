#include "game_world.h"

#include <cstring>

#include "server.h"

game_world::game_world() {

}

game_world::~game_world() {
	for (auto obj : objects) {
		if (obj) delete obj;
	}
}

void game_world::addObject(game_obj *obj) {
    objects.insert(obj);
}

void game_world::tick(game_server *server) {
    auto it = objects.begin();
    while (it != objects.end()) {
        game_obj *obj = *it;
        if (obj && obj->isNotDestroyed()) {
            obj->tick(server);
            ++it;
        } else {
            it = objects.erase(it);
        }
    }
}
