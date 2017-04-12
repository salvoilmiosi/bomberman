#include <fstream>

#include "game_map.h"

int main(int argc, char **argv) {
	game_map g_map;
	for (int i=0; i<8; ++i) {
		map_zone zone = static_cast<map_zone>(i);
		g_map.createMap(MAP_WIDTH, MAP_HEIGHT, zone);

		byte_array packet;
		g_map.writeToPacket(packet);

		std::string filename = g_map.getZoneName(zone);
		filename += ".zml";

		std::ofstream ofs(filename, std::ios::binary | std::ios::out);
		ofs.write(reinterpret_cast<const char *>(packet.getData()), packet.size());
		ofs.close();
	}
}
