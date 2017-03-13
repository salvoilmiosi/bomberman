#include "packet_io.h"
#include <cstdio>
#include <cstring>

byte_array::byte_array(const byte_array &ba) {
    memcpy(p_data, ba.p_data, PACKET_SIZE);
    data_ptr = ba.data_ptr - ba.p_data + p_data;
    data_len = ba.data_len;
}

void byte_array::clear() {
    memset(p_data, 0, PACKET_SIZE);
    data_ptr = p_data;
    data_len = 0;
}

void byte_array::setData(const uint8_t *data, size_t len) {
    memcpy(p_data, data, len);
    data_len = len;
    data_ptr = p_data;
}

void byte_array::writeChar(const uint8_t num) {
    *data_ptr++ = num;
    ++data_len;
}

uint8_t byte_array::readChar() {
    return *data_ptr++;
}

void byte_array::writeShort(const uint16_t num) {
    SDLNet_Write16(num, data_ptr);
    data_ptr += 2;
    data_len += 2;
}

uint16_t byte_array::readShort() {
    uint16_t ret = SDLNet_Read16(data_ptr);
    data_ptr += 2;
    return ret;
}

void byte_array::writeInt(const uint32_t num) {
    SDLNet_Write32(num, data_ptr);
    data_ptr += 4;
    data_len += 4;
}

uint32_t byte_array::readInt() {
    uint32_t ret = SDLNet_Read32(data_ptr);
    data_ptr += 4;
    return ret;
}

void byte_array::writeLong(const uint64_t num) {
    uint32_t upper = (num & 0xffffffff00000000) >> 32;
    uint32_t lower = (num & 0x00000000ffffffff);
    writeInt(upper);
    writeInt(lower);
}

uint64_t byte_array::readLong() {
    uint32_t upper = readInt();
    uint32_t lower = readInt();
    return ((uint64_t)upper << 32) | lower;
}

void byte_array::writeString(const char *str, short max_len) {
    int str_len = max_len == 0 ? strlen(str) : max_len;
    writeShort(str_len);
    memcpy(data_ptr, str, str_len);
    data_ptr += str_len;
    data_len += str_len;
}

char *byte_array::readString() {
    static char buffer[1024];
    memset(buffer, 0, 1024);
    int str_len = readShort();
    strncpy(buffer, reinterpret_cast<const char *>(data_ptr), str_len);
    data_ptr += str_len;
    return buffer;
}

void byte_array::writeByteArray(const byte_array &ba) {
    writeShort(ba.data_len);
    memcpy(data_ptr, ba.p_data, ba.data_len);
    data_ptr += ba.data_len;
    data_len += ba.data_len;
}

byte_array byte_array::readByteArray() {
    byte_array ba;

    ba.data_len = readShort();
    memcpy(ba.p_data, data_ptr, ba.data_len);

    data_ptr += ba.data_len;

    return ba;
}

bool byte_array::atEnd() {
    return p_data + data_len - data_ptr <= 0;
}

packet_ext::packet_ext(UDPsocket socket, bool input) : socket(socket) {
    memset(&packet, 0, sizeof(packet));
    packet.data = p_data;
    packet.channel = -1;
    packet.maxlen = PACKET_SIZE;

    if (!input)
        writeInt(MAGIC);
}

packet_ext::packet_ext(const packet_ext &p) : byte_array(p) {
    packet = p.packet;
    socket = p.socket;
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

int packet_ext::receive() {
    clear();

    int err = SDLNet_UDP_Recv(socket, &packet);
    data_len = packet.len;
    return err;
}

#include "main.h"

int packet_ext::sendTo(const IPaddress &addr) {
    /*if ((random_engine() % 1000) > 1000) {
        fprintf(stderr, "Lost a packet\n");
        return 0;
    }*/
    packet.len = data_len;
    packet.address = addr;
    return SDLNet_UDP_Send(socket, packet.channel, &packet);
}

const IPaddress &packet_ext::getAddress() {
    return packet.address;
}
