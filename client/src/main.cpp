#include <SDL2/SDL.h>
#include <SDL2/SDL_net.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>

#include <iostream>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <time.h>
#include <map>

#include "main.h"
#include "game_client.h"
#include "game_sound.h"
#include "strings.h"

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
	std::string resource_path;
	if (argc > 0) {
		resource_path = argv[0];
		resource_path = resource_path.substr(0, 1 + resource_path.find_last_of("\\/"));
	}
	resource_path += "resource.dat";

	if (!openResourceFile(resource_path.c_str())) {
		std::cerr << STRING("ERROR_COULD_NOT_OPEN_RESOURCES") << std::endl;
		return 1;
	}

	load_locale(loadStringFromResource("LOCALE_ENGLISH"));

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
		std::cerr << STRING("ERROR_COULD_NOT_INIT_SDL") << std::endl;
		return 1;
	}
	if (SDLNet_Init() < 0) {
		std::cerr << STRING("ERROR_COULD_NOT_INIT_SDLnet") << std::endl;
		return 1;
	}
	if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG) {
		std::cerr << STRING("ERROR_COULD_NOT_INIT_SDLimage") << std::endl;
		return 1;
	}
	if (Mix_Init(MIX_INIT_OGG) != MIX_INIT_OGG) {
		std::cerr << STRING("ERROR_COULD_NOT_INIT_SDLmixer") << std::endl;
		return 1;
	}
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) != 0) {
		std::cerr << STRING("ERROR_COULD_NOT_OPEN_CHANNEL") << std::endl;
		return 1;
	}
	if (TTF_Init() < 0) {
		std::cerr << STRING("ERROR_COULD_NOT_INIT_SDLttf") << std::endl;
		return 1;
	}

	srand(time(NULL));

	window = SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
	if (!window) {
		std::cerr << STRING("ERROR_COULD_NOT_CREATE_WINDOW") << std::endl;
		return 1;
	}

	SDL_SetWindowIcon(window, icon_surface);

	renderer = SDL_CreateRenderer(window, 0, SDL_RENDERER_ACCELERATED);
	if (!renderer) {
		std::cerr << STRING("ERROR_COULD_NOT_CREATE_RENDERER") << std::endl;
		return 1;
	}

	loadResources(renderer);
	loadSounds();
	setMusicVolume(0);
	setVolume(10);

	setupBindings();

	const char *username = nullptr;
	const char *address = nullptr;
	uint16_t port = DEFAULT_PORT;

	if (argc > 1) username = argv[1];
	if (argc > 2) address = argv[2];
	if (argc > 3) port = atoi(argv[3]);

	if (username && address) {
		client.setName(username);
		client.connect(address, port);
	}

	SDL_Event event;

	int lastTime = SDL_GetTicks();
	float unprocessed = 0;
	float msPerTick = 1000.f / TICKRATE;
	int frames = 0;
	int ticks = 0;
	int secondsTimer = SDL_GetTicks();

	static char title[128];

	SDL_StopTextInput();

	while (client.isOpen()) {
		int now = SDL_GetTicks();
		unprocessed += (now - lastTime) / msPerTick;
		lastTime = now;
		bool shouldRender = true;
		while (unprocessed >= 1) {
			client.tick();

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
				client.quit();
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

	SDL_DestroyWindow(window);

	clearSounds();
	clearResources();

	Mix_CloseAudio();

	TTF_Quit();
	Mix_Quit();
	IMG_Quit();
	SDLNet_Quit();
	SDL_Quit();
	return 0;
}
