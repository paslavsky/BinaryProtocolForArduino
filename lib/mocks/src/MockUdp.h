#ifndef MOCK_UDP_H
#define MOCK_UDP_H

#include <Udp.h>

class MockUDP : public UDP
{
public:
    // Constructor(s) and destructor
    MockUDP();
    ~MockUDP() override = default;

    uint8_t begin(uint16_t) override { return 1; }
    void stop() override {};


    int beginPacket(IPAddress ip, uint16_t port) override;
    int beginPacket(const char *host, uint16_t port) override;
    int endPacket() override;
    int parsePacket() override;

    IPAddress remoteIP() override;
    uint16_t remotePort() override;


    size_t write(uint8_t) override;
    size_t write(const uint8_t *buffer, size_t size) override;

    int available() override;
    int read() override;
    int read(unsigned char *buffer, size_t len) override;
    int read(char *buffer, size_t len) override;
    int peek() override;
    void flush() override;

    void mock_setRemoteIP(IPAddress ip);
    void mock_setRemotePort(uint16_t port);
    
    void mock_setPacketToParse(const uint8_t *packet, size_t size);
    IPAddress mock_getPacketIP();
    uint16_t mock_getPacketPort();

    size_t mock_getWroteData(uint8_t *buffer, size_t len);
    
    void mock_reset();
private:
    IPAddress remote_ip;
    uint16_t remote_port;  
    const uint8_t *packet;
    size_t packet_size;
    size_t packet_index;

    IPAddress packet_ip;
    uint16_t packet_port;
    uint8_t write_buffer[512];
    size_t write_size;
};

#endif // MOCK_UDP_H