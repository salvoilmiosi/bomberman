#ifndef __HEALTHBAR_H__
#define __HEALTHBAR_H__

#include <SDL2/SDL.h>

static const int HEALTHBAR_W = 50;
static const int HEALTHBAR_H = 10;
static const float MAX_HEALTH = 100.f;
static constexpr float HEALTH_LOSE_PER_TICK = 0.001f;

class healthbar {
private:
    float health_points;
    float new_health_points;

public:
    bool friendly;

public:
    healthbar(bool friendly = false);

public:
    void tick();

    void render(SDL_Renderer *renderer, int x, int y);

    void setNewHp(int new_hp) {
        float new_hp_f = new_hp / MAX_HEALTH;
        new_health_points = new_hp_f;
    }
};

#endif // __HEALTHBAR_H__
