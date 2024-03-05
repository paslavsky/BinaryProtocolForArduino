#ifndef BINARY_MESSAGE_H
#define BINARY_MESSAGE_H

#include <Stream.h>
#include <utility>
#include "common.h"

/**
 * @namespace bpa
 * @brief Namespace containing classes and enums related to binary message handling.
 */
namespace bpa {
    /**
     * @enum StartByte
     * @brief Enum representing the possible start bytes of a binary message.
     */
    enum StartByte {
        UNDEFINED          = 0x00, ///< Undefined start byte
        START_V1           = 0x30, ///< Start byte for version 1
        CONFIRM            = 0x41, ///< Confirm start byte
        INCORRECT_FORMAT   = 0x46, ///< Incorrect format start byte
        INCORRECT_CHECKSUM = 0x48, ///< Incorrect checksum start byte
        PING               = 0x50, ///< Ping start byte
        REJECTED           = 0x52, ///< Rejected start byte
        HANDSHAKE_INIT     = 0x2A, ///< Handshake init start byte
        HANDSHAKE_RESP     = 0x2B, ///< Handshake response start byte
        HANDSHAKE_COMPLETE = 0x2E, ///< Handshake complete start byte
        DISCONNECT         = 0x7E, ///< Disconnect start byte
    };

    const char* startByteToString(StartByte start); ///< Helper function to convert a StartByte to a string

    /**
     * @brief Checks if the specified byte is a data byte for a specific version.
     */
    bool isVersionStartByte(uint8_t start);

    /**
     * @brief Checks if the specified byte is a control byte.
     */
    bool isControlStartByte(uint8_t start);

    /**
     * @brief Checks if the specified byte is a handshake byte.
     */
    bool isHandshakeStartByte(uint8_t start);

    /**
     * @brief Checks if the specified byte is a supported start byte.
     */
    bool isSupportedStartByte(uint8_t start);

    /**
     * @enum ValidationStatus
     * @brief Enum representing the possible validation statuses of a binary message.
     */
    enum ValidationStatus {
        STATUS_OK,                       ///< Validation successful
        STATUS_MISSED_START_BYTE,        ///< Missing start byte
        STATUS_MISSED_DEVICE_ID,         ///< Missing device ID
        STATUS_MISSED_MESSAGE_ID,        ///< Missing message ID
        STATUS_INCORRECT_CHECKSUM,       ///< Incorrect checksum
        STATUS_INCORRECT_FORMAT,         ///< Incorrect message format
        STATUS_STREAM_ERROR,             ///< Stream error
        STATUS_UNEXPECTED_END_OF_STREAM, ///< Unexpected end of stream
    };

    /**
     * @brief Converts a ValidationStatus enum value to its corresponding string representation.
     *
     * @param status The ValidationStatus enum value to convert.
     * @return The string representation of the ValidationStatus value.
     */
    const char* validationStatusToString(ValidationStatus status);

    /**
     * @struct BinaryMessage
     * @brief Struct representing a binary message.
     */
    struct BinaryMessage {
        StartByte start;      ///< Start byte of the message
        DeviceID device_id;   ///< Device ID
        MessageID message_id; ///< Message ID
        uint8_t size;         ///< Size of the message data
        uint8_t* data;        ///< Pointer to the message data
    };

    BinaryMessage emptyMessage();                      ///< Helper function to create an empty BinaryMessage
    bool isMessageEmpty(const BinaryMessage& message); ///< Helper function to check if a BinaryMessage is empty

    /**
     * @class BinaryMessageIO
     * @brief Class for reading, writing, and validating binary messages.
     */
    class BinaryMessageIO {
    public:
        /**
         * @brief Default constructor.
         * @param stream The stream to be used for reading and writing.
         */
        explicit BinaryMessageIO(Stream& stream) : stream(&stream) {
        }

        /**
         * @brief Destructor.
         */
        ~BinaryMessageIO() = default;

        /**
         * @brief Reads a binary message from the stream.
         * @return A pair containing the read BinaryMessage and its validation status.
         */
        std::pair<BinaryMessage, ValidationStatus> read();

        /**
         * @brief Writes a binary message to the stream.
         * @param message The BinaryMessage to be written.
         */
        void write(const BinaryMessage& message) const;

        /**
         * @brief Validates a binary message.
         * @param message The BinaryMessage to be validated.
         * @return The validation status of the message.
         */
        static ValidationStatus validate(const BinaryMessage& message);

    private:
        Stream* stream;                                    ///< Pointer to the stream used for reading and writing
        static StartByte identify_start_byte(uint8_t val); ///< Helper function to read the start byte from the stream
        uint8_t buffer[BPA_MAX_SIZE]{};                    ///< Buffer for reading the message data
    };
} // namespace bpa

#endif // BINARY_MESSAGE_H
