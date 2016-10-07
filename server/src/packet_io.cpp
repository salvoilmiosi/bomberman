#include "packet_io.h"
#include <cstdio>
#include <cstring>

packet_ext::packet_ext(UDPsocket socket, bool input) : socket(socket) {
    packet = SDLNet_AllocPacket(PACKET_SIZE);
    packet->channel = -1;

    memset(packet->data, 0, PACKET_SIZE);
    packet->len = 0;

    data_ptr = packet->data;

    if (!input)
        writeInt(MAGIC);
}

packet_ext::~packet_ext() {
    SDLNet_FreePacket(packet);
}

constexpr uint16_t ntohs(uint16_t num) {
    return ((num & 0xff00) >> 8) | ((num & 0x00ff) << 8);
}

const char *ipString(const IPaddress &ip) {
    static char addr_name[128];
    memset(addr_name, 0, 128);
    uint32_t full_ip = ip.host;
    uint8_t addr1 = (full_ip & 0x000000ff) >> (0 * 8);
    uint8_t addr2 = (full_ip & 0x0000ff00) >> (1 * 8);
    uint8_t addr3 = (full_ip & 0x00ff0000) >> (2 * 8);
    uint8_t addr4 = (full_ip & 0xff000000) >> (3 * 8);
    sprintf(addr_name, "%d.%d.%d.%d:%d", addr1, addr2, addr3, addr4, ntohs(ip.port));
    return addr_name;
}

Uint64 addrLong(const IPaddress &ip) {
    return ((Uint64)ip.host<<32) | ((Uint64)ip.port);
}

IPaddress longAddr(Uint64 num) {
    IPaddress addr;
    addr.host = (num & 0xffffffff00000000) >> 32;
    addr.port = (num & 0x00000000ffffffff);
    return addr;
}

void packet_ext::writeInt(const uint32_t num) {
    SDLNet_Write32(num, data_ptr);
    data_ptr += 4;
    packet->len += 4;
}

uint32_t packet_ext::readInt() {
    if (remaining() < 4) return 0;
    uint32_t ret = SDLNet_Read32(data_ptr);
    data_ptr += 4;
    return ret;
}

void packet_ext::writeChar(const uint8_t num) {
    *data_ptr++ = num;
    ++packet->len;
}

uint8_t packet_ext::readChar() {
    if (remaining() < 0) return 0;
    return *data_ptr++;
}

void packet_ext::writeShort(const uint16_t num) {
    SDLNet_Write16(num, data_ptr);
    data_ptr += 2;
    packet->len += 2;
}

uint16_t packet_ext::readShort() {
    if (remaining() < 2) return 0;
    uint16_t ret = SDLNet_Read16(data_ptr);
    data_ptr += 2;
    return ret;
}

void packet_ext::writeLong(const Uint64 num) {
    uint32_t upper = (num & 0xffffffff00000000) >> 32;
    uint32_t lower = (num & 0x00000000ffffffff);
    writeInt(upper);
    writeInt(lower);
}

uint16_t packet_ext::readLong() {
    if (remaining() < 8) return 0;
    uint32_t upper = readInt();
    uint32_t lower = readInt();
    return ((Uint64)upper << 32) | lower;
}

void packet_ext::writeString(const char *str, short max_len) {
    int str_len = max_len == 0 ? strlen(str) : max_len;
    writeShort(str_len);
    memcpy(data_ptr, str, str_len);
    data_ptr += str_len;
    packet->len += str_len;
}

char *packet_ext::readString() {
    static char buffer[1024];
    memset(buffer, 0, 1024);
    int str_len = readShort();
    if (str_len > remaining()) {
        str_len = remaining() - 1;
    }
    strncpy(buffer, (const char *)data_ptr, str_len);
    data_ptr += str_len;
    return buffer;
}

void packet_ext::skip(int bytes) {
    data_ptr += bytes;
}

int packet_ext::receive() {
    return SDLNet_UDP_Recv(socket, packet);
}

bool packet_ext::sendTo(const IPaddress &addr) const {
    packet->address = addr;
    return SDLNet_UDP_Send(socket, packet->channel, packet) != 0;
}

const IPaddress &packet_ext::getAddress() {
    return packet->address;
}

int packet_ext::remaining() {
    return packet->data + packet->len - data_ptr;
}
