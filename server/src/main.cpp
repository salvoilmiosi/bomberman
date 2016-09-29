#include <SDL2/SDL_net.h>
#include <cstdio>
#include <cstdlib>
#include <time.h>

#include "server.h"

int main(int argc, char **argv) {
    SDL_Init(SDL_INIT_VIDEO);

    srand(time(NULL));

    if (SDLNet_Init() < 0) {
        fprintf(stderr, "Could not init SDL_net\n");
        return 1;
    } else {
        game_server server;
        if (server.isOpen()) {
            printf("Server listening on port %d\n", PORT);
        } else {
            fprintf(stderr, "Could not open server socket: %s\n", SDLNet_GetError());
            return 1;
        }

        int err = server.run();

        printf("Server quit, returned %d\n", err);
    }

    SDLNet_Quit();
    SDL_Quit();
    return 0;
}
