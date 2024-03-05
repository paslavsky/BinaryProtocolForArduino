#ifndef BPA_COMMON_H
#define BPA_COMMON_H

#include <cstdint>
#include <Arduino.h>

namespace bpa {
    /**
     * @brief The version of the Binary Protocol for Arduino.
     */
#define BPA_VERSION 1

    /**
     * @brief The maximum size of a binary message payload.
     */
#define BPA_MAX_PAYLOAD_SIZE 256

    /**
     * @brief The maximum size of a binary message.
     */
#define BPA_MAX_SIZE (BPA_MAX_PAYLOAD_SIZE + 6)

#ifndef BPA_LOST_PACKET_TIMEOUT
    /**
     * @brief If this timeout is exceeded, the packet is considered "LOST"
     */
#define BPA_LOST_PACKET_TIMEOUT 1000
#endif

#ifndef BPA_PING_FREQUENCY
    /**
     * @brief The frequency of the ping message in milliseconds.
     */
#define BPA_PING_FREQUENCY 1000
#endif

#ifndef BPA_STALE_TIMEOUT
    /**
     * @brief @brief If this timeout is exceeded, the device is considered "LOST"
     */
#define BPA_STALE_TIMEOUT 10000
#endif

#ifndef BPA_DISCONNECTED_TIMEOUT
    /**
     * @brief If this timeout is exceeded, the device is considered "DISCONNECTED"
     */
#define BPA_DISCONNECTED_TIMEOUT 10000
#endif

#ifndef BPA_DISCONNECT_ON_LOST_N_PACKETS
    /**
     * @brief If this number of packets is lost, the device is considered "DISCONNECTED". If set to 0, this feature is disabled.
     */
#define BPA_DISCONNECT_ON_LOST_N_PACKETS 0
#endif

    /**
     * @typedef DeviceID
     * @brief Type representing the device ID of a binary message.
     */
    typedef uint8_t DeviceID;

    /**
     * @typedef MessageID
     * @brief Type representing the message ID of a binary message.
     */
    typedef uint8_t MessageID;

    // Add example below before including this file to enable debug output
    //  #define BPA_DEBUG_ENABLED

#ifdef BPA_DEBUG_ENABLED

#define DEBUG_BEGIN(speed) Serial.begin(speed)
#define DEBUG_PRINT(...) Serial.print(__VA_ARGS__)
#define DEBUG_PRINTLN(...) Serial.println(__VA_ARGS__)
#define DEBUG_PRINTF(format, ...) Serial.printf(format, __VA_ARGS__)
#else
#define DEBUG_BEGIN(speed)
#define DEBUG_PRINT(...)
#define DEBUG_PRINTLN(...)
#define DEBUG_PRINTF(format, ...)
#endif

#ifdef ARDUINO
    /**
     * @typedef TimeStamp
     * @brief Type representing a timestamp. This is only defined if the Arduino library is available and provides the millis() function.
     */
    typedef unsigned long TimeStamp;

#define GET_CURRENT_TIMESTAMP millis
#endif

} // namespace bpa

#endif // BPA_COMMON_H
