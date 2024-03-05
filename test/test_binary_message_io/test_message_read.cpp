#include "test_message_read.h"

#include <unity.h>

#include "mock_udp.h"

void test_readMessage_withoutData() {
    const uint8_t data[] = {0x41, 0x01, 0x01, 0x00, 0xF0, 0x76};
    udp.mock_setPacketToParse(data, 6);
    const auto [message, status] = io.read();
    TEST_ASSERT_EQUAL(bpa::StartByte::CONFIRM, message.start);
    TEST_ASSERT_EQUAL(1, message.device_id);
    TEST_ASSERT_EQUAL(1, message.message_id);
    TEST_ASSERT_EQUAL(0, message.size);
    TEST_ASSERT_EQUAL(nullptr, message.data);
    TEST_ASSERT_EQUAL(bpa::ValidationStatus::STATUS_OK, status);
}

void test_readMessage_withData() {
    const uint8_t data[] = {0x30, 0x01, 0x01, 0x03, 0x01, 0x02, 0x03, 0xB9, 0xA4};
    udp.mock_setPacketToParse(data, 9);
    const auto [message, status] = io.read();
    TEST_ASSERT_EQUAL(bpa::StartByte::START_V1, message.start);
    TEST_ASSERT_EQUAL(1, message.device_id);
    TEST_ASSERT_EQUAL(1, message.message_id);
    TEST_ASSERT_EQUAL(3, message.size);
    TEST_ASSERT_EQUAL(1, message.data[0]);
    TEST_ASSERT_EQUAL(2, message.data[1]);
    TEST_ASSERT_EQUAL(3, message.data[2]);
    TEST_ASSERT_EQUAL(bpa::ValidationStatus::STATUS_OK, status);
}

void test_readMessage_incorrectStreamLength() {
    const uint8_t data[] = {0x30, 0x01, 0x01, 0x03, 0x01, 0x02, 0x00, 0xB9};
    udp.mock_setPacketToParse(data, 8);
    const auto [message, status] = io.read();
    TEST_ASSERT_TRUE(bpa::isMessageEmpty(message));
    TEST_ASSERT_EQUAL(bpa::ValidationStatus::STATUS_UNEXPECTED_END_OF_STREAM, status);
}

void test_readMessage_invalidChecksum() {
    const uint8_t data[] = {0x41, 0x01, 0x01, 0x00, 0x01, 0x01};
    udp.mock_setPacketToParse(data, 6);
    const auto [message, status] = io.read();
    TEST_ASSERT_EQUAL(bpa::StartByte::CONFIRM, message.start);
    TEST_ASSERT_EQUAL(1, message.device_id);
    TEST_ASSERT_EQUAL(1, message.message_id);
    TEST_ASSERT_EQUAL(0, message.size);
    TEST_ASSERT_EQUAL(nullptr, message.data);
    TEST_ASSERT_EQUAL(bpa::ValidationStatus::STATUS_INCORRECT_CHECKSUM, status);
}
