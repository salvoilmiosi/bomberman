#ifndef __GAME_WORLD_H__
#define __GAME_WORLD_H__

#include "game_obj.h"
#include <set>

class game_world {
private:
    std::set <game_obj *> objects;

public:
    game_world();
    virtual ~game_world();

public:
    void addObject(game_obj *obj);
    void tick(class game_server *server);

    int objectCount() {
        return objects.size();
    }

    const std::set<game_obj *>&getObjects() {
        return objects;
    }
};

#endif // __GAME_WORLD_H__
