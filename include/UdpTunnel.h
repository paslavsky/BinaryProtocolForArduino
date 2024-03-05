#ifndef BPA_UDP_H
#define BPA_UDP_H

#include <WiFiUdp.h>
#include "common.h"
#include "BinaryMessage.h"
#include "BinaryTunnel.h"
#include <map>
#include <utility>

/**
 * @namespace bpa::udp
 * @brief Namespace containing types for UDP communication.
 */
namespace bpa::udp {
#define UDP_DEVICE_INFO_TYPE 0x01 ///< The type of the UdpDeviceInfo

    /**
     * @brief Class representing information about a device.
     */
    class UdpDeviceInfo : public DeviceInfo {
    public:
        static constexpr uint8_t TYPE = UDP_DEVICE_INFO_TYPE; ///< The type of the UdpDeviceInfo

        /**
         * @brief Constructs a UdpDeviceInfo object with the specified IP address and port number.
         *
         * @param ip The IP address of the device.
         * @param port The port number of the device.
         */
        UdpDeviceInfo(IPAddress ip, const uint16_t port) : DeviceInfo(), ip(std::move(ip)), port(port) {
        }

        /**
         * @brief Destroys the UdpDeviceInfo object.
         */
        ~UdpDeviceInfo() override;

        [[nodiscard]] IPAddress getIP() const { return ip; }    ///< Gets the IP address of the device
        [[nodiscard]] uint16_t getPort() const { return port; } ///< Gets the port number of the device

        [[nodiscard]] uint8_t type() override {
            return UDP_DEVICE_INFO_TYPE;
        }

    private:
        IPAddress ip;  ///< The IP address of the device
        uint16_t port; ///< The port number of the device
    };

    /**
     * @namespace internal
     * @brief Namespace containing internal types.
     */
    namespace internal {
#define UDP_CONNECTED_DEVICE_TYPE 0x02 ///< The type of the ConnectedDevice

        class ConnectedDevice final : public UdpDeviceInfo {
        public:
            static constexpr uint8_t TYPE = UDP_CONNECTED_DEVICE_TYPE; ///< The type of the ConnectedDevice

            ConnectedDevice(IPAddress ip, const uint16_t port) : UdpDeviceInfo(std::move(ip), port), lastSeen(0),
                                                                 lastUpdated(0),
                                                                 state(DISCONNECTED), countOfErrors(0), countOfLost(0) {
            }

            ~ConnectedDevice() override = default;

            TimeStamp lastSeen;    ///< The timestamp of the last seen message from the device
            TimeStamp lastUpdated; ///< The timestamp of the last update by the tunnel
            TimeStamp lastPing{};  ///< The timestamp of the last ping
            enum State {
                CONNECTED,
                LOST,
                DISCONNECTED
            } state; ///< The state of the device

            uint8_t countOfErrors; ///< The number of errors received from the device
            uint8_t countOfLost;   ///< The number of lost packets received from the device

            [[nodiscard]] uint8_t type() override {
                return UDP_CONNECTED_DEVICE_TYPE;
            }
        };

        struct PacketInfo {
            TimeStamp timestamp; ///< The timestamp of the packet
            DeviceID device_id;  ///< The ID of the device
        };

        struct HandshakeInfo {
            IPAddress ip;        ///< The IP address of the device
            uint16_t port;       ///< The port number of the device
            TimeStamp timestamp; ///< The timestamp of the handshake
        };

        enum HandshakeByte {
            HANDSHAKE_INIT     = 0x3C, ///< Handshake init start byte
            HANDSHAKE_RESP     = 0x3E, ///< Handshake response start byte
            HANDSHAKE_COMPLETE = 0x2E  ///< Handshake complete start byte
        };
    };

    /**
     * @brief The UDPTunnel class provides a high-level interface for sending and receiving binary messages over UDP.
     */
    class UDPTunnel final : public Tunnel {
    public:
        /**
         * @brief Constructs a UDPTunnel object with the specified UDP instance.
         *
         * @param udp The UDP instance to use for communication.
         * @param id The device ID to use for communication.
         */
        UDPTunnel(UDP& udp, const DeviceID id) : Tunnel(id), udp(udp), io(udp), messageCounter(0) {
        };

        /**
         * @brief Destroys the UDPTunnel object.
         */
        ~UDPTunnel() override;

        /**
         * @copydoc Tunnel::sendMessage()
         */
        void sendMessage(DeviceID to, uint8_t* buffer, uint8_t size) override;

        /**
         * @copydoc Tunnel::loop()
         */
        void loop() override;

        /**
         * @copydoc Tunnel::connect()
         */
        void connect(DeviceInfo& info) override;

        /**
         * @brief Connects to a device by IP address and port number.
         *
         * @param ip The IP address of the device.
         * @param port The port number of the device.
         */
        void connect(IPAddress ip, uint16_t port);

        /**
         * @copydoc Tunnel::disconnect()
         */
        void disconnect(DeviceID deviceId) override;

        /**
         * @copydoc Tunnel::isConnected()
         */
        bool isConnected(DeviceID deviceId) override;

        /**
         * @brief Checks if the specified device is known. Known devices can be in any state (connected or lost).
         *
         * @param id The ID of the device to check.
         * @return True if the device is known, false otherwise.
         */
        bool isKnownDevice(DeviceID id);

        /**
         * @brief Checks if the specified device is known but no response was received for a long time.
         *
         * @param id The ID of the device to check.
         * @return True if the device is lost, false otherwise.
         */
        bool isLostDevice(DeviceID id);

    private:
        UDP& udp; ///< The UDP instance used for communication
        BinaryMessageIO io; ///< The BinaryMessageIO instance used for reading and writing messages
        uint8_t messageCounter; ///< The counter used to generate unique message IDs
        std::map<DeviceID, internal::ConnectedDevice *> connectedDevices; ///< A map containing the connected devices
        std::map<uint8_t, internal::HandshakeInfo> pendingConnections; ///< A map containing the pending connections
        std::map<MessageID, internal::PacketInfo> pendingPackets; ///< A map containing the pending packets

        MessageID generateMessageID();      ///< Generates a unique message ID
        uint8_t generateSeedForHandshake(); ///< Generates a seed for the handshake

        /**
         * @brief Sends a handshake to the specified device with the given byte and seed.
         *
         * @param byte The handshake byte to send.
         * @param seed The seed to use for the handshake.
         */
        void handshake(internal::HandshakeByte byte, uint8_t seed);

        /**
         * @brief Process the received binary message.
         *
         * This method processes the received binary message and performs the necessary operations based on the message type and content.
         *
         * @param message The binary message received.
         *
         * @return Returns true if the message was processed successfully, false otherwise.
         */
        bool processReceivedMessage(const BinaryMessage& message);

        /**
         * @brief Processes an invalid message.
         *
         * This method is called when an invalid message is received. It logs the status of the invalid message and performs the necessary operations based on the status and message content.
         *
         * @param status The validation status of the message.
         * @param message The binary message that was received.
         */
        void processInvalidMessage(ValidationStatus status, const BinaryMessage& message);

        /**
         * @brief Sends a binary message to the specified IP address and port number.
         *
         * This method sends a binary message to the specified IP address and port number using the UDP protocol.
         *
         * @param ip The IP address of the device.
         * @param port The port number of the device.
         * @param start The start byte of the message.
         * @param data The data to be sent.
         * @param size The size of the data.
         *
         * @return The message ID of the sent message.
         */
        MessageID doSend(IPAddress ip, uint16_t port, StartByte start, uint8_t* data = nullptr, uint8_t size = 0);

        /**
         * @brief Check for lost packets and perform necessary actions.
         *
         * This method checks for lost packets in the `pendingPackets` container.
         * If a packet has been pending for longer than `BPA_LOST_PACKET_TIMEOUT` milliseconds,
         * the packet is considered lost and the corresponding device is notified.
         *
         * The method iterates over each pending packet and checks its timestamp.
         * If the difference between the current time and the packet timestamp exceeds
         * `BPA_LOST_PACKET_TIMEOUT`, the packet is considered lost.
         *
         * If a packet is lost, a log message is printed using `DEBUG_PRINTF` to indicate
         * the packet loss, and the `connectedDevice_lostPacket` method is called to
         * notify the connected device about the lost packet.
         *
         * Finally, the method removes the lost packet from the `pendingPackets` container.
         *
         * @note This method uses the `millis()` function to get the current time.
         *
         * @see connectedDevice_lostPacket
         * @see BPA_LOST_PACKET_TIMEOUT
         */
        void checkForLostPackets();

        /**
         * @brief Updates the state of connected devices.
         *
         * This method is responsible for updating the state of connected devices in the UDPTunnel.
         * It performs the following actions for each connected device:
         * 1. Checks if the device has been inactive for too long and triggers an error if necessary.
         * 2. Sends a ping message to the device if it has not been pinged recently.
         * 3. Disconnects the device if it has lost too many packets or has been inactive for too long.
         *
         * @note This method does not return any value.
         */
        void updateConnectedDevicesState();

        /**
         * @brief Clears stale handshake connections.
         *
         * This method clears any pending handshake connections that have exceeded the stale timeout duration.
         * It iterates through the map of pendingConnections and checks the timestamp of each connection.
         * If the current timestamp minus the timestamp of the connection is greater than the stale timeout duration,
         * the connection is considered stale and removed from the map.
         *
         * @note This method does not return any value.
         *
         * @see BPA_STALE_TIMEOUT
         */
        void clearStaleHandshakes();

        /**
         * @brief Handles the event when a packet is lost for a connected device.
         *
         * @param id The ID of the device for which the packet is lost.
         */
        void connectedDevice_lostPacket(DeviceID id);

        /**
         * @brief Handles an error event for a connected device.
         *
         * This method is responsible for handling error events for a connected device in the UdpTunnel class.
         * If the given device ID is not known, the method returns without taking any action.
         * Otherwise, it updates the device's lastUpdated and lastSeen timestamps with the current time,
         * increments the countOfErrors variable, and prints a debug message indicating the error occurred.
         *
         * @param id The ID of the device for which the error occurred.
         */
        void connectedDevice_error(DeviceID id);

        /**
         * @brief This method is called when a packet is received from a connected device.
         *
         * It updates the device's statistics and state, and logs debugging information.
         * If the device is not known, the method returns without performing any action.
         *
         * @param id The ID of the device that sent the packet.
         *
         * @see UDPTunnel::isKnownDevice()
         * @see internal::ConnectedDevice::State
         */
        void connectedDevice_receivedPacket(DeviceID id);

        /**
         * @brief Adds a packet to the pendingPackets map.
         *
         * This method adds a packet to the pendingPackets map with the given deviceId and message_id. The added packet has a timestamp set to the current millis().
         *
         * @param deviceId The ID of the device.
         * @param message_id The ID of the message packet.
         */
        void addPendingPackets(DeviceID deviceId, MessageID message_id);

        /**
         * @brief Handle the received response for a pending packet.
         *
         * This method is responsible for handling the received response for a pending packet.
         * It removes the packet from the list of pending packets if it exists.
         *
         * @param message_id The ID of the received response message.
         */
        void pendingPackets_receivedResponse(MessageID message_id);

        /**
         * @brief Reads a UDP message from the network.
         *
         * This method reads a UDP message from the network, processes the message,
         * and returns the binary message. If the received message is valid, it is
         * processed using the processReceivedMessage() method. If the message is
         * invalid, it is processed using the processInvalidMessage() method.
         *
         * @return BinaryMessage - The binary message received from the network.
         */
        BinaryMessage _readMessage();
    };
}


#endif // BPA_UDP_H
