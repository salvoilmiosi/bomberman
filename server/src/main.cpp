#include <SDL2/SDL_net.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>

#include "game_world.h"
#include "resource_load.h"

#include "strings.h"
#include "random.h"

#ifndef CREATE_MAP_FILES

static const char *LOCALE = "LOCALE_ENGLISH";

static const uint16_t DEFAULT_PORT = 27015;

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

	load_locale(loadStringFromResource(LOCALE));

	init_rng();

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		std::cerr << STRING("ERROR_COULD_NOT_INIT_SDL") << std::endl;
	}

	if (SDLNet_Init() < 0) {
		std::cerr << STRING("ERROR_COULD_NOT_INIT_SDLnet") << std::endl;
		return 1;
	}

	uint16_t port = DEFAULT_PORT;
	uint8_t num_bots = 0;

	if (argc > 1) port = atoi(argv[1]);
	if (argc > 2) num_bots = atoi(argv[2]);

	game_world world;
	world.addBots(num_bots);

	int err = world.startServer(port);

	std::cout << STRING("SERVER_END", err) << std::endl;

	SDLNet_Quit();
	SDL_Quit();
	return 0;
}

#else

#include <fstream>

#include "game_map.h"

int main(int argc, char **argv) {
	game_map g_map;
	for (int i=1; i<=8; ++i) {
		map_zone zone = static_cast<map_zone>(i);
		g_map.createMap(MAP_WIDTH, MAP_HEIGHT, zone);

		byte_array packet;
		g_map.writeToPacket(packet);

		std::string filename = "zones/";
		filename += g_map.getZoneName(zone);
		filename += ".zml";

		std::ofstream ofs(filename, std::ios::binary | std::ios::out);
		ofs.write(reinterpret_cast<const char *>(packet.getData()), packet.size());
		ofs.close();
	}
}

#endif