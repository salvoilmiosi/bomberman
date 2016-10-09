#include <SDL2/SDL.h>
#include <SDL2/SDL_net.h>
#include <SDL2/SDL_image.h>

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <time.h>
#include <map>

#include "main.h"
#include "game_client.h"
#include "resources.h"
#include "bindings.h"

static SDL_Window *window;
static SDL_Renderer *renderer;

static game_client client;

void render() {
    static SDL_Rect win_rect = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
    SDL_SetRenderDrawColor(renderer, 0, 0, 50, 255);
    SDL_RenderFillRect(renderer, &win_rect);
    client.render(renderer);
    SDL_RenderPresent(renderer);
}

int main(int argc, char **argv) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "Could not init SDL\n");
        return 1;
    }
    if (SDLNet_Init() < 0) {
        fprintf(stderr, "Could not init SDL_net\n");
        return 1;
    }
    if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG) {
        fprintf(stderr, "Could not init SDL_image\n");
        return 1;
    }

    srand(time(NULL));

    window = SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        fprintf(stderr, "Could not create window\n");
        return 1;
    }

    SDL_SetWindowIcon(window, icon_surface);

    renderer = SDL_CreateRenderer(window, 0, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        fprintf(stderr, "Could not create renderer\n");
        return 1;
    }

    loadResources(renderer);

    setupBindings();

    const char *address = "localhost";
    const char *username = "Salvo";
    int port = PORT;
    if (argc >= 4) {
        address = argv[1];
        username = argv[2];
        port = atoi(argv[3]);
    }

    if (!client.connect(address, username, port)) {
        fprintf(stderr, "Could not connect to server\n");
        return 1;
    }

    SDL_Event event;
    bool quit = false;

	int lastTime = SDL_GetTicks();
	float unprocessed = 0;
	float msPerTick = 1000.f / TICKRATE;
	int frames = 0;
	int ticks = 0;
	int secondsTimer = SDL_GetTicks();

    static char title[128];

    SDL_StopTextInput();

	while (!quit) {
		int now = SDL_GetTicks();
		unprocessed += (now - lastTime) / msPerTick;
		lastTime = now;
		bool shouldRender = true;
		while (unprocessed >= 1) {
			if (!client.tick()) {
                quit = true;
			}

			++ticks;
			--unprocessed;
			shouldRender = true;
		}

		SDL_Delay(2);

		if (shouldRender) {
			render();
			++frames;
		}

		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_QUIT:
				quit = true;
				break;
            default:
                client.handleEvent(event);
                break;
			}
		}

		if (SDL_GetTicks() - secondsTimer > 1000) {
			secondsTimer += 1000;

            sprintf(title, "%s --- Frames: %4d --- Ticks: %3d --- Ping: %3d",
                    WINDOW_TITLE, frames, ticks, client.getPingMsecs());
            SDL_SetWindowTitle(window, title);

			frames = 0;
			ticks = 0;
		}
	}

    client.disconnect();

    SDL_DestroyWindow(window);

    clearResources();

    IMG_Quit();
    SDLNet_Quit();
    SDL_Quit();
    return 0;
}
