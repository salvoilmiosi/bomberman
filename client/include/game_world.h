#ifndef __GAME_WORLD_H__
#define __GAME_WORLD_H__

#include "game_obj.h"
#include <map>

class game_world {
private:
    std::map <Uint16, game_obj *> objects;

public:
    game_world();
    virtual ~game_world();

public:
    void addObject(game_obj *obj);
    void removeObject(game_obj *obj);

    void tick();

    int objectCount() {
        return objects.size();
    }

    game_obj *findID(Uint16 id);

    const std::map<Uint16, game_obj *>&getObjects() {
        return objects;
    }
};


#endif // __GAME_WORLD_H__
