#include <BinaryMessage.h>
#include <unity.h>

void test_isSupportedStartByte()
{
    TEST_ASSERT_TRUE(bpa::isSupportedStartByte(bpa::StartByte::START_V1));
    TEST_ASSERT_TRUE(bpa::isSupportedStartByte(bpa::StartByte::CONFIRM));
    TEST_ASSERT_TRUE(bpa::isSupportedStartByte(bpa::StartByte::INCORRECT_FORMAT));
    TEST_ASSERT_TRUE(bpa::isSupportedStartByte(bpa::StartByte::INCORRECT_CHECKSUM));
    TEST_ASSERT_TRUE(bpa::isSupportedStartByte(bpa::StartByte::PING));
    TEST_ASSERT_TRUE(bpa::isSupportedStartByte(bpa::StartByte::REJECTED));
    TEST_ASSERT_TRUE(bpa::isSupportedStartByte(bpa::StartByte::HANDSHAKE_INIT));
    TEST_ASSERT_TRUE(bpa::isSupportedStartByte(bpa::StartByte::HANDSHAKE_RESP));
    TEST_ASSERT_TRUE(bpa::isSupportedStartByte(bpa::StartByte::HANDSHAKE_COMPLETE));
    TEST_ASSERT_TRUE(bpa::isSupportedStartByte(bpa::StartByte::DISCONNECT));

    TEST_ASSERT_FALSE(bpa::isSupportedStartByte(0x29));
    TEST_ASSERT_FALSE(bpa::isSupportedStartByte(0x3A));
    TEST_ASSERT_FALSE(bpa::isSupportedStartByte(0x40));
    TEST_ASSERT_FALSE(bpa::isSupportedStartByte(0x5B));
    TEST_ASSERT_FALSE(bpa::isSupportedStartByte(0x7D));
}

void test_isVersionStartByte()
{
    TEST_ASSERT_TRUE(bpa::isVersionStartByte(0x30));
    TEST_ASSERT_TRUE(bpa::isVersionStartByte(0x31));
    TEST_ASSERT_TRUE(bpa::isVersionStartByte(0x32));
    TEST_ASSERT_TRUE(bpa::isVersionStartByte(0x33));
    TEST_ASSERT_TRUE(bpa::isVersionStartByte(0x34));
    TEST_ASSERT_TRUE(bpa::isVersionStartByte(0x35));
    TEST_ASSERT_TRUE(bpa::isVersionStartByte(0x36));
    TEST_ASSERT_TRUE(bpa::isVersionStartByte(0x37));
    TEST_ASSERT_TRUE(bpa::isVersionStartByte(0x38));
    TEST_ASSERT_TRUE(bpa::isVersionStartByte(0x39));

    TEST_ASSERT_FALSE(bpa::isVersionStartByte(0x2F));
    TEST_ASSERT_FALSE(bpa::isVersionStartByte(0x3A));
    TEST_ASSERT_FALSE(bpa::isVersionStartByte(0x2E));
    TEST_ASSERT_FALSE(bpa::isVersionStartByte(0x40));
    TEST_ASSERT_FALSE(bpa::isVersionStartByte(0x5B));
}

void test_isControlStartByte()
{
    TEST_ASSERT_TRUE(bpa::isControlStartByte(0x41));
    TEST_ASSERT_TRUE(bpa::isControlStartByte(0x42));
    TEST_ASSERT_TRUE(bpa::isControlStartByte(0x43));
    TEST_ASSERT_TRUE(bpa::isControlStartByte(0x44));
    TEST_ASSERT_TRUE(bpa::isControlStartByte(0x45));
    TEST_ASSERT_TRUE(bpa::isControlStartByte(0x46));
    TEST_ASSERT_TRUE(bpa::isControlStartByte(0x47));
    TEST_ASSERT_TRUE(bpa::isControlStartByte(0x48));
    TEST_ASSERT_TRUE(bpa::isControlStartByte(0x49));
    TEST_ASSERT_TRUE(bpa::isControlStartByte(0x4A));
    TEST_ASSERT_TRUE(bpa::isControlStartByte(0x4B));
    TEST_ASSERT_TRUE(bpa::isControlStartByte(0x4C));
    TEST_ASSERT_TRUE(bpa::isControlStartByte(0x4D));
    TEST_ASSERT_TRUE(bpa::isControlStartByte(0x4E));
    TEST_ASSERT_TRUE(bpa::isControlStartByte(0x4F));
    TEST_ASSERT_TRUE(bpa::isControlStartByte(0x50));
    TEST_ASSERT_TRUE(bpa::isControlStartByte(0x51));
    TEST_ASSERT_TRUE(bpa::isControlStartByte(0x52));
    TEST_ASSERT_TRUE(bpa::isControlStartByte(0x53));
    TEST_ASSERT_TRUE(bpa::isControlStartByte(0x54));
    TEST_ASSERT_TRUE(bpa::isControlStartByte(0x55));
    TEST_ASSERT_TRUE(bpa::isControlStartByte(0x56));
    TEST_ASSERT_TRUE(bpa::isControlStartByte(0x57));
    TEST_ASSERT_TRUE(bpa::isControlStartByte(0x58));
    TEST_ASSERT_TRUE(bpa::isControlStartByte(0x59));
    TEST_ASSERT_TRUE(bpa::isControlStartByte(0x5A));

    TEST_ASSERT_FALSE(bpa::isControlStartByte(0x40));
    TEST_ASSERT_FALSE(bpa::isControlStartByte(0x5B));
}

void test_isHandshakeStartByte()
{
    TEST_ASSERT_TRUE(bpa::isHandshakeStartByte(0x2A));
    TEST_ASSERT_TRUE(bpa::isHandshakeStartByte(0x2B));
    TEST_ASSERT_TRUE(bpa::isHandshakeStartByte(0x2E));

    TEST_ASSERT_FALSE(bpa::isHandshakeStartByte(0x29));
    TEST_ASSERT_FALSE(bpa::isHandshakeStartByte(0x2F));
    TEST_ASSERT_FALSE(bpa::isHandshakeStartByte(0x30));
    TEST_ASSERT_FALSE(bpa::isHandshakeStartByte(0x3A));
    TEST_ASSERT_FALSE(bpa::isHandshakeStartByte(0x40));
    TEST_ASSERT_FALSE(bpa::isHandshakeStartByte(0x5B));
}

void test_emptyMessage()
{
    bpa::BinaryMessage message = bpa::emptyMessage();
    TEST_ASSERT_EQUAL(bpa::StartByte::UNDEFINED, message.start);
    TEST_ASSERT_EQUAL(0, message.device_id);
    TEST_ASSERT_EQUAL(0, message.message_id);
    TEST_ASSERT_EQUAL(0, message.size);
    TEST_ASSERT_NULL(message.data);
}

void test_isMessageEmpty()
{
    bpa::BinaryMessage message = bpa::emptyMessage();
    TEST_ASSERT_TRUE(bpa::isMessageEmpty(message));

    message.start = bpa::StartByte::START_V1;
    TEST_ASSERT_FALSE(bpa::isMessageEmpty(message));
}
