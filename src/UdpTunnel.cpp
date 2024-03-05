#include "UdpTunnel.h"
#include <Arduino.h>

#include <utility>
#include "BinaryMessage.h"

using namespace bpa;
using namespace bpa::udp;

uint16_t encode(const DeviceID id, const uint8_t seed) {
    return (id ^ seed) << 8 | (id + seed) % 256;
}

uint8_t decodeSeed(const DeviceID id, const uint16_t encoded) {
    const uint8_t seed = encoded & 0xFF;
    return seed - id % 256;
}

UdpDeviceInfo::~UdpDeviceInfo() {
    DEBUG_PRINTLN("UdpDeviceInfo::~UdpDeviceInfo()");
}

UDPTunnel::~UDPTunnel() {
    DEBUG_PRINTLN("UDPTunnel::~UDPTunnel()");

    for (auto& [deviceId, deviceInfo]: connectedDevices) {
        delete deviceInfo;
    }
    connectedDevices.clear();
}

void UDPTunnel::sendMessage(const DeviceID to, uint8_t* buffer, const uint8_t size) {
    if (isConnected(to) == false) {
        triggerError(to, DEVICE_NOT_CONNECTED, "Device not connected");
        return;
    }

    DEBUG_PRINTF("UDPTunnel::sendMessage() - Sending message to %d\n\r", to);
    const auto info       = connectedDevices[to];
    const auto message_id = doSend(info->getIP(), info->getPort(), START_V1, buffer, size);
    addPendingPackets(to, message_id);
}

void UDPTunnel::loop() {
    const auto result = _readMessage();
    checkForLostPackets();
    updateConnectedDevicesState();
    clearStaleHandshakes();
    if (isMessageEmpty(result) == false) {
        DEBUG_PRINTLN("UDPTunnel::loop() - Message received");
        triggerMessageReceived(result.device_id, result.data, result.size);
    }
}

BinaryMessage UDPTunnel::_readMessage() {
    if (udp.parsePacket()) {
        if (const auto [binaryMessage, validationStatus] = io.read(); validationStatus == STATUS_OK) {
            DEBUG_PRINTLN("UDPTunnel::_readMessage() - Received packet");
            if (processReceivedMessage(binaryMessage)) {
                return binaryMessage;
            }
        }
        else {
#if defined(BPA_DEBUG_ENABLED)
            DEBUG_PRINTF("UDPTunnel::_readMessage() - Invalid message (status: %s)\n\r", validationStatusToString(validationStatus));
#endif
            processInvalidMessage(validationStatus, binaryMessage);
        }
    }
    return emptyMessage();
}

bool UDPTunnel::processReceivedMessage(const BinaryMessage& message) {
    const auto deviceId = message.device_id;
    const auto isKnown  = isKnownDevice(deviceId);

    if ((isVersionStartByte(message.start) || isControlStartByte(message.start)) && !isKnown) {
        doSend(udp.remoteIP(), udp.remotePort(), DISCONNECT);
        DEBUG_PRINTF("UDPTunnel::processReceivedMessage() - Device %d not connected\n\r", deviceId);
        return false;
    }

    switch (message.start) {
        case START_V1: {
            DEBUG_PRINTF("UDPTunnel::processReceivedMessage() - Received message from %d\n\r", deviceId);
            const auto message_id = doSend(udp.remoteIP(), udp.remotePort(), CONFIRM);
            addPendingPackets(deviceId, message_id);
            connectedDevice_receivedPacket(deviceId);
            return true;
        }
        case CONFIRM: {
            DEBUG_PRINTF("UDPTunnel::processReceivedMessage() - Received confirmation from %d\n\r", deviceId);
            pendingPackets_receivedResponse(message.message_id);
            connectedDevice_receivedPacket(deviceId);
            break;
        }
        case INCORRECT_FORMAT:
        case INCORRECT_CHECKSUM:
        case REJECTED: {
            DEBUG_PRINTF("UDPTunnel::processReceivedMessage() - Received error from %d\n\r", deviceId);
            pendingPackets_receivedResponse(message.message_id);
            connectedDevice_error(deviceId);
            triggerError(deviceId, INCORRECT_FORMAT_ERROR, "Incorrect format");
            break;
        }
        case PING: {
            DEBUG_PRINTF("UDPTunnel::processReceivedMessage() - Received ping from %d\n\r", deviceId);
            const auto message_id = doSend(udp.remoteIP(), udp.remotePort(), CONFIRM);
            addPendingPackets(deviceId, message_id);
            connectedDevice_receivedPacket(deviceId);
            break;
        }
        case HANDSHAKE_INIT: {
            if (const auto bpaVersion = message.data[0]; bpaVersion != BPA_VERSION) {
                DEBUG_PRINTF(
                    "UDPTunnel::processReceivedMessage() - Received handshake init from %d with unsuported version\n\r",
                    deviceId);
                doSend(udp.remoteIP(), udp.remotePort(), REJECTED);
                break;
            }

            DEBUG_PRINTF("UDPTunnel::processReceivedMessage() - Received handshake init from %d\n\r", deviceId);
            const auto seed          = decodeSeed(getID(), message.data[1] << 8 | message.data[2]);
            pendingConnections[seed] = {udp.remoteIP(), udp.remotePort(), GET_CURRENT_TIMESTAMP()};
            handshake(internal::HandshakeByte::HANDSHAKE_RESP, seed);
            break;
        }
        case HANDSHAKE_RESP: {
            const auto bpaVersion = message.data[0];
            if (bpaVersion != BPA_VERSION) {
                DEBUG_PRINTF(
                    "UDPTunnel::processReceivedMessage() - Received handshake init from %d with unsuported version\n\r",
                    deviceId);
                doSend(udp.remoteIP(), udp.remotePort(), StartByte::REJECTED);
                break;
            }

            DEBUG_PRINTF("UDPTunnel::processReceivedMessage() - Received handshake response from %d\n\r", deviceId);
            const auto seed = decodeSeed(getID(), (message.data[1] << 8) | message.data[2]);

            const auto infoRef = pendingConnections.find(seed);
            if (infoRef == pendingConnections.end()) {
                DEBUG_PRINTF(
                    "UDPTunnel::processReceivedMessage() - Received handshake response from %d with unknown seed\n\r",
                    deviceId);
                doSend(udp.remoteIP(), udp.remotePort(), StartByte::REJECTED);
                break;
            }

            auto [ip, port, timestamp] = infoRef->second;
            const auto device          = new internal::ConnectedDevice(ip, port);
            connectedDevices[deviceId] = device;
            pendingConnections.erase(seed);
            connectedDevice_receivedPacket(deviceId);
            triggerDeviceConnected(deviceId, *device);
            doSend(ip, port, HANDSHAKE_COMPLETE);
            break;
        }
        case HANDSHAKE_COMPLETE: {
            DEBUG_PRINTF("UDPTunnel::processReceivedMessage() - Received handshake complete from %d\n\r", deviceId);
            const auto seed    = decodeSeed(getID(), message.data[1] << 8 | message.data[2]);
            const auto infoRef = pendingConnections.find(seed);
            if (infoRef == pendingConnections.end()) {
                DEBUG_PRINTF(
                    "UDPTunnel::processReceivedMessage() - Received handshake response from %d with unknown seed\n\r",
                    deviceId);
                doSend(udp.remoteIP(), udp.remotePort(), StartByte::REJECTED);
                break;
            }

            auto [ip, port, timestamp] = infoRef->second;
            const auto device          = new internal::ConnectedDevice(ip, port);
            connectedDevices[deviceId] = device;
            pendingConnections.erase(seed);
            connectedDevice_receivedPacket(deviceId);
            triggerDeviceConnected(deviceId, *device);
            break;
        }
        case DISCONNECT: {
            DEBUG_PRINTF("UDPTunnel::processReceivedMessage() - Received disconnect from %d\n\r", deviceId);
            if (isKnown) {
                const auto device = connectedDevices[deviceId];
                device->state     = internal::ConnectedDevice::State::DISCONNECTED;
                delete device;
                connectedDevices.erase(deviceId);
            }
            break;
        }
        default: {
            DEBUG_PRINTF("UDPTunnel::processReceivedMessage() - Unsupported start byte: 0x%02X\n\r", message.start);
            break;
        }
    }
    return false;
}

void UDPTunnel::processInvalidMessage(const ValidationStatus status, const BinaryMessage& message) {
    DEBUG_PRINTF("UDPTunnel::processInvalidMessage() - Invalid message (status: %d)\n\r", status);
    if (message.message_id != 0) {
        pendingPackets_receivedResponse(message.message_id);
    }

    switch (status) {
        case STATUS_MISSED_START_BYTE:
        case STATUS_MISSED_DEVICE_ID:
        case STATUS_INCORRECT_FORMAT:
            doSend(udp.remoteIP(), udp.remotePort(), INCORRECT_FORMAT);
            break;
        case STATUS_INCORRECT_CHECKSUM:
            doSend(udp.remoteIP(), udp.remotePort(), INCORRECT_CHECKSUM);
            break;
        default:
            break;
    }
}

MessageID UDPTunnel::doSend(IPAddress ip, const uint16_t port, const StartByte start, uint8_t* data,
                            const uint8_t size) {
    const BinaryMessage message = {start, getID(), generateMessageID(), size, data};
    udp.beginPacket(std::move(ip), port);
    io.write(message);
    udp.endPacket();
    return message.message_id;
}

void UDPTunnel::checkForLostPackets() {
    const auto now = GET_CURRENT_TIMESTAMP();
    for (auto it = pendingPackets.begin(); it != pendingPackets.end();) {
        auto [timestamp, device_id] = it->second;
        if (now - timestamp > BPA_LOST_PACKET_TIMEOUT) {
            DEBUG_PRINTF("UDPTunnel::checkForLostPackets() - Packet to device %d lost\n\r", packet.device_id);
            connectedDevice_lostPacket(device_id);
            it = pendingPackets.erase(it);
        }
        else {
            ++it;
        }
    }
}

void UDPTunnel::updateConnectedDevicesState() {
    const auto now = GET_CURRENT_TIMESTAMP();
    for (auto it = connectedDevices.begin(); it != connectedDevices.end();) {
        const auto deviceId = it->first;
        const auto device   = it->second;

        if (now - device->lastPing > BPA_PING_FREQUENCY) {
            const auto message_id = doSend(device->getIP(), device->getPort(), PING);
            addPendingPackets(deviceId, message_id);
            device->lastPing = now;
        }

        if constexpr (BPA_DISCONNECT_ON_LOST_N_PACKETS && device->countOfLost > BPA_DISCONNECT_ON_LOST_N_PACKETS) {
            DEBUG_PRINTF("UDPTunnel::updateConnectedDevicesState() - Too many packets lost for device %d\n\r",
                         deviceId);
            device->state = internal::ConnectedDevice::State::LOST;
            triggerError(deviceId, DEVICE_LOST, "Device lost");
        }
        else if (device->state == internal::ConnectedDevice::State::CONNECTED && now - device->lastSeen >
                 BPA_STALE_TIMEOUT) {
            DEBUG_PRINTF("UDPTunnel::updateConnectedDevicesState() - Device %d stale\n\r", deviceId);
            device->state       = internal::ConnectedDevice::State::LOST;
            device->lastUpdated = now;
            triggerError(deviceId, DEVICE_LOST, "Device lost");
        }
        else if (device->state == internal::ConnectedDevice::State::LOST && now - device->lastSeen >
                 BPA_DISCONNECTED_TIMEOUT) {
            DEBUG_PRINTF("UDPTunnel::updateConnectedDevicesState() - Device %d disconnected by timeout\n\r", deviceId);
            device->lastUpdated = now;
            doSend(device->getIP(), device->getPort(), DISCONNECT);
            delete device;
            it = connectedDevices.erase(it);
        }
        else {
            ++it;
        }
    }
}

void UDPTunnel::clearStaleHandshakes() {
    const auto now = GET_CURRENT_TIMESTAMP();
    for (auto it = pendingConnections.begin(); it != pendingConnections.end();) {
        auto [ip, port, timestamp] = it->second;
        if (now - timestamp > BPA_STALE_TIMEOUT) {
            DEBUG_PRINTF("UDPTunnel::clearStaleHandshakes() - Clearing stale handshake (IP: %s, port: %d)\n\r",
                         ip.toString().c_str(), port);
            it = pendingConnections.erase(it);
        }
        else {
            ++it;
        }
    }
}

uint8_t UDPTunnel::generateSeedForHandshake() {
    uint8_t seed = random(0, 255);
    while (pendingConnections.find(seed) != pendingConnections.end()) {
        seed = random(0, 255);
    }
    return seed;
}

void UDPTunnel::handshake(internal::HandshakeByte byte, const uint8_t seed) {
    DEBUG_PRINTF("UDPTunnel::handshake() - Sending handshake (byte: %d, seed: %d)\n\r", byte, seed);
    auto [ip, port, timestamp] = pendingConnections[seed];

    const uint16_t enc = encode(getID(), seed);
    uint8_t data[3]    = {BPA_VERSION, lowByte(enc), highByte(enc)};
    doSend(ip, port, static_cast<StartByte>(byte), data, 3);
}

void UDPTunnel::connect(DeviceInfo& info) {
    if (info.type() == UdpDeviceInfo::TYPE) {
        const auto udpInfo = static_cast<UdpDeviceInfo *>(&info); // NOLINT(*-pro-type-static-cast-downcast)
        connect(udpInfo->getIP(), udpInfo->getPort());
    }
    else {
        DEBUG_PRINTLN("UDPTunnel::connect() - Unsupported device info");
    }
}

void UDPTunnel::connect(IPAddress ip, const uint16_t port) {
    DEBUG_PRINTF("UDPTunnel::connect() - Connecting to %s:%d\n\r", ip.toString().c_str(), port);

    const uint8_t seed       = generateSeedForHandshake();
    pendingConnections[seed] = {std::move(ip), port, GET_CURRENT_TIMESTAMP()};

    handshake(internal::HandshakeByte::HANDSHAKE_INIT, seed);
}

void UDPTunnel::disconnect(const DeviceID deviceId) {
    if (!isKnownDevice(deviceId)) {
        DEBUG_PRINTF("UDPTunnel::disconnect() - Device %d not connected\n\r", deviceId);
        return; // Device is already lost or disconnected
    }

    const auto device = connectedDevices[deviceId];
    DEBUG_PRINTF("UDPTunnel::disconnect() - Disconnecting device %d\n\r", deviceId);
    device->state = internal::ConnectedDevice::State::DISCONNECTED;
    doSend(device->getIP(), device->getPort(), DISCONNECT);
    delete device;
    connectedDevices.erase(deviceId);
}

bool UDPTunnel::isConnected(const DeviceID deviceId) {
    const auto device = connectedDevices.find(deviceId);
    if (device != connectedDevices.end()) {
        return device->second->state == internal::ConnectedDevice::State::CONNECTED;
    }
    return false;
}

MessageID UDPTunnel::generateMessageID() {
    messageCounter = (messageCounter == 255) ? 1 : messageCounter + 1;
    return messageCounter;
}

void UDPTunnel::connectedDevice_lostPacket(const DeviceID id) {
    if (!isKnownDevice(id)) {
        return; // Device is not known
    }

    const auto device = connectedDevices[id];
    DEBUG_PRINTF(
        "UDPTunnel::connectedDevice_lostPacket() - Device %d did not confirm packet (triggered by timeout)\n\r", id);
    device->countOfLost++;
    device->lastUpdated = GET_CURRENT_TIMESTAMP();
}

void UDPTunnel::connectedDevice_error(const DeviceID id) {
    if (!isKnownDevice(id)) {
        return; // Device is not known
    }

    const auto device = connectedDevices[id];
    DEBUG_PRINTF("UDPTunnel::connectedDevice_error() - Error occurred while communicating with device %d\n\r", id);
    device->lastUpdated = GET_CURRENT_TIMESTAMP();
    device->lastSeen    = GET_CURRENT_TIMESTAMP();
    device->countOfErrors++;
}

void UDPTunnel::connectedDevice_receivedPacket(const DeviceID id) {
    if (!isKnownDevice(getID())) {
        return; // Device is not known
    }

    const auto device = connectedDevices[id];
    DEBUG_PRINTF("UDPTunnel::connectedDevice_receivedPacket() - Received packet from device %d\n\r", device_id);
    device->countOfLost   = 0;
    device->countOfErrors = 0;
    device->lastUpdated   = GET_CURRENT_TIMESTAMP();
    device->lastSeen      = GET_CURRENT_TIMESTAMP();
    device->lastPing      = GET_CURRENT_TIMESTAMP();

    if (device->state == internal::ConnectedDevice::State::LOST) {
        DEBUG_PRINTF("UDPTunnel::connectedDevice_receivedPacket() - Set device %d state to CONNECTED\n\r", device_id);
        device->state = internal::ConnectedDevice::State::CONNECTED;
    }
}

void UDPTunnel::addPendingPackets(const DeviceID deviceId, const MessageID message_id) {
    pendingPackets[message_id] = {GET_CURRENT_TIMESTAMP(), deviceId};
}

void UDPTunnel::pendingPackets_receivedResponse(const MessageID message_id) {
    const auto packet = pendingPackets.find(message_id);
    if (packet != pendingPackets.end()) {
        pendingPackets.erase(packet);
    }
}

bool UDPTunnel::isKnownDevice(const DeviceID id) {
    return connectedDevices.find(id) != connectedDevices.end();
}

bool UDPTunnel::isLostDevice(const DeviceID id) {
    const auto device = connectedDevices.find(id);
    if (device != connectedDevices.end()) {
        return device->second->state == internal::ConnectedDevice::State::LOST;
    }
    return false;
}
