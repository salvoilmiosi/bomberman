#ifndef __PACKET_IO_H__
#define __PACKET_IO_H__

#include <SDL2/SDL_net.h>

static constexpr uint32_t str2int(const char *str) {
    return
        str[0] << (8 * 3) |
        str[1] << (8 * 2) |
        str[2] << (8 * 1) |
        str[3] << (8 * 0);
}

static const int PACKET_SIZE = 1024;

static const uint32_t MAGIC = str2int("GAME");

class packet_ext {
private:
    UDPpacket *packet;
    uint8_t *data_ptr;

    UDPsocket socket;

public:
    packet_ext(UDPsocket socket, bool input = false);
    virtual ~packet_ext();

public:
    void writeInt(const uint32_t num);
    uint32_t readInt();

    void writeChar(const uint8_t num);
    uint8_t readChar();

    void writeShort(const uint16_t num);
    uint16_t readShort();

    void writeLong(const Uint64 num);
    uint16_t readLong();

    void writeString(const char *str, short max_len = 0);
    char *readString();

    void skip(int bytes);

public:
    int receive();
    bool sendTo(const IPaddress &addr);

    const IPaddress &getAddress();

private:
    int remaining();
};

const char *ipString(const IPaddress &ip);

Uint64 addrLong(const IPaddress &ip);

IPaddress longAddr(Uint64 num);

#endif // __PACKET_IO_H__
