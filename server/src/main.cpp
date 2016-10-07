#include <SDL2/SDL_net.h>
#include <cstdio>
#include <cstdlib>
#include <ctime>

#include "game_world.h"
#include "main.h"

std::default_random_engine random_engine;

int main(int argc, char **argv) {
    srand(time(0));
    random_engine.seed(rand());

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "Could not inid SDL\n");
    }

    if (SDLNet_Init() < 0) {
        fprintf(stderr, "Could not init SDL_net\n");
        return 1;
    }

    game_world world;

    int err = world.startServer();

    printf("Server quit, returned %d\n", err);

    SDLNet_Quit();
    SDL_Quit();
    return 0;
}
