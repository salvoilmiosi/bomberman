#ifndef __PACKET_IO_H__
#define __PACKET_IO_H__

#include <SDL2/SDL_net.h>

static constexpr Uint32 str2int(const char *str) {
    return
        str[0] << (8 * 3) |
        str[1] << (8 * 2) |
        str[2] << (8 * 1) |
        str[3] << (8 * 0);
}

static const int PACKET_SIZE = 1024;

static const Uint32 MAGIC = str2int("GAME");

class packet_ext {
private:
    UDPpacket *packet;
    Uint8 *data_ptr;

    UDPsocket socket;

public:
    packet_ext(UDPsocket socket, bool input = false);
    virtual ~packet_ext();

public:
    void writeInt(const Uint32 num);
    Uint32 readInt();

    void writeChar(const Uint8 num);
    Uint8 readChar();

    void writeShort(const Uint16 num);
    Uint16 readShort();

    void writeLong(const Uint64 num);
    Uint16 readLong();

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
