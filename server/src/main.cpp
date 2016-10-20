#include <SDL2/SDL_net.h>
#include <cstdio>
#include <cstdlib>
#include <ctime>

#include "game_world.h"
#include "main.h"

std::default_random_engine random_engine;

static const uint16_t DEFAULT_PORT = 27015;

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

    uint8_t num_players = 2;
    uint16_t port = DEFAULT_PORT;

    if (argc > 1) num_players = atoi(argv[1]);
    if (argc > 2) port = atoi(argv[2]);

    game_world world(num_players);

    int err = world.startServer(port);

    printf("Server quit, returned %d\n", err);

    SDLNet_Quit();
    SDL_Quit();
    return 0;
}
