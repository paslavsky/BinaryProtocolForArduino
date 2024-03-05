#include "BinaryMessage.h"
#include <set>

using namespace bpa;

uint16_t fnv1a_hash16(const uint8_t* bytes, const size_t length) {
    auto hash = static_cast<uint16_t>(0x97);
    for (size_t i = 0; i < length; i++) {
        hash ^= bytes[i];
        hash *= 0xA1;
    }
    return hash;
}

uint16_t calculate_hash(const BinaryMessage& message) {
    uint8_t bytes[4 + message.size];

    bytes[0] = message.start;
    bytes[1] = message.device_id;
    bytes[2] = message.message_id;
    bytes[3] = message.size;

    if (message.data != nullptr) {
        for (size_t i = 0; i < message.size; i++) {
            bytes[4 + i] = message.data[i];
        }
    }

    return fnv1a_hash16(bytes, 4 + message.size);
}

std::pair<BinaryMessage, ValidationStatus> BinaryMessageIO::read() {
    BinaryMessage message = emptyMessage();
    if (this->stream == nullptr) {
        DEBUG_PRINTLN("Stream not initialized");
        return {message, STATUS_STREAM_ERROR};
    }

    const auto count = stream->readBytes(buffer, BPA_MAX_SIZE);
    if (count <= 4) {
        DEBUG_PRINTLN("BinaryMessageIO::read() - No data to read");
        return {message, STATUS_UNEXPECTED_END_OF_STREAM};
    }

    const uint8_t messageSize = buffer[3];
    if (count != static_cast<unsigned int>(messageSize + 6)) {
        DEBUG_PRINTF("BinaryMessageIO::read() - Incorrect message size: %d, expected: %d\n", count, messageSize + 6);
        return {message, STATUS_UNEXPECTED_END_OF_STREAM};
    }

    message.start      = identify_start_byte(buffer[0]);
    message.device_id  = buffer[1];
    message.message_id = buffer[2];
    message.size       = messageSize;

    if (message.size == 0) {
        message.data = nullptr;
    }
    else {
        message.data = buffer + 4;
    }

    const auto checksum = buffer[count - 2] << 8 | buffer[count - 1];

    ValidationStatus status = validate(message);
    const uint16_t calculatedChecksum = calculate_hash(message);
    status = status == STATUS_OK && checksum != calculatedChecksum ? STATUS_INCORRECT_CHECKSUM : status;

#if defined(BPA_DEBUG_ENABLED)
    DEBUG_PRINTF("BinaryMessageIO::read() - Read message: start=0x%02X, device_id=%d, message_id=%d, size=%d, data=",
                 message.start, message.device_id, message.message_id, message.size);
    for (size_t i = 0; i < message.size; i++) {
        DEBUG_PRINTF("%02X", message.data[i]);
    }
    DEBUG_PRINTLN();
    DEBUG_PRINT("Validation status: ");
    DEBUG_PRINTLN(validationStatusToString(status));
    DEBUG_PRINTF("Checksum: 0x%04X, Calculated: 0x%04X\n", checksum, calculatedChecksum);
#endif

    return {message, status};
}

void BinaryMessageIO::write(const BinaryMessage& message) const {
    if (this->stream == nullptr) {
        DEBUG_PRINTLN("Stream not initialized");
        return;
    }

    stream->write(message.start);
    stream->write(message.device_id);
    stream->write(message.message_id);
    stream->write(message.size);
    stream->write(message.data, message.size);
    const auto checksum = fnv1a_hash16(message.data, message.size);
    stream->write(checksum >> 8);
    stream->write(checksum & 0xFF);

#if defined(BPA_DEBUG_ENABLED)
    DEBUG_PRINTF("BinaryMessageIO::write() -  Wrote message: start=0x%02X, device_id=%d, message_id=%d, size=%d, data=",
                 message.start, message.device_id, message.message_id, message.size);
    for (size_t i = 0; i < message.size; i++) {
        DEBUG_PRINTF("%02X", message.data[i]);
    }
    DEBUG_PRINTLN();
#endif
}

StartByte BinaryMessageIO::identify_start_byte(uint8_t val) {
    if (isSupportedStartByte(val)) {
        return static_cast<StartByte>(val);
    }
    DEBUG_PRINTF("BinaryMessageIO::read_start_byte() - Unsupported start byte: 0x%02X\n", byte);
    return UNDEFINED;
}

ValidationStatus BinaryMessageIO::validate(const BinaryMessage& message) {
    if (!isSupportedStartByte(message.start)) {
        DEBUG_PRINTLN("BinaryMessageIO::validate() - Missing start byte");
        return STATUS_MISSED_START_BYTE;
    }
    if (message.device_id == 0) {
        DEBUG_PRINTLN("BinaryMessageIO::validate() - Missing device ID");
        return STATUS_MISSED_DEVICE_ID;
    }
    if (message.message_id == 0) {
        DEBUG_PRINTLN("BinaryMessageIO::validate() - Missing message ID");
        return STATUS_MISSED_MESSAGE_ID;
    }
    if (message.start == START_V1 && message.size == 0) {
        DEBUG_PRINTLN("BinaryMessageIO::validate() - payload is required for START_V1 message");
        return STATUS_INCORRECT_FORMAT;
    }
    if (message.size > 0 && message.data == nullptr) {
        DEBUG_PRINTLN("BinaryMessageIO::validate() - Missing message data");
        return STATUS_INCORRECT_FORMAT;
    }
    if (message.size == 0 && message.data != nullptr) {
        DEBUG_PRINTLN("BinaryMessageIO::validate() - Incorrect message format - payload is defined but size is 0");
        return STATUS_INCORRECT_FORMAT;
    }
    if (message.start == HANDSHAKE_INIT && message.size != 3) {
        DEBUG_PRINTLN(
            "BinaryMessageIO::validate() - Incorrect message format - payload size for HANDSHAKE_INIT message should be 3")
        ;
        return STATUS_INCORRECT_FORMAT;
    }
    if (message.start == HANDSHAKE_RESP && message.size != 3) {
        DEBUG_PRINTLN(
            "BinaryMessageIO::validate() - Incorrect message format - payload size for HANDSHAKE_RESP message should be 3")
        ;
        return STATUS_INCORRECT_FORMAT;
    }
    if (message.start == HANDSHAKE_COMPLETE && message.size != 3) {
        DEBUG_PRINTLN(
            "BinaryMessageIO::validate() - Incorrect message format - payload size for HANDSHAKE_COMPLETE message should be 3")
        ;
        return STATUS_INCORRECT_FORMAT;
    }
    if (message.start == PING && message.size != 0) {
        DEBUG_PRINTLN(
            "BinaryMessageIO::validate() - Incorrect message format - payload size for PING message should be 0");
        return STATUS_INCORRECT_FORMAT;
    }
    if (message.start == CONFIRM && message.size != 0) {
        DEBUG_PRINTLN(
            "BinaryMessageIO::validate() - Incorrect message format - payload size for CONFIRM message should be 0");
        return STATUS_INCORRECT_FORMAT;
    }
    if (message.start == INCORRECT_FORMAT && message.size != 0) {
        DEBUG_PRINTLN(
            "BinaryMessageIO::validate() - Incorrect message format - payload size for INCORRECT_FORMAT message should be 0")
        ;
        return STATUS_INCORRECT_FORMAT;
    }
    if (message.start == INCORRECT_CHECKSUM && message.size != 0) {
        DEBUG_PRINTLN(
            "BinaryMessageIO::validate() - Incorrect message format - payload size for INCORRECT_CHECKSUM message should be 0")
        ;
        return STATUS_INCORRECT_FORMAT;
    }
    if (message.start == REJECTED && message.size != 0) {
        DEBUG_PRINTLN(
            "BinaryMessageIO::validate() - Incorrect message format - payload size for REJECTED message should be 0");
        return STATUS_INCORRECT_FORMAT;
    }
    if (message.start == DISCONNECT && message.size != 0) {
        DEBUG_PRINTLN(
            "BinaryMessageIO::validate() - Incorrect message format - payload size for DISCONNECT message should be 0");
        return STATUS_INCORRECT_FORMAT;
    }

    return STATUS_OK;
}

const char* bpa::startByteToString(const StartByte start) {
    switch (start) {
        case UNDEFINED:
            return "UNDEFINED";
        case START_V1:
            return "START_V1";
        case CONFIRM:
            return "CONFIRM";
        case INCORRECT_FORMAT:
            return "INCORRECT_FORMAT";
        case INCORRECT_CHECKSUM:
            return "INCORRECT_CHECKSUM";
        case PING:
            return "PING";
        case REJECTED:
            return "REJECTED";
        case HANDSHAKE_INIT:
            return "HANDSHAKE_INIT";
        case HANDSHAKE_RESP:
            return "HANDSHAKE_RESP";
        case HANDSHAKE_COMPLETE:
            return "HANDSHAKE_COMPLETE";
        case DISCONNECT:
            return "DISCONNECT";
        default:
            return "UNKNOWN";
    }
}

bool bpa::isVersionStartByte(const uint8_t start) {
    return start >= START_V1 && start <= 0x39;
}

bool bpa::isControlStartByte(const uint8_t start) {
    return start >= 0x41 && start <= 0x5A;
}

bool bpa::isHandshakeStartByte(const uint8_t start) {
    return start == HANDSHAKE_INIT || start == HANDSHAKE_RESP || start == HANDSHAKE_COMPLETE;
}

const std::set<uint8_t> SUPPORTED_START_BYTES = {
    START_V1,
    CONFIRM,
    INCORRECT_FORMAT,
    INCORRECT_CHECKSUM,
    PING,
    REJECTED,
    HANDSHAKE_INIT,
    HANDSHAKE_RESP,
    HANDSHAKE_COMPLETE,
    DISCONNECT
};

bool bpa::isSupportedStartByte(const uint8_t start) {
    return SUPPORTED_START_BYTES.find(start) != SUPPORTED_START_BYTES.end();
}

BinaryMessage bpa::emptyMessage() {
    return {UNDEFINED, 0, 0, 0, nullptr};
}

bool bpa::isMessageEmpty(const BinaryMessage& message) {
    return message.start == UNDEFINED && message.device_id == 0 && message.message_id == 0 && message.size == 0 &&
           message.data == nullptr;
}

const char* bpa::validationStatusToString(const ValidationStatus status) {
    switch (status) {
        case STATUS_OK:
            return "STATUS_OK";
        case STATUS_MISSED_START_BYTE:
            return "STATUS_MISSED_START_BYTE";
        case STATUS_MISSED_DEVICE_ID:
            return "STATUS_MISSED_DEVICE_ID";
        case STATUS_MISSED_MESSAGE_ID:
            return "STATUS_MISSED_MESSAGE_ID";
        case STATUS_INCORRECT_CHECKSUM:
            return "STATUS_INCORRECT_CHECKSUM";
        case STATUS_INCORRECT_FORMAT:
            return "STATUS_INCORRECT_FORMAT";
        case STATUS_STREAM_ERROR:
            return "STATUS_STREAM_ERROR";
        case STATUS_UNEXPECTED_END_OF_STREAM:
            return "STATUS_UNEXPECTED_END_OF_STREAM";
        default:
            return "UNKNOWN";
    }
}
