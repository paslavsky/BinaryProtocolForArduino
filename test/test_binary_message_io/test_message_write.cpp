#include "test_message_write.h"

#include <unity.h>

#include "BinaryMessage.h"
#include "mock_udp.h"

void test_writeMessage_withoutData() {
    uint8_t buffer[BPA_MAX_SIZE] = {0};
    const bpa::BinaryMessage message = {bpa::StartByte::PING, 1, 1, 0, nullptr};
    io.write(message);
    const auto count = udp.mock_getWroteData(buffer, BPA_MAX_SIZE);
    TEST_ASSERT_EQUAL(6, count);
    TEST_ASSERT_EQUAL(0x50, buffer[0]);
    TEST_ASSERT_EQUAL(1, buffer[1]);
    TEST_ASSERT_EQUAL(1, buffer[2]);
    TEST_ASSERT_EQUAL(0, buffer[3]);
}

void test_writeMessage_withData() {
    uint8_t buffer[BPA_MAX_SIZE] = {0};
    uint8_t data[] = {1, 2, 3};
    const bpa::BinaryMessage message = {bpa::StartByte::START_V1, 1, 1, 3, data};
    io.write(message);
    const auto count = udp.mock_getWroteData(buffer, BPA_MAX_SIZE);
    TEST_ASSERT_EQUAL(9, count);
    TEST_ASSERT_EQUAL(0x30, buffer[0]);
    TEST_ASSERT_EQUAL(1, buffer[1]);
    TEST_ASSERT_EQUAL(1, buffer[2]);
    TEST_ASSERT_EQUAL(3, buffer[3]);
    TEST_ASSERT_EQUAL(1, buffer[4]);
    TEST_ASSERT_EQUAL(2, buffer[5]);
    TEST_ASSERT_EQUAL(3, buffer[6]);
}

void test_writeMessage_checksumShouldBeCalculated() {
    uint8_t buffer[BPA_MAX_SIZE] = {0};
    const bpa::BinaryMessage message1 = {bpa::StartByte::PING, 1, 1, 0, nullptr};
    io.write(message1);
    auto count = udp.mock_getWroteData(buffer, BPA_MAX_SIZE);
    const uint16_t checksum1 = buffer[count - 2] << 8 | buffer[count - 1];

    uint8_t data[] = {1, 2, 3};
    const bpa::BinaryMessage message2 = {bpa::StartByte::START_V1, 1, 1, 3, data};
    io.write(message2);
    count = udp.mock_getWroteData(buffer, BPA_MAX_SIZE);
    const uint16_t checksum2 = buffer[count - 2] << 8 | buffer[count - 1];

    TEST_ASSERT_EQUAL_HEX16(0x0097, checksum1);
    TEST_ASSERT_EQUAL_HEX16(0x1937, checksum2);
}
