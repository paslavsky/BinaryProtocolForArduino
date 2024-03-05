#include "MockUdp.h"

#include <unity.h>

uint8_t randomByte()
{
    return random() % 256;
}

IPAddress randomIP()
{
    return {randomByte(), randomByte(), randomByte(), randomByte()};
}

MockUDP::MockUDP() : remote_ip(randomIP()), remote_port(randomByte()), packet(nullptr), packet_size(0), packet_index(0), packet_ip(0, 0, 0, 0), packet_port(0), write_buffer{0}, write_size(0)
{
}

int MockUDP::beginPacket(IPAddress ip, uint16_t port)
{
    packet_ip = ip;
    packet_port = port;
    return 1;
}

int MockUDP::beginPacket(const char *host, uint16_t port)
{
    auto res = packet_ip.fromString(host);
    packet_port = port;
    return res;
}

int MockUDP::endPacket()
{
    return 1;
}

int MockUDP::parsePacket()
{
    return packet_size;
}

IPAddress MockUDP::remoteIP()
{
    return remote_ip;
}

uint16_t MockUDP::remotePort()
{
    return remote_port;
}

size_t MockUDP::write(uint8_t data)
{
    TEST_MESSAGE("MockUDP::write(uint8_t)");
    if (write_size >= sizeof(write_buffer))
    {
        return 0;
    }

    write_buffer[write_size++] = data;
    TEST_MESSAGE(("MockUDP::write(uint8_t) - write_size: " + String(write_size)).c_str());
    return 1;
}

size_t MockUDP::write(const uint8_t *buffer, size_t size)
{
    TEST_MESSAGE("MockUDP::write(const uint8_t *, size_t)");
    // Write as much as possible
    size_t to_write = sizeof(write_buffer) - write_size;
    if (to_write > size)
    {
        to_write = size;
    }

    memcpy(write_buffer + write_size, buffer, to_write);
    write_size += to_write;
    TEST_MESSAGE(("MockUDP::write(const uint8_t *, size_t) - write_size: " + String(write_size)).c_str());
    return to_write;
}

int MockUDP::available()
{
    return packet_size - packet_index;
}

int MockUDP::read()
{
    if (packet_index >= packet_size)
    {
        return -1;
    }

    return packet[packet_index++];
}

int MockUDP::read(unsigned char *buffer, size_t len)
{
    // Read as much as possible
    size_t to_read = packet_size - packet_index;
    if (to_read > len)
    {
        to_read = len;
    }

    memcpy(buffer, packet + packet_index, to_read);
    packet_index += to_read;
    return to_read;
}

int MockUDP::read(char *buffer, size_t len)
{
    // Read as much as possible
    size_t to_read = packet_size - packet_index;
    if (to_read > len)
    {
        to_read = len;
    }

    memcpy(buffer, packet + packet_index, to_read);
    packet_index += to_read;
    return to_read;
}

int MockUDP::peek()
{
    if (packet_index >= packet_size)
    {
        return -1;
    }

    return packet[packet_index];
}

void MockUDP::flush()
{
}

void MockUDP::mock_setRemoteIP(IPAddress ip)
{
    remote_ip = ip;
}

void MockUDP::mock_setRemotePort(uint16_t port)
{
    remote_port = port;
}

void MockUDP::mock_setPacketToParse(const uint8_t *packet, size_t size)
{
    this->packet = packet;
    packet_size = size;
    packet_index = 0;
}

IPAddress MockUDP::mock_getPacketIP()
{
    return packet_ip;
}

uint16_t MockUDP::mock_getPacketPort()
{
    return packet_port;
}

size_t MockUDP::mock_getWroteData(uint8_t *buffer, size_t len)
{
    TEST_MESSAGE(("MockUDP::mock_getWroteData(..., " + String(len) + "): write_size = " + String(write_size)).c_str());
    if (len > write_size)
    {
        len = write_size;
    }

    memcpy(buffer, write_buffer, len);
    return len;
}

void MockUDP::mock_reset()
{
    remote_ip = randomIP();
    remote_port = randomByte();
    packet = nullptr;
    packet_size = 0;
    packet_index = 0;
    packet_ip = IPAddress(0, 0, 0, 0);
    packet_port = 0;
    write_size = 0;
}
