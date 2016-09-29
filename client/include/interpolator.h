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
        Uint32 time;
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
        while (!shots.empty() && SDL_GetTicks() > shots.front().time + interp_delay) {
            shots.pop_front();
        }
    }

    void addSnapshot(const T &obj) {
        snapshot pos = {obj, SDL_GetTicks() + interp_delay};
        if (shots.empty()) {
            last_shot = pos;
        }
        shots.push_back(pos);
    }

    void interpolate(float *result, float T::* member) {
        if (shots.empty()) {
            return;
        }

        if (shots.size() <= 1) {
            *result = shots.back().obj.*member;
            return;
        }

        float time = SDL_GetTicks();
        float pos_start = last_shot.obj.*member;
        float time_start;
        for (const snapshot &it : shots) {
            if (it.time > time) {
                float pos_end = it.obj.*member;
                float time_end = it.time;

                float time_amt = (time - time_start) / (time_end - time_start);
                *result = pos_start + time_amt * (pos_end - pos_start);
                return;
            } else {
                pos_start = it.obj.*member;
                time_start = it.time;
            }
        }

        *result = pos_start;
    }

    void clear() {
        shots.clear();
    }
};

#endif // __INTERPOLATOR_H__
