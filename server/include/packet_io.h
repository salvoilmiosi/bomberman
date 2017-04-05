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

class byte_array {
protected:
    uint8_t p_data[PACKET_SIZE];
    uint8_t *data_ptr;
    size_t data_len;

public:
    byte_array() {
        clear();
    }

    byte_array(const uint8_t *data, size_t len) {
        setData(data, len);
    }

    byte_array(const byte_array &ba);

    virtual ~byte_array() {}

public:
    void clear();

    void setData(const uint8_t *data, size_t len);
    uint8_t *getData() {
        return p_data;
    }

    size_t size() {
        return data_len;
    }

public:
    void writeInt(const uint32_t num);
    uint32_t readInt();

    void writeChar(const uint8_t num);
    uint8_t readChar();

    void writeShort(const uint16_t num);
    uint16_t readShort();

    void writeLong(const uint64_t num);
    uint64_t readLong();

    void writeString(const std::string &str);
    std::string readString();

    void writeByteArray(const byte_array &ba);
    byte_array readByteArray();

    bool atEnd();
};

class packet_ext: public byte_array {
private:
    UDPpacket packet;

    UDPsocket socket;

public:
    packet_ext(UDPsocket socket, bool input = false);

    packet_ext(const packet_ext &p);

    virtual ~packet_ext() {}

public:
    int receive();
    int sendTo(const IPaddress &addr);

    const IPaddress &getAddress();
};

const char *ipString(const IPaddress &ip);

Uint64 addrLong(const IPaddress &ip);

IPaddress longAddr(Uint64 num);

#endif // __PACKET_IO_H__
