#include <BinaryMessage.h>
#include <Arduino.h>
#include <unity.h>
#include <MockUdp.h>
#include "test_utility_functions.h"
#include "test_message_validation.h"
#include "test_message_write.h"
#include "test_message_read.h"

MockUDP udp;

bpa::BinaryMessageIO io(udp);

void setUp()
{
    // set stuff up here
}

void tearDown()
{
    udp.mock_reset();
}

void setup()
{
    Serial.begin(115200);
    delay(2000); // service delay
    UNITY_BEGIN();

    RUN_TEST(test_isSupportedStartByte);
    RUN_TEST(test_isVersionStartByte);
    RUN_TEST(test_isControlStartByte);
    RUN_TEST(test_isHandshakeStartByte);
    RUN_TEST(test_emptyMessage);
    RUN_TEST(test_isMessageEmpty);
    RUN_TEST(test_validateMessage_ok);
    RUN_TEST(test_validateMessage_invalidStartByte);
    RUN_TEST(test_validateMessage_invalidDeviceID);
    RUN_TEST(test_validateMessage_invalidMessageID);
    RUN_TEST(test_validateMessage_invalidSize);
    RUN_TEST(test_validateMessage_handshake_sizeShouldBe3);
    RUN_TEST(test_validateMessage_ping_payloadShouldBeEmpty);
    RUN_TEST(test_validateMessage_confirm_payloadShouldBeEmpty);
    RUN_TEST(test_validateMessage_rejected_payloadShouldBeEmpty);
    RUN_TEST(test_validateMessage_disconnect_payloadShouldBeEmpty);
    RUN_TEST(test_validateMessage_incorrectFormat_payloadShouldBeEmpty);
    RUN_TEST(test_validateMessage_incorrectChecksum_payloadShouldBeEmpty);
    RUN_TEST(test_writeMessage_withData);
    RUN_TEST(test_writeMessage_withoutData);
    RUN_TEST(test_writeMessage_checksumShouldBeCalculated);
    RUN_TEST(test_readMessage_withoutData);
    RUN_TEST(test_readMessage_withData);
    RUN_TEST(test_readMessage_incorrectStreamLength);
    RUN_TEST(test_readMessage_invalidChecksum);

    UNITY_END(); // stop unit testing
}

void loop()
{
}