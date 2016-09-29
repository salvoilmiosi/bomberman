#include "game_world.h"

#include <cstring>

game_world::game_world() {

}

game_world::~game_world() {
	for (auto it : objects) {
		if (it.second) delete it.second;
	}
}

void game_world::addObject(game_obj *obj) {
    objects[obj->getID()] = obj;
}

void game_world::removeObject(game_obj *obj) {
    if (obj == nullptr) return;

    objects.erase(obj->getID());
    delete obj;
}

void game_world::tick() {
    for (auto it : objects) {
        if (it.second) {
            it.second->tick();
        }
    }
}

game_obj *game_world::findID(Uint16 id) {
    auto it = objects.find(id);
    if (it != objects.end()) {
        return it->second;
    } else {
        return nullptr;
    }
}
