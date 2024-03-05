#ifndef BPA_TUNNEL_H
#define BPA_TUNNEL_H

#include "common.h"
#include "Errors.h"

/**
 * @brief This is a library skeleton with the namespace "bpa" and a virtual class.
 *
 * This library provides a basic structure for creating libraries with the "bpa" namespace.
 * It includes a virtual class that can be used as a base for implementing specific functionality.
 */
namespace bpa {
    class DeviceInfo {
    public:
        virtual ~DeviceInfo() = 0;

        /**
         * @brief Get the type of the object.
         *
         * RTTI is not available in Arduino, so this method is used to determine the type of the object.
         *
         * @return The type of the object as a uint8_t value.
         */
        virtual uint8_t type() = 0;
    };

    /**
     * @brief Represents a tunnel for communication between devices.
     *
     * The Tunnel class provides an interface for sending and receiving messages between devices.
     * It allows users to connect to devices, send messages, and register callback functions for various events.
     */
    class Tunnel {
    public:
        /**
         * @brief Constructs a Tunnel object with the specified device ID.
         *
         * @param id The device ID to use for communication.
         */
        explicit Tunnel(const DeviceID id) : id(id), onDeviceConnectedCallback(nullptr), onErrorCallback(nullptr),
                                             onDeviceDisconnectedCallback(nullptr),
                                             onMessageReceivedCallback(nullptr) {
        };

        /**
         * @brief Destroys the Tunnel object.
         */
        virtual ~Tunnel() = default;

        /**
         * @brief Sends a message to the specified recipient.
         *
         * @param to The ID of the recipient.
         * @param buffer A pointer to the message buffer.
         * @param size The size of the message.
         */
        virtual void sendMessage(DeviceID to, uint8_t* buffer, uint8_t size) = 0;

        /**
         * @brief The loop method is used to perform all necessary repeated actions to maintain communication.
         * This method should be called continuously from your main program loop, it handles the communication process,
         * processes incoming messages and triggers registered callbacks.
         * Device-specific implementation can add functionalities like checking for timeouts, processing queued messages, etc.
         */
        virtual void loop() = 0;

        /**
         * @brief Gets the ID of the device.
         *
         * @return The ID of the device.
         */
        [[nodiscard]] DeviceID getID() const { return id; }

        /**
         * @brief Connects to a device with the specified information.
         * Behavior is implementation-specific.
         * In any case, the onDeviceConnected and onError callbacks will allow you to know the result of the connection.
         *
         * @param info The device information.
         */
        virtual void connect(DeviceInfo& info) = 0;

        /**
         * @brief Disconnects from a device with the specified ID.
         *
         * @param deviceId The ID of the device to disconnect from.
         */
        virtual void disconnect(DeviceID deviceId) = 0;

        /**
         * @brief Checks if the device with the specified ID is connected.
         *
         * @param deviceId The ID of the device to check.
         * @return True if the device is connected, false otherwise.
         */
        virtual bool isConnected(DeviceID deviceId) = 0;

        /**
         * @brief Registers a callback function to be invoked when a device is connected.
         *
         * The registered callback function will be called when a device is connected, and will be
         * passed the device ID and device information of the connected device.
         *
         * @param callback A pointer to the function that will be called when a device is connected.
         *                 The function should have the following signature:
         *                 void callback(DeviceID, DeviceInfo&)
         *                 The first parameter is the device ID of the connected device.
         *                 The second parameter is a reference to a DeviceInfo object containing information about the connected device.
         *
         * @return No return value.
         */
        void onDeviceConnected(void (*callback)(DeviceID, DeviceInfo&)) { onDeviceConnectedCallback = callback; }

        /**
         * @brief Sets the callback function to be called when a device is disconnected.
         *
         * This method allows you to register a callback function that will be called when a device is disconnected.
         * The callback function should have the signature `void callback(DeviceID)`.
         *
         * @param callback A pointer to the function that will be called when a device is disconnected.
         *                  The function should take a single parameter of type DeviceID.
         */
        void onDeviceDisconnected(void (*callback)(DeviceID)) { onDeviceDisconnectedCallback = callback; }

        /**
         * @brief Sets the callback function for when a message is received.
         *
         * This function allows the user to set a callback function that will be called when a message is received.
         * The callback function should have the following signature: void callback(DeviceID, uint8_t*, uint8_t).
         * The first parameter is the device ID of the sender, the second parameter is a pointer to the message data, and
         * the third parameter is the length of the message data.
         *
         * Example usage:
         * @code
         * void handleMessage(DeviceID sender, uint8_t* data, uint8_t length)
         * {
         *     // Process the received message
         * }
         *
         * // Set the callback function
         * onMessageReceived(handleMessage);
         * @endcode
         *
         * @param callback A pointer to the callback function to be called when a message is received.
         *                 The function should have the signature: void callback(DeviceID, uint8_t*, uint8_t).
         */
        void onMessageReceived(void (*callback)(DeviceID, uint8_t*, uint8_t)) { onMessageReceivedCallback = callback; }

        /**
         * @brief Sets the callback function to handle error events.
         *
         * This method allows the user to specify a callback function to be called
         * when an error occurs. The callback function takes three parameters:
         * the device ID, the error code, and a string message describing the error.
         *
         * @param callback A pointer to the callback function to be called when an error occurs.
         *                 The function signature should be: void (*callback)(DeviceID, ErrorCode, const char*)
         */
        void onError(void (*callback)(DeviceID, ErrorCode, const char*)) { onErrorCallback = callback; }

    protected:
        void triggerDeviceConnected(const DeviceID id, DeviceInfo& info) const {
            if (onDeviceConnectedCallback) {
                onDeviceConnectedCallback(id, info);
            }
        }

        void triggerError(const DeviceID id, const ErrorCode code, const char* message) const {
            if (onErrorCallback) {
                onErrorCallback(id, code, message);
            }
        }

        void triggerDeviceDisconnected(const DeviceID id) const {
            if (onDeviceDisconnectedCallback) {
                onDeviceDisconnectedCallback(id);
            }
        }

        void triggerMessageReceived(const DeviceID id, uint8_t* payload, const uint8_t size) const {
            if (onMessageReceivedCallback) {
                onMessageReceivedCallback(id, payload, size);
            }
        }

    private:
        DeviceID id; ///< The ID of the device

        /**
         * @brief Pointer to a callback function that is called when a device is connected.
         *
         * The function signature of the callback function should be: void (*onDeviceConnectedCallback)(DeviceID, DeviceInfo&).
         * This callback function is used to handle any actions that need to be performed when a device is connected.
         *
         * @param id The ID of the device that was connected.
         * @param info The information of the device that was connected.
         */
        void (*onDeviceConnectedCallback)(DeviceID, DeviceInfo&);

        /**
         * @brief Pointer to a callback function that is called when an error occurs.
         *
         * The function signature of the callback function should be: void (*onErrorCallback)(DeviceID, ErrorCode, const char*);
         *
         * This callback function is used to handle any errors that occur during communication with a device.
         *
         * @param id The ID of the device that caused the error.
         * @param code The error code.
         * @param message A pointer to the error message.
         */
        void (*onErrorCallback)(DeviceID, ErrorCode, const char*);

        /**
         * @brief Callback function that is called when a device is disconnected.
         *
         * This function pointer type represents a callback function that is triggered when a device is disconnected.
         * The function takes a single parameter, which is the ID of the disconnected device.
         * User code should assign a function with this signature to this variable to handle device disconnection events.
         *
         * @param deviceId The ID of the device that received the message.
         */
        void (*onDeviceDisconnectedCallback)(DeviceID);

        /**
         * @brief A function pointer that is used to define the callback function for message received event.
         *
         * This function pointer can be used to register a callback function that will be called when a message is received.
         * The callback function should have the following signature:
         *   void callback(DeviceID deviceId, uint8_t* data, uint8_t length);
         * where:
         *   - deviceId: The ID of the device that received the message.
         *   - data: A pointer to the data of the received message.
         *   - length: The length of the data.
         *
         * @param deviceId The ID of the device that received the message.
         * @param data A pointer to the data of the received message.
         * @param length The length of the data.
         */
        void (*onMessageReceivedCallback)(DeviceID, uint8_t*, uint8_t);
    };
}

#endif // BPA_TUNNEL_H
