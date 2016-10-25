#ifndef __INTERPOLATOR_H__
#define __INTERPOLATOR_H__

#include <deque>

#include <SDL2/SDL.h>

#include <cstdio>

template<typename T>
class interpolator {
private:
    struct snapshot {
        T obj;
        uint32_t time;
    };

    snapshot last_shot;

    std::deque<snapshot> shots;

    int interp_delay;

public:
    interpolator(const int update_rate = 60, const int interp_ratio = 2) {
        interp_delay = 1000 * interp_ratio / update_rate;
    }

public:
    void tick() {
        while (!shots.empty() && SDL_GetTicks() > shots.front().time + 100) {
            shots.pop_front();
        }
    }

    void addSnapshot(const T &obj) {
        snapshot pos = {obj, SDL_GetTicks() + interp_delay};
        last_shot = pos;
        shots.push_back(pos);
    }

    T interpolate() {
        if (shots.empty()) {
            return last_shot.obj;
        }

        if (shots.size() <= 1) {
            return shots.back().obj;
        }

        float time = SDL_GetTicks();
        T pos_start = shots.front().obj;
        float time_start = shots.front().time;
        for (const snapshot &it : shots) {
            if (it.time > time) {
                T pos_end = it.obj;
                float time_end = it.time;

                float time_amt = (time - time_start) / (time_end - time_start);
                return pos_start + time_amt * (pos_end - pos_start);
            } else {
                pos_start = it.obj;
                time_start = it.time;
            }
        }

        return last_shot.obj;
    }

    void clear() {
        shots.clear();
    }
};

#endif // __INTERPOLATOR_H__
