#include "test_message_validation.h"
#include <unity.h>

#include "BinaryMessage.h"


void test_validateMessage_ok() {
    uint8_t data[] = {1};
    const bpa::BinaryMessage message = {bpa::StartByte::START_V1, 1, 1, 1, data};

    TEST_ASSERT_EQUAL(bpa::STATUS_OK, bpa::BinaryMessageIO::validate(message));
}

void test_validateMessage_invalidStartByte() {
    uint8_t data[] = {1};
    const bpa::BinaryMessage message = {bpa::StartByte::UNDEFINED, 1, 1, 1, data};

    TEST_ASSERT_EQUAL(bpa::STATUS_MISSED_START_BYTE, bpa::BinaryMessageIO::validate(message));
}

void test_validateMessage_invalidDeviceID() {
    uint8_t data[] = {1};
    const bpa::BinaryMessage message = {bpa::StartByte::START_V1, 0, 1, 1, data};

    TEST_ASSERT_EQUAL(bpa::STATUS_MISSED_DEVICE_ID, bpa::BinaryMessageIO::validate(message));
}

void test_validateMessage_invalidMessageID() {
    uint8_t data[] = {1};
    const bpa::BinaryMessage message = {bpa::StartByte::START_V1, 1, 0, 0, data};

    TEST_ASSERT_EQUAL(bpa::STATUS_MISSED_MESSAGE_ID, bpa::BinaryMessageIO::validate(message));
}

void test_validateMessage_invalidSize() {
    const bpa::BinaryMessage message1 = {bpa::StartByte::START_V1, 1, 1, 0, nullptr};
    const bpa::BinaryMessage message2 = {bpa::StartByte::PING, 1, 1, 1, nullptr};
    uint8_t data[] = {1};
    const bpa::BinaryMessage message3 = {bpa::StartByte::PING, 1, 1, 0, data};

    TEST_ASSERT_EQUAL(bpa::STATUS_INCORRECT_FORMAT, bpa::BinaryMessageIO::validate(message1));
    TEST_ASSERT_EQUAL(bpa::STATUS_INCORRECT_FORMAT, bpa::BinaryMessageIO::validate(message2));
    TEST_ASSERT_EQUAL(bpa::STATUS_INCORRECT_FORMAT, bpa::BinaryMessageIO::validate(message3));
}

void test_validateMessage_handshake_sizeShouldBe3() {
    uint8_t data[] = {};
    const bpa::BinaryMessage handshakeInit = {bpa::StartByte::HANDSHAKE_INIT, 1, 1, 1, data};
    const bpa::BinaryMessage handshakeResp = {bpa::StartByte::HANDSHAKE_RESP, 1, 2, 2, data};
    const bpa::BinaryMessage handshakeComplete = {bpa::StartByte::HANDSHAKE_COMPLETE, 1, 3, 4, data};

    TEST_ASSERT_EQUAL(bpa::STATUS_INCORRECT_FORMAT, bpa::BinaryMessageIO::validate(handshakeInit));
    TEST_ASSERT_EQUAL(bpa::STATUS_INCORRECT_FORMAT, bpa::BinaryMessageIO::validate(handshakeResp));
    TEST_ASSERT_EQUAL(bpa::STATUS_INCORRECT_FORMAT, bpa::BinaryMessageIO::validate(handshakeComplete));

    const bpa::BinaryMessage handshakeInit_ok = {bpa::StartByte::HANDSHAKE_INIT, 1, 4, 3, data};
    const bpa::BinaryMessage handshakeResp_ok = {bpa::StartByte::HANDSHAKE_RESP, 1, 5, 3, data};
    const bpa::BinaryMessage handshakeComplete_ok = {bpa::StartByte::HANDSHAKE_COMPLETE, 1, 6, 3, data};

    TEST_ASSERT_EQUAL(bpa::STATUS_OK, bpa::BinaryMessageIO::validate(handshakeInit_ok));
    TEST_ASSERT_EQUAL(bpa::STATUS_OK, bpa::BinaryMessageIO::validate(handshakeResp_ok));
    TEST_ASSERT_EQUAL(bpa::STATUS_OK, bpa::BinaryMessageIO::validate(handshakeComplete_ok));
}

void test_validateMessage_ping_payloadShouldBeEmpty() {
    uint8_t data[] = {1};
    const bpa::BinaryMessage incorrectPing = {bpa::StartByte::PING, 1, 1, 1, data};
    const bpa::BinaryMessage correctPing = {bpa::StartByte::PING, 1, 2, 0, nullptr};

    TEST_ASSERT_EQUAL(bpa::STATUS_INCORRECT_FORMAT, bpa::BinaryMessageIO::validate(incorrectPing));
    TEST_ASSERT_EQUAL(bpa::STATUS_OK, bpa::BinaryMessageIO::validate(correctPing));
}

void test_validateMessage_confirm_payloadShouldBeEmpty() {
    uint8_t data[] = {1};
    const bpa::BinaryMessage incorrectConfirm = {bpa::StartByte::CONFIRM, 1, 1, 1, data};
    const bpa::BinaryMessage correctConfirm = {bpa::StartByte::CONFIRM, 1, 2, 0, nullptr};

    TEST_ASSERT_EQUAL(bpa::STATUS_INCORRECT_FORMAT, bpa::BinaryMessageIO::validate(incorrectConfirm));
    TEST_ASSERT_EQUAL(bpa::STATUS_OK, bpa::BinaryMessageIO::validate(correctConfirm));
}

void test_validateMessage_rejected_payloadShouldBeEmpty() {
    uint8_t data[] = {1};
    const bpa::BinaryMessage incorrectRejected = {bpa::StartByte::REJECTED, 1, 1, 1, data};
    const bpa::BinaryMessage correctRejected = {bpa::StartByte::REJECTED, 1, 2, 0, nullptr};

    TEST_ASSERT_EQUAL(bpa::STATUS_INCORRECT_FORMAT, bpa::BinaryMessageIO::validate(incorrectRejected));
    TEST_ASSERT_EQUAL(bpa::STATUS_OK, bpa::BinaryMessageIO::validate(correctRejected));
}

void test_validateMessage_disconnect_payloadShouldBeEmpty() {
    uint8_t data[] = {1};
    const bpa::BinaryMessage incorrectDisconnect = {bpa::StartByte::DISCONNECT, 1, 1, 1, data};
    const bpa::BinaryMessage correctDisconnect = {bpa::StartByte::DISCONNECT, 1, 2, 0, nullptr};

    TEST_ASSERT_EQUAL(bpa::STATUS_INCORRECT_FORMAT, bpa::BinaryMessageIO::validate(incorrectDisconnect));
    TEST_ASSERT_EQUAL(bpa::STATUS_OK, bpa::BinaryMessageIO::validate(correctDisconnect));
}

void test_validateMessage_incorrectFormat_payloadShouldBeEmpty() {
    uint8_t data[] = {1};
    const bpa::BinaryMessage incorrectFormat = {bpa::StartByte::INCORRECT_FORMAT, 1, 1, 1, data};
    const bpa::BinaryMessage correctFormat = {bpa::StartByte::INCORRECT_FORMAT, 1, 2, 0, nullptr};

    TEST_ASSERT_EQUAL(bpa::STATUS_INCORRECT_FORMAT, bpa::BinaryMessageIO::validate(incorrectFormat));
    TEST_ASSERT_EQUAL(bpa::STATUS_OK, bpa::BinaryMessageIO::validate(correctFormat));
}

void test_validateMessage_incorrectChecksum_payloadShouldBeEmpty() {
    uint8_t data[] = {1};
    const bpa::BinaryMessage incorrectChecksum = {bpa::StartByte::INCORRECT_CHECKSUM, 1, 1, 1, data};
    const bpa::BinaryMessage correctChecksum = {bpa::StartByte::INCORRECT_CHECKSUM, 1, 2, 0, nullptr};

    TEST_ASSERT_EQUAL(bpa::STATUS_INCORRECT_FORMAT, bpa::BinaryMessageIO::validate(incorrectChecksum));
    TEST_ASSERT_EQUAL(bpa::STATUS_OK, bpa::BinaryMessageIO::validate(correctChecksum));
}