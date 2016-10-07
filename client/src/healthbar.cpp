#include "healthbar.h"

healthbar::healthbar(bool friendly) : friendly(friendly) {
    health_points = 1.f;
    new_health_points = 1.f;
}

void healthbar::tick() {
    if (health_points > new_health_points) {
        health_points -= HEALTH_LOSE_PER_TICK;
    } else {
        health_points = new_health_points;
    }
}

void healthbar::render(SDL_Renderer *renderer, int x, int y) {
    SDL_Rect rect = {x - HEALTHBAR_W / 2, y, HEALTHBAR_W, HEALTHBAR_H};

    SDL_SetRenderDrawColor(renderer, 0x0, 0x0, 0x0, 0xff);
    SDL_RenderFillRect(renderer, &rect);

    if (new_health_points < health_points) {
        SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
        rect.w = HEALTHBAR_W * health_points;
        SDL_RenderFillRect(renderer, &rect);
    }

    if (friendly) {
        SDL_SetRenderDrawColor(renderer, 0x0, 0xff, 0x0, 0xff);
    } else {
        SDL_SetRenderDrawColor(renderer, 0xff, 0x0, 0x0, 0xff);
    }

    rect.w = HEALTHBAR_W * new_health_points;
    SDL_RenderFillRect(renderer, &rect);
}
