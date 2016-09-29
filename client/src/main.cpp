#include <SDL2/SDL.h>
#include <SDL2/SDL_net.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <time.h>
#include <map>

#include "game_client.h"

static SDL_Window *window;
static SDL_Renderer *renderer;

static game_client client;

static const char *const WINDOW_TITLE = "Online game base";
static const int WINDOW_WIDTH = 800;
static const int WINDOW_HEIGHT = 600;

static const int FPS = 300;
static const int TICKRATE = 300;

static const int PORT = 27015;

static const int NUM_KEYS = 512;
static const int NUM_MOUSE_BUTTONS = 16;

static Uint8 key_bindings[NUM_KEYS];
static Uint8 mouse_bindings[NUM_MOUSE_BUTTONS];

bool tick() {
    if (!client.receive()) {
        return false;
    }

    client.tick();

    return true;
}

void render() {
    static SDL_Rect win_rect = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, &win_rect);
    for (auto it : client.getObjects()) {
        it.second->render(renderer);
    }
    SDL_RenderPresent(renderer);
}

bool sendInput(SDL_Scancode key, bool down) {
    Uint8 cmd = key_bindings[key];
    if (!cmd) return false;

    return client.sendInput(cmd, down);
}

bool sendMouseInput(Uint8 button, bool down) {
    Uint8 cmd = mouse_bindings[button];
    if (!cmd) return false;

    return client.sendInput(cmd, down);
}

void handleEvent(const SDL_Event &event) {
    bool down = false;
    switch(event.type) {
    case SDL_KEYDOWN:
    case SDL_KEYUP:
        down = event.type == SDL_KEYDOWN;
        if (!event.key.repeat) {
            sendInput(event.key.keysym.scancode, down);
        }
        break;
    case SDL_MOUSEMOTION:
        client.sendMouse(event.motion.x, event.motion.y);
        break;
    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP:
        down = event.type == SDL_MOUSEBUTTONDOWN;
        sendMouseInput(event.button.button, down);
        break;
    }
}

void setupBindings() {
    memset(key_bindings, 0, NUM_KEYS);
    key_bindings[SDL_SCANCODE_UP] = 1;
    key_bindings[SDL_SCANCODE_W] = 1;
    key_bindings[SDL_SCANCODE_DOWN] = 2;
    key_bindings[SDL_SCANCODE_S] = 2;
    key_bindings[SDL_SCANCODE_LEFT] = 3;
    key_bindings[SDL_SCANCODE_A] = 3;
    key_bindings[SDL_SCANCODE_RIGHT] = 4;
    key_bindings[SDL_SCANCODE_D] = 4;
    key_bindings[SDL_SCANCODE_SPACE] = 5;
    key_bindings[SDL_SCANCODE_E] = 6;

    memset(mouse_bindings, 0, NUM_MOUSE_BUTTONS);
    mouse_bindings[SDL_BUTTON_LEFT] = 7;
    mouse_bindings[SDL_BUTTON_RIGHT] = 6;
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

    srand(time(NULL));

    window = SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        fprintf(stderr, "Could not create window\n");
        return 1;
    }

    renderer = SDL_CreateRenderer(window, 0, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        fprintf(stderr, "Could not create renderer\n");
        return 1;
    }

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

	while (!quit) {
		int now = SDL_GetTicks();
		unprocessed += (now - lastTime) / msPerTick;
		lastTime = now;
		bool shouldRender = true;
		while (unprocessed >= 1) {
			if (!tick()) {
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
			case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_ESCAPE) {
					quit = true;
                    break;
				}
            default:
                handleEvent(event);
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
    SDLNet_Quit();
    SDL_Quit();
    return 0;
}
